/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:  collision.h
    Desc:      2d collision tests
    Created:   22.05.2026 by DimaSkup
\**********************************************************************************/
#ifndef COLLISION_H
#define COLLISION_H

#include "rect.h"
#include "circle.h"
#include "macros.h"

inline bool CollisionRectPoint(const Recti& r, const Vec2i& p)
{
    return ((r.x0 <= p.x) && (p.x <= r.x1) && (r.y0 <= p.y) && (p.y <= r.y1));
}

inline bool CollisionRectPoint(const Rect& r, const Vec2& p)
{
    return ((r.x0 <= p.x) && (p.x <= r.x1) && (r.y0 <= p.y) && (p.y <= r.y1));
}

inline bool CollisionRectiRecti(const Recti& r1, const Recti& r2)
{
    return !(r1.x1 < r2.x0 || r2.x1 < r1.x0) &&
           !(r1.y1 < r2.y0 || r2.y1 < r1.y0);
}

inline bool CollisionCircleCircle(const CircleFixed& a, const CircleFixed& b)
{
    // sqr dist between circles' centers
    const int sqrDist = SQR(a.cx - b.cx) + SQR(a.cy - b.cy);

    return (SQR(a.r + b.r)) >= sqrDist;
}

inline bool CollisionBoxCircle(const Recti& rect, const CircleFixed& c)
{
    // translate coordinates, placing C at origin
    const Vec2i min = { rect.x0-c.cx, rect.y0-c.cy };
    const Vec2i max = { rect.x1-c.cx, rect.y1-c.cy };

    // rect to left of circle center
    if (max.x < 0)   
    {
        if (max.y < 0)                                      // in upper left corner
            return (SQR(max.x) + SQR(max.y)) <= SQR(c.r);

        else if (min.y > 0)                                 // in lower left corner
            return (SQR(max.x) + SQR(min.y)) <= SQR(c.r);

        else                                                // rect due West of circle
            return abs(max.x) <= c.r;
    }
    // rect to right of circle center
    else if (min.x > 0)  
    {
        if (max.y < 0)                                      // in upper right corner
            return (SQR(min.x) + SQR(max.y)) <= SQR(c.r);

        else if (min.y > 0)                                 // in lower right corner
            return (SQR(min.x) + SQR(min.y)) <= SQR(c.r);

        else                                                // rect due East of circle
            return min.x < c.r;
    }
    // rect on circle vertical centerline
    else   
    {
        if (min.y > 0)                                      // rect due South of circle
            return min.y <= c.r;

        else if (max.y < 0)                                 // rect due North of circle
            return abs(max.y) <= c.r;

        else                                                // rect contains circle centerpoint
            return true;
    }
}

#endif
