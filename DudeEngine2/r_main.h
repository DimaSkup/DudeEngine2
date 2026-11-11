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

    bool           Init();
    void           Shutdown();

    bool           CreateViewportRenderTarget(const uint w, const uint h);
    Texture*       CreateRenderTarget(const uint w, const uint h);
    const Texture* GetCurrentRenderTarget() const;
    void           BindRenderTarget(const Texture* pRenderTarget);
    void           UnbindRenderTargets();

    void           SetFullscreen(bool bFullscreen);
    void           ShowSysCursor(bool showHide);

    SDL_Window*    GetWindow();
    SDL_Renderer*  GetRenderer();
    Texture*       GetViewportRenderTarget();

    Color          SampleTexel(const Texture* pTex, const uint texX, const uint texY);
                   
    Vec2i          GetFullscreenSize() const;
    Vec2i          GetWndSize() const;
    Recti          GetWndRecti() const;
    
    void           BeginFrame();
    void           RenderPresent();
                   
    void           SetRenderDrawBlendMode(const eBlendMode mode);
    void           SetDrawColor(const uint8 r, const uint8 g, const uint8 b, const uint8 a);
                   
    void           SetDrawScale(const uint8 s);
    int            GetDrawScale(void) const;
                   
    void           DrawFillRect(const Recti& rect);
    void           DrawFillRect(int x0, int y0, int x1, int y1);
    void           DrawRect(int x0, int y0, int x1, int y1);
                   
    void           DrawPixel(int x, int y);
    void           DrawLine(int x1, int y1, int x2, int y2);
    void           DrawCircle(int cx, int cy, int r);


    void DrawRect(Texture* pTex,
                  const Recti& srcRect,
                  const Recti& dstRect,
                  const Color rgba,
                  const uint32 flip,
                  const Vec2i* center,
                  const float angle);

public:
    SDL_Window*    pWindow_;
    SDL_Renderer*  pRenderer_;
    const Texture* pCurrRenderTarget_;       // currently used render target (NULL if the default one is used)
    Texture*       pViewportRenderTarget_;   // is used to visualize scene in the editor
    uint8          scale_;                   // we can scale down the whole world in 2,4,8 times, so save the current value of scaling (1 if no scaling is used now)
    bool           bFullscreen_;
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

inline const Texture* CRender::GetCurrentRenderTarget() const
{
    return pCurrRenderTarget_;
}

inline Texture* CRender::GetViewportRenderTarget()
{
    return pViewportRenderTarget_;
}

inline int CRender::GetDrawScale(void) const
{
    return scale_;
}

#endif
