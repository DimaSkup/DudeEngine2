#include "noise.h"

Noise::vec2   Noise::table_[TABLE_SIZE];
unsigned char Noise::lut_[TABLE_SIZE];
Noise::vec2   Noise::octaveOffsets_[Noise::MAX_OCTAVES];


static inline float mod289(float x)
{
    return x - floorf(x * (1.0f / 289.0f)) * 289.0f;
}

static inline float permute(float x)
{
    return mod289(((x * 34.0f) + 1.0f) * x);
}

static inline float fade(float t)
{
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static inline float mix(float a, float b, float t)
{
    return a + (b - a) * t;
}

static inline float grad(float hash, float x, float y, float z)
{
    int h = static_cast<int>(hash) & 15;

    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);

    return ((h & 1) ? -u : u) +
        ((h & 2) ? -v : v);
}

float Noise::Perlin2(float x, float y)
{
    float X = floorf(x);
    float Y = floorf(y);
    float Z = 0;

    x -= X;
    y -= Y;
    float z = 0;

    X = mod289(X);
    Y = mod289(Y);
    Z = mod289(Z);

    float A = permute(X) + Y;
    float AA = permute(A) + Z;
    float AB = permute(A + 1.0f) + Z;
    float B = permute(X + 1.0f) + Y;
    float BA = permute(B) + Z;
    float BB = permute(B + 1.0f) + Z;

    float u = fade(x);
    float v = fade(y);
    float w = fade(z);

    return mix(
        mix(
            mix(grad(permute(AA), x, y, z),
                grad(permute(BA), x - 1, y, z), u),
            mix(grad(permute(AB), x, y - 1, z),
                grad(permute(BB), x - 1, y - 1, z), u),
            v),
        mix(
            mix(grad(permute(AA + 1), x, y, z - 1),
                grad(permute(BA + 1), x - 1, y, z - 1), u),
            mix(grad(permute(AB + 1), x, y - 1, z - 1),
                grad(permute(BB + 1), x - 1, y - 1, z - 1), u),
            v),
        w);
}