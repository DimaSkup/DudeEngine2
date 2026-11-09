/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: r_main.cpp
    Created:  by DimaSkup
\**********************************************************************************/
#include "r_main.h"
#include "log.h"
#include "game_state.h"
#include "consts.h"
#include <assert.h>


//---------------------------------------------------------
// global instance
//---------------------------------------------------------
CRender g_Render;

//---------------------------------------------------------
// constructor / destructor
//---------------------------------------------------------
CRender::CRender() :
    pWindow_(nullptr),
    pRenderer_(nullptr),
    pTexRenderTarget_(nullptr),
    scale_(1),
    bFullscreen_(false)
{
}

//---------------------------------------------------------

CRender::~CRender()
{
    Shutdown();
    pWindow_ = nullptr;
    pRenderer_ = nullptr;
}



//---------------------------------------------------------
// Desc:  initialize the renderer
//---------------------------------------------------------
bool CRender::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        LogErr(LOG, "can't init SDL");
        return false;
    }

    SDL_DisplayMode dm;

    if (SDL_GetCurrentDisplayMode(0, &dm) != 0)
    {
        LogErr(LOG, "SDL_GetCurrentDisplayMode error: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    // for using RenderDoc on Windows (because fucking SDL2 enforces OpenGL 2.1 and RenderDoc fucks up)
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11");

    pWindow_ = SDL_CreateWindow(
        "DudeEngine2",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        dm.w,
        dm.h,
        SDL_WINDOW_SHOWN);
    if (!pWindow_)
    {
        LogErr(LOG, "can't create a window");
        Shutdown();
        return false;
    }

    pRenderer_ = SDL_CreateRenderer(pWindow_, -1, SDL_RENDERER_ACCELERATED);
    if (!pRenderer_)
    {
        LogErr(LOG, "can't create a renderer");
        Shutdown();
        return false;
    }

    // create a render target to render scene into (used as a viewport in the editor)
    const Vec2i wndSize = GetWndSize();
    if (!g_Render.CreateViewportRenderTarget(wndSize.w, wndSize.h))
    {
        LogErr(LOG, "can't create a render target for game/editor viewport");
        Shutdown();
        return false;
    }

    // by default we're in the windowed mode
    SetFullscreen(false);

    return true;
}

//---------------------------------------------------------
// create a render target which is used to render scene into
//---------------------------------------------------------
bool CRender::CreateViewportRenderTarget(int wndWidth, int wndHeight)
{
    assert(pTexRenderTarget_ == nullptr);

    if (wndWidth <= 0 || wndHeight <= 0)
    {
        LogErr(LOG, "invalid window dimensions (w: %d, h: %d)", wndWidth, wndHeight);
        return false;
    }

    // create a render target
    pTexRenderTarget_ = CreateRenderTarget(wndWidth, wndHeight);
    if (!pTexRenderTarget_)
    {
        LogErr(LOG, "can't create a render target");
        Shutdown();
        return false;
    }

    return true;
}

//---------------------------------------------------------
// shutdown the renderer
//---------------------------------------------------------
void CRender::Shutdown()
{
    // shutdown SDL stuff
    if (pTexRenderTarget_)
        DestroyTexture(pTexRenderTarget_);

    SDL_DestroyRenderer(pRenderer_);
    SDL_DestroyWindow(pWindow_);
    SDL_Quit();
}

//---------------------------------------------------------
// get dimensions of the current screen/monitor
//---------------------------------------------------------
Vec2i CRender::GetFullscreenSize() const
{
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0)
    {
        LogErr(LOG, "can't get fullscreen size because: %s", SDL_GetError());
        return Vec2i{ 0,0 };
    }

    return Vec2i{ dm.w, dm.h };
}

//---------------------------------------------------------
// get dimensions of the current window
//---------------------------------------------------------
Vec2i CRender::GetWndSize() const
{
    if (!pWindow_)
    {
        LogErr(LOG, "your ptr to window == NULL, create a window first!");
        return { 1, 1 };
    }

    Vec2i size;
    SDL_GetWindowSize(pWindow_, &size.w, &size.h);
    return size;
}

//---------------------------------------------------------
// get a rectangle of the main window (x0,y0 -- top-left; x1,y1 -- bottom right)
//---------------------------------------------------------
Recti CRender::GetWndRecti() const
{
    if (!pWindow_)
    {
        LogErr(LOG, "your ptr to window == NULL, create a window first!");
        return { 1, 1 };
    }

    Vec2i size;
    SDL_GetWindowSize(pWindow_, &size.w, &size.h);
    return Recti{ 0, size.w, 0, size.h };
}

