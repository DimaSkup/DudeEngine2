/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: timer.cpp
    Created:  by DimaSkup
\**********************************************************************************/
#include "timer.h"
#include "defines.h"

#if USE_SDL2
    #include "backends/SDL/sdl_timer.cpp"
#elif USE_LDL
    #include "backends/LDL/ldl_timer.cpp"
#else
    #error "No implementation selected"
#endif