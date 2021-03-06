/**
 * @file spec.h
 *
 * platform settings for GENERAL 2WD platform
 *
 * @author Ricardo <tsao.ricardo@iac.com.tw>
 */

#pragma once

#ifdef PLATFORM_OMR

/* Specification */
#define DEFAULT_DIFF_DRV                2           // 2WD
#define DEFAULT_L1                      (450.0f/2)  // mm, width/2
#define DEFAULT_L2                      (480.0f/2)  // mm, high/2
#define DEFAULT_R                       (100.0f/2)  // mm
#define DEFAULT_WET                     0.6f        // Kg, weight
#define DEFAULT_MAX_V                   0.6f        // m/s, velocity
#define DEFAULT_MAX_W                   0.6f        // rad/s, rotation rate
#define DEFAULT_MAX_LOAD                20.0f       // Kg

/* Motor Driver */
#define DEFAULT_MOTOR_POWER             30.0f       // Watt (W)
#define DEFAULT_RPM2PWM_SLOPE           1.0f

/* Guide Sensor */
#define DEFAULT_GUIDE_SENSOR_OFFSET     (450.0f/2)

#endif
