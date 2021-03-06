/**
 * @file motion_control.c
 *
 * motion control
 *
 * @author Ricardo <tsao.ricardo@iac.com.tw>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "system.h"
#include "../platform/platform.h"

#include "small-matrix-inverse/invert4x4_c.h"

#define DEBUG false

#define USE_PID
//#define USE_FUZ

static bool initialized = false;
static float R = DEFAULT_R;

/* kinematics equations */
bool kinematics_init(system_data* sd);
bool forward_kinematics(system_data* sd);
bool inverse_kinematics(system_data* sd);

bool motion_control_init(system_data* sd)
{
    MSG(sd->log, "%s", "[INFO] motion_control_init... \n");

    if(sd == NULL)
    {
        MSG(sd->log, "[ERROR] motion_control_update, failed! \n");
        return false;
    }

    if(initialized == true)
        return true;

    if(!kinematics_init(sd))
        return false;

    #if defined(USE_PID)
        /* use pid control */
        pid_control_init(sd);
    #elif defined(USE_FUZ)
        /* use fuzzy control */
        fuzzy_control_init(sd);
    #else
        #error "USE_PID/USE_FUZ must be defined!"
    #endif

    initialized = true;

    return true;
}

bool motion_control_update(system_data* sd)
{
    if((sd == NULL) || (initialized != true))
    {
        MSG(sd->log, "[ERROR] motion_control_update, failed! \n");
        return false;
    }

    if(sd->sv.vx > DEFAULT_MAX_VX)
        sd->sv.vx = DEFAULT_MAX_VX;

    if(sd->sv.vy > DEFAULT_MAX_VY)
        sd->sv.vy = DEFAULT_MAX_VY;

    if(sd->sv.w0 > DEFAULT_MAX_W0)
        sd->sv.w0 = DEFAULT_MAX_W0;

    /* calculating feedback signals, vx, vy, w0 */
    forward_kinematics(sd);

    /* calculating output signals, vx, vy, w0 */
    #if defined(USE_PID)
        /* use pid control */
        pid_control_update(sd);
    #elif defined(USE_FUZ)
        /* use fuzzy control */
        fuzzy_control_update(sd);
    #endif

    /* neural network learning */
    neural_network_update(sd);

    /* calculating output signals, w1, w2, w3, w4 */
    inverse_kinematics(sd);

    return true;
}

/* get the inverted matrix of RV(4x4) */
bool kinematics_init(system_data* sd)
{
    uint16_t i = 0, row = 0, col = 0;

    float mat_inverse[4][4] = {{1.0f,  1.0f, -(DEFAULT_L1 + DEFAULT_L2), 0.0f},
                               {1.0f, -1.0f,  (DEFAULT_L1 + DEFAULT_L2), 0.0f},
                               {1.0f, -1.0f, -(DEFAULT_L1 + DEFAULT_L2), 0.0f},
                               {1.0f,  1.0f,  (DEFAULT_L1 + DEFAULT_L2), 1.0f}};

    float mat_forward[4][4] = {0};

    float mat_src[16] = {0};
    float mat_dst[16] = {0};

    if(sd == NULL)
    {
        MSG(sd->log, "[ERROR] kinematics_init, failed! \n");
        return false;
    }

    if(initialized == true)
        return true;

    #if DEBUG
    for(i = 0 ; i < 16 ; i++)
    {
        if(i < 1)
            MSG(sd->log, "[DEBUG] mat_inverse(2D) : \n");

        row = (i / 4); col = (i % 4);
        MSG(sd->log, "%9.4f(%d%d) ", mat_inverse[row][col], row, col);

        if(((i + 1) % 4 == 0) || (i == 15))
            MSG(sd->log, "%s", (i < 15)? "\n":"\n\n");
    }
    #endif

    for(i = 0; i < 16; i++)
    {
        row = (i / 4); col = (i % 4);
        mat_src[i] = mat_inverse[row][col];

        #if DEBUG
        if(i < 1)
            MSG(sd->log, "[DEBUG] mat_inverse(2D) to  mat_src(1D) \n");

        MSG(sd->log, "%9.4f(%2d) ", mat_src[i], i);

        if(((i + 1) % 4 == 0) || (i == 15))
            MSG(sd->log, "%s", (i < 15)? "\n":"\n\n");
        #endif
    }

    if(!invert4x4(mat_src, mat_dst))
    {
        MSG(sd->log, "[ERROR] matrix singular! \n");
        return false;
    }

    #if DEBUG
    for(i = 0 ; i < 16 ; i++)
    {
        if(i < 1)
            MSG(sd->log, "[DEBUG] mat_dst(1D) = \n");

        row = (i / 4); col = (i % 4);
        MSG(sd->log, "%9.4f(%2d) ", mat_dst[i], i);

        if(((i + 1) % 4 == 0) || (i == 15))
            MSG(sd->log, "%s", (i < 15)? "\n":"\n\n");
    }
    #endif

    for(i = 0; i < 16; i++)
    {
        row = (i / 4); col = (i % 4);
        mat_forward[row][col] = mat_dst[i];

        #if DEBUG
        if(i < 1)
            MSG(sd->log, "[DEBUG] mat_dst(1D) to  mat_forward(2D) \n");

        MSG(sd->log, "%9.4f(%d%d) ", mat_forward[row][col], row, col);

        if(((i + 1) % 4 == 0) || (i == 15))
            MSG(sd->log, "%s", (i < 15)? "\n":"\n\n");
        #endif
    }

    memcpy(sd->mat_inverse, mat_inverse, sizeof(mat_inverse));
    memcpy(sd->mat_forward, mat_forward, sizeof(mat_forward));

    #if DEBUG
    for(i = 0 ; i < 16 ; i++)
    {
        if(i < 1)
            MSG(sd->log, "[DEBUG] sd->mat_inverse(2D) = \n");

        row = (i / 4); col = (i % 4);
        MSG(sd->log, "%9.4f(%d%d) ", sd->mat_inverse[row][col], row, col);

        if(((i + 1) % 4 == 0) || (i == 15))
            MSG(sd->log, "%s", (i < 15)? "\n":"\n\n");
    }

    for(i = 0 ; i < 16 ; i++)
    {
        if(i < 1)
            MSG(sd->log, "[DEBUG] sd->mat_forward(2D) = \n");

        row = (i / 4); col = (i % 4);
        MSG(sd->log, "%9.4f(%d%d) ", sd->mat_forward[row][col], row, col);

        if(((i + 1) % 4 == 0) || (i == 15))
            MSG(sd->log, "%s", (i < 15)? "\n":"\n\n");
    }
    #endif

    return true;
}

