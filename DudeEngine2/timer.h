/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   timer.h
    Desc:       wrapper over timers from different backends (SDL2, LDL, etc.)
    Created:    by DimaSkup
\**********************************************************************************/
#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void     Sys_Delay(uint32_t ms);
uint32_t Sys_GetTicks();

#ifdef __cplusplus
}
#endif

#endif
