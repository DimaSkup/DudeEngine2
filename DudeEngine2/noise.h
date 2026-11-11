/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: noise.h
    Desc:     generation of different types of noises
    Created:  07.08.2026 by DimaSkup
\**********************************************************************************/
#ifndef NOISE_H
#define NOISE_H

#include "math_helpers.h"
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

class Noise
{
public:
    // params for a Perlin Noise
    struct PerlinConfig
    {
        uint16_t width            = 256;
        uint16_t height           = 256;
        float    lacunarity       = 2.0f;       // multiply frequency of octave by this value to make each next octave be unique
        float    persistence      = 0.5f;       // if < 1 it reduces amplitude of each next octave by this factor
        uint32_t seed             : 8  = 0;     // seed for randomization
        uint32_t octaves          : 5  = 4;      
        uint32_t scale            : 15 = 50;
        uint32_t bNormalize       : 1  = 1;     // bool
        uint32_t bUseFirstTypeGen : 1  = 1;     // bool
        uint32_t bInverseColor    : 1  = 0;     // bool
    };

    // params for a Voronoi Noise
    struct VoronoiConfig
    {
        uint16_t width            = 256;
        uint16_t height           = 256;
        float    powerMid         = 8.0f;
        float    powerFinal       = 0.0625f;
        uint32_t seed             : 8  = 0;     // seed for randomization
        uint32_t scale            : 16 = 50;
        uint32_t bInverseColor    : 1  = 0;     // bool
    };

    struct vec2
    {
        float x, y;

        vec2(float _x, float _y) : x(_x), y(_y) {};
        vec2() {};
    };

public:
    enum 
    {
        MAX_OCTAVES = 16,
        TABLE_SIZE = 256,
        TABLE_MASK = TABLE_SIZE-1,
    };

    Noise();
    ~Noise();

    // white noise generation
    static float Hash(const float x, const float y);
    
    // value noise generation
    static float Value(const float x, const float y);

    // classic Perlin noise (returns a result in range [-sqrt(2)/2, +sqrt(2)/2])
    static float Perlin(const int x,   const int y);
    static float Perlin(const float x, const float y);

    // uses octaves to make a better result
    static float PerlinFBM(const float x, const float y, const PerlinConfig& cfg);

    // GLM-like implementation (returns a result in range [-1,1])
    static float Perlin2(float x, float y);

    // Voronoi noise generation
    static float Voronoi(const float x, const float y, const VoronoiConfig& cfg);

private:
    // precomputed params for Perlin Noise
    static vec2          table_[TABLE_SIZE];
    static unsigned char lut_[TABLE_SIZE];
    static vec2          octaveOffsets_[Noise::MAX_OCTAVES];

private:
    static void Setup();
    static const vec2& GetVec(const int x, const int y);
};

//---------------------------------------------------------

inline void Noise::Setup()
{
    float step = 6.24f / (float)TABLE_SIZE;
    float val = 0.0f;

    for (int i = 0; i < TABLE_SIZE; ++i)
    {
        table_[i].x = sinf(val);
        table_[i].y = cosf(val);
        val += step;

        lut_[i] = rand() & TABLE_MASK;
    }

    for (int i = 0; i < MAX_OCTAVES; ++i)
    {
        octaveOffsets_[i].x = (float)(rand() % 200000 - 100000);
        octaveOffsets_[i].y = (float)(rand() % 200000 - 100000);
    }
}

//---------------------------------------------------------

inline const Noise::vec2& Noise::GetVec(const int x, const int y)
{
    const unsigned char a   = lut_[x & TABLE_MASK];
    const unsigned char b   = lut_[y & TABLE_MASK];
    const unsigned char val = lut_[(a + b) & TABLE_MASK];
    return table_[val];
}

//---------------------------------------------------------
// Desc:  generate a pseudo-random value based on a 2D coordinate
// Ret:   value in range (0,1)
//---------------------------------------------------------
inline float Noise::Hash(const float x, const float y)
{
    return frac(sinf(x * 12.9898f + y * 78.233f) * 43758.5453f);
}

//---------------------------------------------------------
// value noise generator
//---------------------------------------------------------
inline float Noise::Value(const float x, const float y)
{
    // integer coords of the grid cell
    const float ix = floorf(x);
    const float iy = floorf(y);

    // fractional coords within the grid cell
    const float fx = frac(x);
    const float fy = frac(y);

    // smoothstep function for interpolation
    const float ux = fx * fx * (3.0f - 2.0f * fx);
    const float uy = fy * fy * (3.0f - 2.0f * fy);

    // hash values for the four corners of the grid cell
    const float a = Hash(ix, iy);
    const float b = Hash(ix + 1.0f, iy);
    const float c = Hash(ix + 0.0f, iy + 1.0f);
    const float d = Hash(ix + 1.0f, iy + 1.0f);

    // bilinear interpolation
    return lerp(lerp(a, b, ux), lerp(c, d, ux), uy);
}