//---------------------------------------------------------
// attach a render target to render scene into
//---------------------------------------------------------
void CRender::AttachRenderTarget()
{
    SDL_SetRenderTarget(pRenderer_, pTexRenderTarget_->pTex);
    SDL_SetRenderDrawColor(pRenderer_, 255, 255, 255, 255);
    SDL_RenderClear(pRenderer_);
}

//---------------------------------------------------------
// detach render targets, reset to default
//---------------------------------------------------------
void CRender::DetachRenderTarget()
{
    // reset to default render target
    SDL_SetRenderTarget(pRenderer_, NULL);
}

//---------------------------------------------------------
// switch btw windowed and fullscreen modes
//---------------------------------------------------------
void CRender::SetFullscreen(bool bFullscreen)
{
    assert(pWindow_);

    // switch to fullscreen (used in the game mode)
    if (bFullscreen)
    {
        SDL_DisplayMode dm;
        SDL_SetWindowBordered(pWindow_, SDL_FALSE);

        if (SDL_GetCurrentDisplayMode(0, &dm) != 0)
        {
            LogErr(LOG, "SDL_GetCurrentDisplayMode error: %s", SDL_GetError());
            Shutdown();
            SDL_Quit();
            return;
        }

        SDL_SetWindowPosition(pWindow_, 0, 0);
        SDL_SetWindowSize(pWindow_, dm.w, dm.h);
    }

    // switch to windowed (used in the editor mode)
    else
    {
        SDL_SetWindowBordered(pWindow_, SDL_TRUE);

        SDL_Rect rect;
        if (SDL_GetDisplayUsableBounds(0, &rect) != 0)
        {
            LogErr(LOG, "can't SDL_GetDisplayUsableBounds");
            return;
        }

        int top, left, bottom, right;
        if (SDL_GetWindowBordersSize(pWindow_, &top, &left, &bottom, &right) != 0)
        {
            LogErr(LOG, "can't SDL_GetWindowBordersSize");
            return;
        }
       
        SDL_SetWindowPosition(pWindow_, 0, top);
        SDL_SetWindowSize(pWindow_, rect.w, rect.h-50);
    }
}

//---------------------------------------------------------
// handle beginning of a new frame
//---------------------------------------------------------
void CRender::BeginFrame()
{
    SDL_RenderClear(pRenderer_);
}

//---------------------------------------------------------
// present stuff onto the screen
//---------------------------------------------------------
void CRender::RenderPresent()
{
    SDL_RenderPresent(pRenderer_);
}

//---------------------------------------------------------
// set the blend mode used for drawing operations (Fill and Line).
//---------------------------------------------------------
void CRender::SetRenderDrawBlendMode(const eBlendMode mode)
{
    SDL_BlendMode modeToSet = SDL_BLENDMODE_INVALID;

    switch (mode)
    {
        case BLENDMODE_NONE:   modeToSet = SDL_BLENDMODE_NONE;  break;
        case BLENDMODE_BLEND:  modeToSet = SDL_BLENDMODE_BLEND; break;
        case BLENDMODE_ADD:    modeToSet = SDL_BLENDMODE_ADD;   break;
        case BLENDMODE_MOD:    modeToSet = SDL_BLENDMODE_MOD;   break;
        case BLENDMODE_MUL:    modeToSet = SDL_BLENDMODE_MUL;   break;
        default:
            LogErr(LOG, "unknown blend mode: %d", mode);
    }

    if (modeToSet != SDL_BLENDMODE_INVALID)
        SDL_SetRenderDrawBlendMode(pRenderer_, modeToSet);
}

//---------------------------------------------------------
// setup a color for drawing SDL primitives
//---------------------------------------------------------
void CRender::SetDrawColor(const uint8 r, const uint8 g, const uint8 b, const uint8 a)
{
    SDL_SetRenderDrawColor(pRenderer_, r, g, b, a);
}

//---------------------------------------------------------
// setup a visual scale of the rendering
//---------------------------------------------------------
void CRender::SetDrawScale(const uint8 s)
{
    assert(s > 0);
    scale_ = s;
}

//---------------------------------------------------------
// render filled rectangle onto the screen;
// setup its color using SetDrawColor()
//---------------------------------------------------------
void CRender::DrawFillRect(const Recti& r)
{
    const SDL_Rect rect = { r.x0, r.y0, r.x1-r.x0, r.y1-r.y0 };
    SDL_RenderFillRect(pRenderer_, &rect);
}

//---------------------------------------------------------

void CRender::DrawFillRect(int x0, int y0, int x1, int y1)
{
    const SDL_Rect rect = { x0, y0, x1-x0, y1-y0 };
    SDL_RenderFillRect(pRenderer_, &rect);
}

