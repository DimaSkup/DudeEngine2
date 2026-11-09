/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   r_main.h
    Desc:       a wrapper for rendering stuff
    Created:    by DimaSkup
\**********************************************************************************/
#ifndef R_MAIN_H
#define R_MAIN_H

#include "types.h"
#include "vec.h"
#include "texture.h"
#include <SDL.h>


//---------------------------------------------------------
// all the rendering specific stuff is here
//---------------------------------------------------------
class CRender
{
public:
    CRender();
    ~CRender();

    bool Init();
    void Shutdown();

    bool CreateViewportRenderTarget(int wndWidth, int wndHeight);
    void AttachRenderTarget();
    void DetachRenderTarget();

    void SetFullscreen(bool bFullscreen);
    void ShowSysCursor(bool showHide);

    SDL_Window*   GetWindow();
    SDL_Renderer* GetRenderer();
    Texture*      GetRenderTarget();

    Vec2i GetFullscreenSize() const;
    Vec2i GetWndSize() const;
    Recti GetWndRecti() const;
    
    void BeginFrame();
    void RenderPresent();

    void SetRenderDrawBlendMode(const eBlendMode mode);
    void SetDrawColor(const uint8 r, const uint8 g, const uint8 b, const uint8 a);

    void SetDrawScale(const uint8 s);
    int  GetDrawScale(void) const;

    void DrawFillRect(const Recti& rect);
    void DrawFillRect(int x0, int y0, int x1, int y1);
    void DrawRect(int x0, int y0, int x1, int y1);

    void DrawPixel(int x, int y);
    void DrawLine(int x1, int y1, int x2, int y2);
    void DrawCircle(int cx, int cy, int r);


    void DrawRect(Texture* pTex,
                  const Recti& srcRect,
                  const Recti& dstRect,
                  const Color rgba,
                  const uint32 flip,
                  const Vec2i* center,
                  const float angle);

public:
    SDL_Window*   pWindow_;
    SDL_Renderer* pRenderer_;
    Texture*      pTexRenderTarget_;
    uint8         scale_;
    bool          bFullscreen_;
};

//---------------------------------------------------------
// global instance (why not?)
//---------------------------------------------------------
extern CRender g_Render;

//---------------------------------------------------------
// inline functions
//---------------------------------------------------------
inline SDL_Window* CRender::GetWindow()
{
    return pWindow_;
}

inline SDL_Renderer* CRender::GetRenderer()
{
    return pRenderer_;
}

inline Texture* CRender::GetRenderTarget()
{
    return pTexRenderTarget_;
}

inline int CRender::GetDrawScale(void) const
{
    return scale_;
}

#endif