//---------------------------------------------------------
// Desc:  generate a Perlin Noise
// Ret:   a result in range [-sqrt(2)/2, +sqrt(2)/2]
//---------------------------------------------------------
inline float Noise::Perlin(const float x, const float y)
{
    const float x0 = floorf(x);
    const float x1 = x0 + 1.0f;
    const float y0 = floorf(y);
    const float y1 = y0 + 1.0f;

    const int ix0 = (int)x0;
    const int ix1 = (int)x1;
    const int iy0 = (int)y0;
    const int iy1 = (int)y1;

    const vec2& v0 = GetVec(ix0, iy0);
    const vec2& v1 = GetVec(ix0, iy1);
    const vec2& v2 = GetVec(ix1, iy0);
    const vec2& v3 = GetVec(ix1, iy1);

    const vec2 d0 = {x-x0, y-y0};
    const vec2 d1 = {x-x0, y-y1};
    const vec2 d2 = {x-x1, y-y0};
    const vec2 d3 = {x-x1, y-y1};

    const float h0 = (d0.x * v0.x) + (d0.y * v0.y);
    const float h1 = (d1.x * v1.x) + (d1.y * v1.y);
    const float h2 = (d2.x * v2.x) + (d2.y * v2.y);
    const float h3 = (d3.x * v3.x) + (d3.y * v3.y);

    float sx, sy;

/*
    Perlin's original equation was faster, 
    but produced artifacts in some situations:
    sx = (3*powf(d0.x, 2.0f))
        -(2*powf(d0.x, 3.0f));

    sx = (3*powf(d0.y, 2.0f))
        -(2*powf(d0.y, 3.0f));
*/

    // the revised blend equation is 
    // considered more ideal, but is 
    // slower to compute
    sx = (6*powf(d0.x,5.0f))
        -(15*powf(d0.x,4.0f))
        +(10*powf(d0.x,3.0f));

    sy = (6*powf(d0.y,5.0f))
        -(15*powf(d0.y,4.0f))
        +(10*powf(d0.y,3.0f));

    const float avgX0 = h0 + (sx * (h2-h0));
    const float avgX1 = h1 + (sx * (h3-h1));

    return avgX0 + (sy * (avgX1-avgX0));
}

//---------------------------------------------------------

inline float Noise::Perlin(const int x, const int y)
{
    return Perlin((float)x, (float)y);
}

//---------------------------------------------------------
// more complex noise generation using octaves
//---------------------------------------------------------
inline float Noise::PerlinFBM(const float x, const float y, const PerlinConfig& cfg)
{
    srand(cfg.seed);

    // prevent division by 0 or wrong scale value
    float scale = 0.0001f;
    if (cfg.scale > 0.0f)
        scale = 1.0f / cfg.scale;

    float sum = 0.0f;
    float freq = 1.0f;
    float amp = 1.0f;

    for (uint32_t oct = 0; oct < cfg.octaves; ++oct)
    {
        // pos * scale * frequency + octave_offset
        const float sx = x * scale * freq + octaveOffsets_[oct].x;
        const float sy = y * scale * freq + octaveOffsets_[oct].y;

        sum += Noise::Perlin(sx, sy) * amp;
        freq *= cfg.lacunarity;
        amp *= cfg.persistence;
    }

    return sum;
}

//---------------------------------------------------------
// Voronoi noise generation
//---------------------------------------------------------
inline float Noise::Voronoi(const float x, const float y, const VoronoiConfig& cfg)
{
    const float px = floorf(x);
    const float py = floorf(y);

    const float fx = frac(x);
    const float fy = frac(y);

    float res = 0.0f;

    for (int j = -1; j <= 1; ++j)
    {
        for (int i = -1; i <= 1; ++i)
        {
            const float bx = (float)(i);
            const float by = (float)(j);
            const float h = Hash(px+bx, py+by);

            const float rx = bx - fx + h;
            const float ry = by - fy + h;
            res += (1.0f / powf(rx*rx + ry*ry, cfg.powerMid));
        }
    }

    return powf(1.0f / res, cfg.powerFinal);
}

//---------------------------------------------------------
// default constructor
//---------------------------------------------------------
inline Noise::Noise()
{
    Setup();
}

//---------------------------------------------------------
// default destructor
//---------------------------------------------------------
inline Noise::~Noise()
{
}

#endif // NOISE_H