/* inverse kinematics equation */
bool inverse_kinematics(system_data* sd)
{
    float vx = sd->cv.vx;
    float vy = sd->cv.vy;
    float w0 = sd->cv.w0;
    float w1, w2, w3, w4;

    float mat[4][4] = {0};

    if(sd == NULL)
    {
        MSG(sd->log, "[ERROR] inverse_kinematics, failed! \n");
        return false;
    }

    if( R <= 0)
    {
        MSG(sd->log, "[ERROR] inverse_kinematics, failed(R <= 0)! \n");
        return false;
    }

    memcpy(mat, sd->mat_inverse, sizeof(mat));

    w1 = (1.0f / R) * (mat[0][0] * vx + mat[0][1] * vy + mat[0][2] * w0);
    w2 = (1.0f / R) * (mat[1][0] * vx + mat[1][1] * vy + mat[1][2] * w0);
    w3 = (1.0f / R) * (mat[2][0] * vx + mat[2][1] * vy + mat[2][2] * w0);
    w4 = (1.0f / R) * (mat[3][0] * vx + mat[3][1] * vy + mat[3][2] * w0);

    /* signed zero to be zero */
    if (w1 == 0) w1 = 0;
    if (w2 == 0) w2 = 0;
    if (w3 == 0) w3 = 0;
    if (w4 == 0) w4 = 0;

    sd->mot.out.w1 = w1;
    sd->mot.out.w2 = w2;
    sd->mot.out.w3 = w3;
    sd->mot.out.w4 = w4;

    #if DEBUG
    MSG(sd->log, "[DEBUG] inverse_kinematics : \n");
    MSG(sd->log, "vx, vy (m/s), w0 (rad/s) = \n");
    MSG(sd->log, "%9.4f %9.4f %9.4f \n\n", vx, vy, w0);
    MSG(sd->log, "w1, w2, w3, w4 (rad/s) = \n");
    MSG(sd->log, "%9.4f %9.4f %9.4f %9.4f \n\n", sd->mot.out.w1, sd->mot.out.w2,
                                                 sd->mot.out.w3, sd->mot.out.w4);
    #endif

    return true;
}

/* forward kinematics equation */
bool forward_kinematics(system_data* sd)
{
    float w1 = sd->mot.in.w1;
    float w2 = sd->mot.in.w2;
    float w3 = sd->mot.in.w3;
    float w4 = sd->mot.in.w4;
    float vx, vy, w0;

    float mat[4][4] = {0};

    if(sd == NULL)
    {
        MSG(sd->log, "[ERROR] forward_kinematics, failed! \n");
        return false;
    }

    memcpy(mat, sd->mat_forward, sizeof(mat));

    vx = R * (mat[0][0] * w1 + mat[0][1] * w2 + mat[0][2] * w3 + mat[0][3] * w4);
    vy = R * (mat[1][0] * w1 + mat[1][1] * w2 + mat[1][2] * w3 + mat[1][3] * w4);
    w0 = R * (mat[2][0] * w1 + mat[2][1] * w2 + mat[2][2] * w3 + mat[2][3] * w4);

    /* signed zero to be zero */
    if (vx == 0) vx = 0;
    if (vy == 0) vy = 0;
    if (w0 == 0) w0 = 0;

    sd->pv.vx = vx;
    sd->pv.vy = vy;
    sd->pv.w0 = w0;

    #if DEBUG
    MSG(sd->log, "[DEBUG] forward_kinematics : \n");
    MSG(sd->log, "w1, w2, w3, w4 (rad/s) = \n");
    MSG(sd->log, "%9.4f %9.4f %9.4f %9.4f \n\n", w1, w2, w3, w4);
    MSG(sd->log, "vx, vy (m/s), w0 (rad/s) = \n");
    MSG(sd->log, "%9.4f %9.4f %9.4f \n\n", sd->pv.vx, sd->pv.vy, sd->pv.w0);
    #endif

    return true;
}