//---------------------------------------------------------
// draw a single pixel at (x,y);
// setup its color using SetDrawColor()
//---------------------------------------------------------
void CRender::DrawPixel(int x, int y)
{
    SDL_RenderDrawPoint(pRenderer_, x, y);
}

//---------------------------------------------------------
// draw a circle with center at (cx,cy) and radius r;
// setup its color using SetDrawColor()
//---------------------------------------------------------
void CRender::DrawCircle(int cx, int cy, int r)
{
    int d = 3 - (2 * r);
    int x = 0;
    int y = r;

    // draw all 8 pixels present at symmetric position
    DrawPixel(cx + x, cy + y);
    DrawPixel(cx + y, cy + x);
    DrawPixel(cx + y, cy - x);
    DrawPixel(cx + x, cy - y);

    DrawPixel(cx - x, cy - y);
    DrawPixel(cx - y, cy - x);
    DrawPixel(cx - y, cy + x);
    DrawPixel(cx - x, cy + y);

    while (y >= x)
    {
        if (d > 0)
        {
            y--;
            d = d + 4*(x - y) + 10;
        }
        else
            d = d + (4 * x) + 6;

        x++;

        // draw all 8 pixels present at symmetric position
        DrawPixel(cx + x, cy + y);
        DrawPixel(cx + y, cy + x);
        DrawPixel(cx + y, cy - x);
        DrawPixel(cx + x, cy - y);

        DrawPixel(cx - x, cy - y);
        DrawPixel(cx - y, cy - x);
        DrawPixel(cx - y, cy + x);
        DrawPixel(cx - x, cy + y);
    }

}

//---------------------------------------------------------
// draw a line from (x1,y1) to (x2,y2);
// setup its color using SetDrawColor()
//---------------------------------------------------------
void CRender::DrawLine(int x1, int y1, int x2, int y2)
{
    SDL_RenderDrawLine(pRenderer_, x1, y1, x2, y2);
}

//---------------------------------------------------------
// draw a rectangle with min point (x0,y0) and max point (x1,y1);
// setup its color using SetDrawColor()
//---------------------------------------------------------
void CRender::DrawRect(int x0, int y0, int x1, int y1)
{
    g_Render.DrawLine(x0, y0, x1, y0);   // upper horizontal
    g_Render.DrawLine(x1, y0, x1, y1);   // right vertical
    g_Render.DrawLine(x0, y1, x1, y1);   // bottom horizontal
    g_Render.DrawLine(x0, y0, x0, y1);   // left vertical
}

//---------------------------------------------------------
// Desc:  draw textured rectangle onto the screen
// Args:  srcRect - params of the original texture (pos + size)
//        dstRect - params where to render this texture (pos + size)
//        center  - rotation center (can be NULL)
//        angle   - rotation angle
//---------------------------------------------------------
void CRender::DrawRect(Texture* pTex,
                       const Recti& srcRect,
                       const Recti& dstRect,
                       const Color color,
                       const uint32 flip,
                       const Vec2i* center,
                       const float angle)
{
    assert(pTex);
    assert(flip >= 0 && flip < NUM_RENDER_FLIP_MODES);

    // setup color mode (tint/alpha)
    SetTextureColorMod(pTex, color.r, color.g, color.b);
    SetTextureAlphaMod(pTex, color.a);

    const SDL_Rect src = { srcRect.x0, srcRect.y0, srcRect.x1 - srcRect.x0, srcRect.y1 - srcRect.y0 };
    SDL_Rect dst = { dstRect.x0, dstRect.y0, dstRect.x1 - dstRect.x0, dstRect.y1 - dstRect.y0 };

    switch (scale_)
    {
        // render the scene 2 times smaller
        case 2:
            dst.x >>= 1;
            dst.y >>= 1;
            dst.w >>= 1;
            dst.h >>= 1;
            break;
        // render the scene 4 times smaller
        case 4:
            dst.x >>= 2;
            dst.y >>= 2;
            dst.w >>= 2;
            dst.h >>= 2;
            break;

        // render the scene 8 times smaller
        case 8:
            dst.x >>= 3;
            dst.y >>= 3;
            dst.w >>= 3;
            dst.h >>= 3;
            break;
    }

    SDL_Point rotCenter;
    
    if (center)
        rotCenter = { center->x, center->y };
    else
        rotCenter = { dst.w >> 1, dst.h >> 1 };


    SDL_RenderCopyEx(pRenderer_,
                     pTex->pTex, 
                     &src, &dst, 
                     angle, 
                     &rotCenter, 
                     SDL_RendererFlip(flip));
}

//---------------------------------------------------------
// show/hide the system mouse cursor
//---------------------------------------------------------
void CRender::ShowSysCursor(const bool showHide)
{
    SDL_ShowCursor(showHide);
}