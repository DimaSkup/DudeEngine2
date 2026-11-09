/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: texture.cpp
    Created:  by DimaSkup
\**********************************************************************************/
#include "texture.h"

#if USE_SDL2
    #include "backends/SDL/sdl_tex.cpp"
#elif USE_LDL
    #include "backends/LDL/ldl_texture.cpp"
#else
    #error "No implementation selected"
#endif
