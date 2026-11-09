/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: ui_noise_gen.h
    Desc:     UI/editor generators of different types of noises
    Created:  09.08.2026 by DimaSkup
\**********************************************************************************/
#ifndef UI_NOISE_GEN_H
#define UI_NOISE_GEN_H

// forward declaration (pointer use only)
struct Texture;

class UI_NoiseGen
{
public:
    UI_NoiseGen();
    ~UI_NoiseGen();

    void Draw();

private:
    void DrawWhiteNoiseGenerator();
    void DrawValueNoiseGenerator();
    void DrawPerlinNoiseGenerator();
    void DrawVoronoiNoiseGenerator();

private:
    Texture* pTexWhiteNoise_;
    Texture* pTexValueNoise_;
    Texture* pTexPerlinNoise_;
    Texture* pTexVoronoiNoise_;

    bool bRegenerate_ = true;
    bool bSaveImage_ = false;
};

#endif
