/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   vec.h
    Desc:       2d vectors and related functional
    Created:    by DimaSkup
\**********************************************************************************/
#ifndef VEC_H
#define VEC_H

#include <math.h>
#include <assert.h>

// helper macros
#define SQR(n)  ((n)*(n))

//---------------------------------------------------------
// a vector of 2 floats
//---------------------------------------------------------
class Vec2
{
public:
    union {
        struct { float x, y; };
        struct { float u, v; };
        struct { float w, h; };
    };

public:
    Vec2() {}
    Vec2(const float x_, const float y_) : x(x_), y(y_) {}
};

//---------------------------------------------------------
// a vector of 2 integers
//---------------------------------------------------------
class Vec2i
{
public:
    union {
        struct { int x, y; };
        struct { int w, h; };
    };

public:
    Vec2i() {}
    Vec2i(int i) : x(i), y(i) {}
    Vec2i(int x_, int y_) : x(x_), y(y_) {}
};


//*****************************************************************************
// VECTORS FUNCTIONS
//*****************************************************************************

inline Vec2 ToVec2(Vec2i v)
{
    return { (float)v.x, (float)v.y };
}

inline float Vec2Dot(const Vec2& v)
{
    return (v.x*v.x + v.y*v.y);
}
inline float Vec2Dot(const Vec2& va, const Vec2& vb)
{
    return (va.x*vb.x + va.y*vb.y);
}

inline bool Vec2Eq(const Vec2& v1, const Vec2& v2)
{
    return (fabsf(v1.x-v2.x) < 1E-5f &&
            fabsf(v1.y-v2.y) < 1E-5f);
}

inline Vec2i ToVec2i(Vec2 v)
{
    return { (int)v.x, (int)v.y };
}

inline Vec2 Vec2Scale(const Vec2& v, float scale)
{
    return { v.x * scale, v.y * scale };
}

inline Vec2i Vec2iScale(Vec2i v, int scale)
{
    return { v.x * scale, v.y * scale };
}

inline void Vec2Div(Vec2& v, float d)
{
    assert (fabsf(d) > (float)(1E-5));
    
    const float inv = 1.0f / d;
    v.x *= d;
    v.y *= d;
}

inline Vec2 Vec2Div(const Vec2& v, float d)
{
    assert(fabs(d) > (float)(1E-5));

    const float inv = 1.0f / d;
    return { v.x * inv, v.y * inv };
}

inline Vec2i Vec2iDiv(const Vec2i& v, int d)
{
    assert(d != 0);
    return Vec2i{ v.x / d, v.y / d };
}

inline Vec2 Vec2Add(const Vec2& v1, const Vec2& v2)
{
    return { v1.x+v2.x, v1.y+v2.y };
}

inline Vec2i Vec2iAdd(const Vec2i& v1, const Vec2i& v2)
{
    return { v1.x+v2.x, v1.y+v2.y };
}

inline Vec2i Vec2iAdd(const Vec2i& v, const int d)
{
    return { v.x + d, v.y + d };
}

inline void Vec2iAdd(Vec2i& v, const int d)
{
    v.x += d;
    v.y += d;
}

inline Vec2 Vec2Sub(const Vec2& v1, const Vec2& v2)
{
    return { v1.x-v2.x, v1.y-v2.y };
}

inline Vec2i Vec2iSub(Vec2i v1, Vec2i v2)
{
    return { v1.x-v2.x, v1.y-v2.y };
}

inline void Vec2Clamp(Vec2& v, const Vec2& lo, const Vec2& hi)
{
    v.x = (v.x < lo.x) ? lo.x : ((v.x > hi.x) ? hi.x : v.x);
    v.y = (v.y < lo.y) ? lo.y : ((v.y > hi.y) ? hi.y : v.y);
}

inline Vec2i Vec2iClamp(const Vec2i& v, const Vec2i& lo, const Vec2i& hi)
{
    return { (v.x < lo.x) ? lo.x : ((v.x > hi.x) ? hi.x : v.x),
             (v.y < lo.y) ? lo.y : ((v.y > hi.y) ? hi.y : v.y) };
}

inline Vec2 Vec2Mul(const Vec2& v, const float s)
{
    return { v.x * s, v.y * s };
}

inline Vec2i Vec2iMul(const Vec2i& v, const int s)
{
    return { v.x * s, v.y * s };
}


inline float Vec2Length(const Vec2& v)
{
    return sqrtf(v.x*v.x + v.y*v.y);
}

inline void Vec2Normalize(Vec2& v)
{
    const float len = sqrtf(SQR(v.x) + SQR(v.y));

    assert(len > (float)(1E-5));

    const float inv = 1.0f / len;
    v.x *= inv;
    v.y *= inv;
}

inline void Vec2Normalize(const Vec2* src, Vec2* dst)
{
    assert(src);
    assert(dst);

    const float len = sqrtf(SQR(src->x) + SQR(src->y));
    assert(len > (float)(1E-5));                        // avoid dividing by zero

    const float inv = 1.0f / len;
    dst->x = src->x * inv;
    dst->y = src->y * inv;
}

inline float Vec2SqrDist(const Vec2& v)
{
    return Vec2Dot(v);
}

// consider two input vectors as a points to calc distance between them
inline float Vec2SqrDist(const Vec2* v1, const Vec2* v2)
{
    assert(v1 && v2);
    return (SQR(v1->x - v2->x) + SQR(v1->y - v2->y));
}

// check if two input vectors are equal
inline bool Vec2iEq(const Vec2i* v1, const Vec2i* v2)
{
    assert(v1 && v2);
    return ((v1->x == v2->x) && (v1->y == v2->y));
}

#endif
