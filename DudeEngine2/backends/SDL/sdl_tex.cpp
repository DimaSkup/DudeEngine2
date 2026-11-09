/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: sdl_tex.cpp
    Desc:     implementation of SDL2-specific functional for handling textures
    Created:  13.06.2026 by DimaSkup
\**********************************************************************************/
#include "../../log.h"
#include "../../r_main.h"
#include <assert.h>

//---------------------------------------------------------
// destroy input texture
//---------------------------------------------------------
void DestroyTexture(Texture* tex)
{
    assert(tex);
    SDL_DestroyTexture(tex->pTex);
    delete (tex);
    tex = nullptr;
}

//---------------------------------------------------------
// create an empty texture
//---------------------------------------------------------
Texture* CreateTexture(const uint width, const uint height)
{
    Texture* tex = new Texture;
    tex->pTex = SDL_CreateTexture(g_Render.pRenderer_,
                                  SDL_PIXELFORMAT_RGBA32,
                                  SDL_TEXTUREACCESS_STATIC,
                                  width,
                                  height);
    if (!tex->pTex)
    {
        LogErr(LOG, "can't create an SDL texture");
        LogErr(LOG, "error info: %s", SDL_GetError());
        delete tex;
        tex = nullptr;
        return nullptr;
    }

    tex->w = (uint16)width;
    tex->h = (uint16)height;

    return tex;
}

//---------------------------------------------------------
// create a texture which can be used as a render target
//---------------------------------------------------------
Texture* CreateRenderTarget(const uint width, const uint height)
{
    Texture* tex = new Texture;
    tex->pTex = SDL_CreateTexture(g_Render.pRenderer_,
                                  SDL_PIXELFORMAT_BGRA8888,
                                  SDL_TEXTUREACCESS_TARGET,
                                  width,
                                  height);
    if (!tex->pTex)
    {
        LogErr(LOG, "can't create an SDL render target");
        LogErr(LOG, "error info: %s", SDL_GetError());
        delete tex;
        tex = nullptr;
        return nullptr;
    }

    tex->w = (uint16)width;
    tex->h = (uint16)height;

    return tex;
}


//---------------------------------------------------------
// setup the blending mode for input texture
//---------------------------------------------------------
void SetTextureBlendMode(Texture* tex, const eBlendMode blendMode)
{
    assert(tex);

    int res = SDL_SetTextureBlendMode(tex->pTex, SDL_BlendMode(blendMode));
    if (res == -1)
        LogErr(LOG, "can't setup a blending mode for a texture (error info: %s)", SDL_GetError());
}

//---------------------------------------------------------
// update input texture with pixels data
//---------------------------------------------------------
bool UpdateTexture(Texture* tex, const Rect* rect, const void* pixels, int pitch)
{
    assert(tex);
    assert(pixels);
    assert(pitch > 0);

    if (rect)
    {
        LogErr(LOG, "wrote an implementation for subrectangle texture update");
        return false;
    }

    int updateRes = SDL_UpdateTexture(tex->pTex, NULL, pixels, pitch);
    if (updateRes != 0)
    {
        LogErr(LOG, "error info: %s", SDL_GetError());
        DestroyTexture(tex);
        return false;
    }

    return true;
}

//---------------------------------------------------------
// setup a color for the input texture
//---------------------------------------------------------
void SetTextureColorMod(Texture* tex, uint8 r, uint8 g, uint8 b)
{
    assert(tex && tex->pTex);
    if (SDL_SetTextureColorMod(tex->pTex, r, g, b) != 0)
    {
        LogErr(LOG, "can't set texture color mod (%d,%d,%d)", (int)r, (int)g, (int)b);
        LogErr(LOG, "SDL err: %s", SDL_GetError());
    }
}

//---------------------------------------------------------
// Desc:  set an additional alpha value multiplied into render copy operations.
// Ret:   0 - success, -1 - error
//---------------------------------------------------------
int SetTextureAlphaMod(Texture* tex, uint8 alpha)
{
    assert(tex);
    if (SDL_SetTextureAlphaMod(tex->pTex, alpha) == -1)
    {
        LogErr(LOG, "can't set texture alpha mod");
        return -1;
    }
    return 0;
}
