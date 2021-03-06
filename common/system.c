/**
 * @file system.c
 *
 * system
 *
 * @author Ricardo <tsao.ricardo@iac.com.tw>
 */

#if defined(_WIN32)
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <time.h>

#include "system.h"
#include "../platform/platform.h"

void signal_callback_handler(int signum);

void timer1_callback_handler(int signum);
void timer2_callback_handler(int signum);
void timer3_callback_handler(int signum);

system_data* system_init(void)
{
    MSG(data.log, "%s", "[INFO] system_init... \n");

    // Register signal and signal handler
    //signal(SIGINT, signal_callback_handler);

    system_data* sd = (system_data*)malloc(sizeof(system_data));

    if(sd == NULL)
        MSG(data.log, "%s", "[ERROR] system_init, failed! \n");

    return sd;
}

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void signal_callback_handler(int signum)
{
    /* Reset handler to catch SIGINT next time.
       Refer http://en.cppreference.com/w/c/program/signal */
    signal(SIGINT, signal_callback_handler);

    printf("[ERROR] cannot be terminated using Ctrl + C \n");

    /* Cleanup and close up stuff here */
    fflush(stdout);
    //system("pause");

    /* Terminate program */
    //exit(signum);
}

int system_task_create(void *(*task) (void *), void *arg)
{
    if((task == NULL) || (arg == NULL))
    {
        MSG(data.log, "%s", "[ERROR] system_task_create, failed! \n");
        return false;
    }

    return true;
}

#if 1
int msleep(unsigned long milisec)
{
    struct timespec req = {0};
    time_t sec = (int)(milisec / 1000);
    milisec = milisec - (sec * 1000);
    req.tv_sec = sec;
    req.tv_nsec = milisec * 1000000L;
    while(nanosleep(&req,&req) == (-1))
         continue;
    return 1;
}
#else
int msleep(unsigned long milisec)
{
    #if defined(_WIN32)
    Sleep(milisec);
    #elif defined(__linux__)
    msleep(milisec); // will caused segmentation fault in linux
    #endif
}
#endif

void timer1_callback_handler(int signum)
{
    printf("signum = %d", signum);
}

void timer2_callback_handler(int signum)
{
    printf("signum = %d", signum);
}

void timer3_callback_handler(int signum)
{
    printf("signum = %d", signum);
}

/* parameters */

#if 0
/* FreeRTOS */
void vAssertCalled( unsigned long ulLine, const char * const pcFileName )
{

}

void vApplicationIdleHook( void )
{

}

void vApplicationMallocFailedHook( void )
{

}

void vApplicationTickHook( void )
{

}

void vConfigureTimerForRunTimeStats( void )
{
    /* This function is not used by the Blinky build configuration, but needs
    to be defined as the Blinky and Full build configurations share a
    FreeRTOSConfig.h header file. */
}

unsigned long ulGetRunTimeCounterValue( void )
{
    /* This function is not used by the Blinky build configuration, but needs
    to be defined as the Blinky and Full build configurations share a
    FreeRTOSConfig.h header file. */
    return 0UL;
}
#endif

