/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: sdl_timer.cpp
    Desc:     implementation of SDL2-specific functional for work with timers
    Created:  13.06.2026 by DimaSkup
\**********************************************************************************/
#include <SDL_timer.h>

void Sys_Delay(uint32_t ms)
{
    SDL_Delay(ms);
}

uint32_t Sys_GetTicks()
{
    return SDL_GetTicks();
}