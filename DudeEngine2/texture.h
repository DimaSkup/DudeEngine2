/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   texture.h
    Desc:       a wrapper for texture from different rendering backends (SDL2, LDL, etc.)
    Created:    09.06.2026
\**********************************************************************************/
#ifndef TEXTURE_H
#define TEXTURE_H

#include "types.h"
#include "defines.h"
#include "rect.h"


//========================================================================
// SDL2 specific stuff
//========================================================================
#if USE_SDL2

#include <SDL_render.h>

struct Texture
{
    SDL_Texture* pTex;
    uint16 w;
    uint16 h;
};

//========================================================================
// LDL specific stuff
//========================================================================
#elif USE_LDL

struct Texture
{
    LDL_Texture* pTex;
    uint16 w;
    uint16 h;
};

#endif

//---------------------------------------------------------
// supported blending modes
//---------------------------------------------------------
enum eBlendMode
{
    BLENDMODE_NONE = 0x00000000,     /**< no blending
                                          dstRGBA = srcRGBA */
    BLENDMODE_BLEND = 0x00000001,    /**< alpha blending
                                          dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA))
                                          dstA = srcA + (dstA * (1-srcA)) */
    BLENDMODE_ADD = 0x00000002,      /**< additive blending
                                          dstRGB = (srcRGB * srcA) + dstRGB
                                          dstA = dstA */
    BLENDMODE_MOD = 0x00000004,      /**< color modulate
                                          dstRGB = srcRGB * dstRGB
                                          dstA = dstA */
    BLENDMODE_MUL = 0x00000008,      /**< color multiply
                                          dstRGB = (srcRGB * dstRGB) + (dstRGB * (1-srcA))
                                          dstA = dstA */
    BLENDMODE_INVALID = 0x7FFFFFFF
};


//========================================================================
// FUNCTIONS
//========================================================================
void     DestroyTexture           (Texture* pTex);
Texture* CreateTexture            (const uint width, const uint height);
Texture* CreateTextureRenderTarget(const uint width, const uint height);
void     SetTextureBlendMode      (Texture* pTex, const eBlendMode blendMode);
bool     UpdateTexture            (Texture* pTex, const Rect* rect, const void* pixels, int pitch);

void     SetTextureColorMod       (Texture* tex, uint8 r, uint8 g, uint8 b);
int      SetTextureAlphaMod       (Texture* pTex, uint8 alpha);

#endif