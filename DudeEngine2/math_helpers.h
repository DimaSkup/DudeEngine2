/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   math_helpers.h
    Created:    06.07.2025  by DimaSkup
\**********************************************************************************/
#ifndef MATH_HELPERS_H
#define MATH_HELPERS_H

#include <math.h>
#include <stdlib.h>
#include "macros.h"

// definition of small numbers
#define EPSILON_E3 (float)(1E-3)
#define EPSILON_E4 (float)(1E-4)
#define EPSILON_E5 (float)(1E-5)
#define EPSILON_E6 (float)(1E-6)

//--------------------------------------------------------------
// Constants
//--------------------------------------------------------------
constexpr float PI          = 3.1415926535897932384626433832795f;
constexpr float M_2PI       = 6.283185307f;
constexpr float M_1DIVPI    = 0.318309886f;
constexpr float M_1DIV2PI   = 0.159154943f;
constexpr float PIDIV2      = 1.570796327f;
constexpr float PIDIV4      = 0.785398163f;

#define PIOVER180  0.0174532925199432957692369076848861f
#define PIUNDER180 57.2957795130823208767981548141052f
#define EPSILON	   1.0e-8
#define SMALL	   1.0e-4f
#define BIG		   1.0e+10f


// ----------------------------------------------------

inline static float clampf(const float x, const float lo, const float hi)
{
    return x < lo ? lo : (x > hi ? hi: x);
}

// ----------------------------------------------------
template<typename T>
inline static T Clamp(const T& x, const T& low, const T& high)
{
    return x < low ? low : (x > high ? high : x);
}

//---------------------------------------------------------
// clamp pitch / yaw
//---------------------------------------------------------
inline float ClampPitch(const float pitch)
{
    if (pitch > PIDIV2 - 0.1f)   return PIDIV2 - 0.1f;
    if (pitch < -PIDIV2 + 0.1f)  return -PIDIV2 + 0.1f;

    return pitch;
}

//---------------------------------------------------------

inline float ClampYaw(const float yaw)
{
    if (yaw > M_2PI)   return -M_2PI;
    if (yaw < -M_2PI)  return M_2PI;

    return yaw;
}

// ----------------------------------------------------

inline int powi(const int value, const int power)
{
    return (int)exp(log(value) * power);
}

// ----------------------------------------------------

inline float frac(const float f)
{
    return f - floorf(f);
}

// ----------------------------------------------------

inline float lerp(const float a, const float b, const float factor)
{
    return (a * factor + (1.0f - factor) * b);
}

// ----------------------------------------------------

inline int FastMin(const int x, const int y)
{
    return y ^ ((x ^ y) & -(x < y)); // min(x, y)
}

// ----------------------------------------------------

inline int FastMax(const int x, const int y)
{
    return x ^ ((x ^ y) & -(x < y)); // max(x, y)
}

// ----------------------------------------------------

template <class T>
inline T Min(const T x, const T y)
{
    return (x < y) ? x : y;
}

// ----------------------------------------------------

template <class T>
inline T Max(const T x, const T y)
{
    return (x > y) ? x : y;
}

//---------------------------------------------------------
// a little helper to check if 2 input floats are "equal"
//---------------------------------------------------------
inline bool FloatEqual(const float a, const float b)
{
    return fabsf(a - b) < EPSILON_E5;
}

//---------------------------------------------------------
// random value in range
//---------------------------------------------------------
inline int RandU(const unsigned int min, const unsigned int max)
{
    return min + rand() / RAND_MAX * (max - min);
}

inline float RandF(const float min, const float max)
{
    return min + (float)rand() / (float)RAND_MAX * (max - min);
}

#endif