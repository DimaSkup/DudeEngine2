/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: rect.h
    Desc:     2D rectangle and related functional
    Created:  22.05.2026 by DimaSkup
\**********************************************************************************/
#ifndef RECT_H
#define RECT_H

#include "vec.h"

struct Rect
{
    float x0, x1, y0, y1;
};

struct Recti
{
    int x0, x1, y0, y1;
};

inline Rect RectInit(const Vec2& pos, const Vec2& sz)
{
    return { pos.x, pos.x+sz.x, pos.y, pos.y+sz.y };
}

inline Recti RectiInit(const Vec2i& pos, const Vec2i& sz)
{
    return { pos.x, pos.x+sz.x, pos.y, pos.y+sz.y };
}

inline Rect ToRect(const Recti& r)
{
    return { (float)r.x0, (float)r.x1, (float)r.y0, (float)r.y1 };
}

inline Recti ToRecti(const Rect& r)
{
    return { (int)r.x0, (int)r.x1, (int)r.y0, (int)r.y1 };
}

#endif