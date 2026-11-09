#include "ui_noise_gen.h"
#include "noise.h"
#include "types.h"
#include "cvector.h"
#include "texture.h"
#include "log.h"
#include "mem_helpers.h"
#include "image.h"
#include "file_sys.h"
#include <tinyfiledialogs/tinyfiledialogs.h>  // for using OS filesystem dialog window
#include <imgui.h>


// forward declaration of private helpers
void CreatePerlinMap(const Noise::PerlinConfig& cfg, cvector<float>& heightMap);
void SaveAsImage(const uint8* data, const int width, const int height, const int bpp);

//---------------------------------------------------------
// default constructor
//---------------------------------------------------------
UI_NoiseGen::UI_NoiseGen() :
    pTexWhiteNoise_(nullptr),
    pTexValueNoise_(nullptr),
    pTexPerlinNoise_(nullptr),
    pTexVoronoiNoise_(nullptr)
{
}

//---------------------------------------------------------
// default destructor
//---------------------------------------------------------
UI_NoiseGen::~UI_NoiseGen()
{
    if (pTexWhiteNoise_)
        DestroyTexture(pTexWhiteNoise_);

    if (pTexValueNoise_)
        DestroyTexture(pTexValueNoise_);

    if (pTexPerlinNoise_)
        DestroyTexture(pTexPerlinNoise_);

    if (pTexVoronoiNoise_)
        DestroyTexture(pTexVoronoiNoise_);

    SafeDelete(pTexWhiteNoise_);
    SafeDelete(pTexValueNoise_);
    SafeDelete(pTexPerlinNoise_);
    SafeDelete(pTexVoronoiNoise_);
}

//---------------------------------------------------------
// draw an editor's tab for noise generation
//---------------------------------------------------------
void UI_NoiseGen::Draw()
{
    if (ImGui::CollapsingHeader("White Noise"))
        DrawWhiteNoiseGenerator();

    if (ImGui::CollapsingHeader("Value Noise"))
        DrawValueNoiseGenerator();

    if (ImGui::CollapsingHeader("Perlin Noise"))
        DrawPerlinNoiseGenerator();

    if (ImGui::CollapsingHeader("Voronoi Noise"))
        DrawVoronoiNoiseGenerator();
}

//---------------------------------------------------------
//---------------------------------------------------------
void UI_NoiseGen::DrawWhiteNoiseGenerator()
{
    Texture** pTex = &pTexWhiteNoise_;
    static int width = 256;
    static int height = 256;
    static bool bRegenerate = true;

    //
    // noise control fields
    //
    ImGui::PushID("##img_sz_hash");
    if (ImGui::DragInt("##Image size", &width, 1.0f))
    {
        bRegenerate = true;
        width = Clamp(width, 8, 8192);
        height = width;
    }
    ImGui::PopID();

    // 
    // generate a noise texture if need
    //
    if (bRegenerate)
    {
        cvector<float> noiseMap;
        cvector<uint32> pixels;

        noiseMap.resize(width * height);
        pixels.resize(width * height);

        // generate a noise (values in range (0,1))
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
                noiseMap[y * width + x] = Noise::Hash((float)x, (float)y);

        // create a new empty texture
        if (*pTex)
            DestroyTexture(*pTex);

        *pTex = CreateTexture(width, height);
        if (*pTex == nullptr)
        {
            LogErr(LOG, "can't create a noise (white) texture");
            return;
        }

        // convert pixels from grayscale (0.0, 1.0) into RGBA [0,255]
        for (index i = 0; i < pixels.size(); ++i)
        {
            assert(noiseMap[i] >= 0.0f && noiseMap[i] <= 1.0f);
            const uint8 c = (uint8)(noiseMap[i] * 255.0f);
            pixels[i] = RGBATOU32(c,c,c,255);
        }

        // update our empty texture with some content
        const uint bytesPerPixel = 4;
        if (!UpdateTexture(*pTex, NULL, pixels.data(), width * bytesPerPixel))
        {
            LogErr(LOG, "can't update a noise (white) texture");
            DestroyTexture(*pTex);
            *pTex = nullptr;
            return;
        }

        bRegenerate = false;
    }

    // visualize our texture
    const ImVec2 imgSz((float)width, (float)height);
    ImGui::Image((ImTextureID)(*pTex)->pTex, imgSz);
}

//---------------------------------------------------------
//---------------------------------------------------------
void UI_NoiseGen::DrawValueNoiseGenerator()
{
    Texture** pTex = &pTexValueNoise_;
    static int width = 256;
    static int height = 256;
    static float scale = 50.0f;
    static int seed = 0;
    static bool bRegenerate = true;

    //
    // noise control fields
    //
    ImGui::PushID("##noise_value");
    if (ImGui::DragInt("Image size", &width))
    {
        bRegenerate = true;
        width = Clamp(width, 8, 8192);
        height = width;
    }

    if (ImGui::DragFloat("Scale", &scale))
    {
        bRegenerate = true;
        if (scale <= 0.0f)
            scale = 0.0001f;
    }

    if (ImGui::DragInt("Seed", &seed))
        bRegenerate = true;

    ImGui::PopID();

    // 
    // generate a noise texture if need
    //
    if (bRegenerate)
    {
        cvector<float> noiseMap;
        cvector<uint32> pixels;

        noiseMap.resize(width * height);
        pixels.resize(width * height);

        const float invScale = 1.0f / scale;

        // randomization control
        srand(seed);
        const float offsetX = (float)(rand() % 200000 - 100000);
        const float offsetY = (float)(rand() % 200000 - 100000);

        // generate a noise (values in range (0,1))
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const float sx = (float)x * invScale + offsetX;
                const float sy = (float)y * invScale + offsetY;
                noiseMap[y * width + x] = Noise::Value(sx, sy);
            }
        }

        // create a new empty texture
        if (*pTex)
            DestroyTexture(*pTex);

        *pTex = CreateTexture(width, height);
        if (*pTex == nullptr)
        {
            LogErr(LOG, "can't create a noise (value) texture");
            return;
        }

        // convert pixels from grayscale (0.0, 1.0) into RGBA [0,255]
        for (index i = 0; i < pixels.size(); ++i)
        {
            assert(noiseMap[i] >= 0.0f && noiseMap[i] <= 1.0f);
            const uint8 c = (uint8)(noiseMap[i] * 255.0f);
            pixels[i] = RGBATOU32(c,c,c,255);
        }

        // update our empty texture with some content
        const uint bytesPerPixel = 4;
        if (!UpdateTexture(*pTex, NULL, pixels.data(), width * bytesPerPixel))
        {
            LogErr(LOG, "can't update a noise (value) texture");
            DestroyTexture(*pTex);
            *pTex = nullptr;
            return;
        }

        bRegenerate = false;
    }

    // visualize our texture
    const ImVec2 imgSz((float)width, (float)height);
    ImGui::Image((ImTextureID)(*pTex)->pTex, imgSz);
}

//---------------------------------------------------------
// draw control fields for generation of a perlin noise
//---------------------------------------------------------
void UI_NoiseGen::DrawPerlinNoiseGenerator()
{
    Texture** pTex = &pTexPerlinNoise_;
    static Noise::PerlinConfig cfg;

    int  width            = cfg.width;
    int  height           = cfg.height;
    int  seed             = cfg.seed;
    int  octaves          = cfg.octaves;
    int  scale            = cfg.scale;
    bool bNormalize       = cfg.bNormalize;
    bool bUseFirstTypeGen = cfg.bUseFirstTypeGen;
    bool bInverseColor    = cfg.bInverseColor;

    if (*pTex == nullptr)
        bRegenerate_ = true;

    //
    // noise control fields
    //
    ImGui::PushID("##noise_perlin");
    if (ImGui::DragInt("Image width", &width))
    {
        bRegenerate_ = true;
        cfg.width = Clamp(width, 8, 8192);
    }

    if (ImGui::DragInt("Image height", &height))
    {
        bRegenerate_ = true;
        cfg.height = Clamp(height, 8, 8192);
    }

    if (ImGui::DragInt("Octaves", &octaves))
    {
        bRegenerate_ = true;
        cfg.octaves = Clamp(octaves, 1, (int)Noise::MAX_OCTAVES);
    }

    if (ImGui::DragFloat("Lacunarity", &cfg.lacunarity, 0.001f))
        bRegenerate_ = true;

    ImGui::BeginDisabled(cfg.bUseFirstTypeGen);
    if (ImGui::DragFloat("Persistence", &cfg.persistence, 0.001f))
        bRegenerate_ = true;
    ImGui::EndDisabled();

    if (ImGui::DragInt("Scale", &scale))
    {
        bRegenerate_ = true;
        if (scale > 0) cfg.scale = scale;
    }

    if (ImGui::DragInt("Seed", &seed))
    {
        bRegenerate_ = true;
        cfg.seed = seed;
    }

    if (ImGui::Checkbox("Normalize", &bNormalize))
    {
        bRegenerate_ = true;
        cfg.bNormalize = bNormalize;
    }

    if (ImGui::Checkbox("Use Basic Algorithm", &bUseFirstTypeGen))
    {
        bRegenerate_ = true;
        cfg.bUseFirstTypeGen = bUseFirstTypeGen;
    }

    if (ImGui::Checkbox("Inverse color", &bInverseColor))
    {
        bRegenerate_ = true;
        cfg.bInverseColor = bInverseColor;
    }

    if (ImGui::Button("Save image as..."))
    {
        bRegenerate_ = true;
        bSaveImage_ = true;
    }

    ImGui::PopID();

    //
    // regenerate a noise texture
    //
    if (bRegenerate_)
    {
        cvector<float> heightMap;
        cvector<uint32> pixels;

        CreatePerlinMap(cfg, heightMap);
        pixels.resize(cfg.width * cfg.height);

        // create a new empty texture
        if (*pTex)
            DestroyTexture(*pTex);

        *pTex = CreateTexture(cfg.width, cfg.height);
        if (*pTex == nullptr)
        {
            LogErr(LOG, "can't create a noise (perlin) texture");
            return;
        }

        // convert a single grayscale channel (in range [0.0, 1.0]) 
        // of each pixel into RGBA (in range [0,255])
        if (cfg.bInverseColor)
        {
            for (index i = 0; i < pixels.size(); ++i)
            {
                const uint8 c = (uint8)((1.0f - heightMap[i]) * 255.0f);
                pixels[i] = RGBATOU32(c, c, c, 255);
            }
        }
        else
        {
            for (index i = 0; i < pixels.size(); ++i)
            {
                const uint8 c = (uint8)(heightMap[i] * 255.0f);
                pixels[i] = RGBATOU32(c, c, c, 255);
            }
        }
       

        // update our empty texture with some content
        const uint bytesPerPixel = 4;
        if (!UpdateTexture(*pTex, NULL, pixels.data(), cfg.width * bytesPerPixel))
        {
            LogErr(LOG, "can't update a noise texture (perlin)");
            DestroyTexture(*pTex);
            *pTex = nullptr;
            return;
        }

        // save generated texture onto the disk
        if (bSaveImage_)
        {
            bSaveImage_ = false;
            const uint8* data = (const uint8*)pixels.data();
            const int bpp = bytesPerPixel * 8;  // bits per pixel

            SaveAsImage(data, cfg.width, cfg.height, bpp);
        }

        bRegenerate_ = false;
    }

    // visualize our texture
    if (*pTex)
    {
        const ImVec2 imgSz((float)cfg.width, (float)cfg.height);
        ImGui::Image((ImTextureID)(*pTex)->pTex, imgSz);
    }
}

//---------------------------------------------------------
// Voronoi noise generator
//---------------------------------------------------------
void UI_NoiseGen::DrawVoronoiNoiseGenerator()
{
    Texture** pTex = &pTexVoronoiNoise_;
    static Noise::VoronoiConfig cfg;
    
    int  width         = cfg.width;
    int  height        = cfg.height;
    int  seed          = cfg.seed;
    int  scale         = cfg.scale;
    bool bInverseColor = cfg.bInverseColor;

    if (*pTex == nullptr)
        bRegenerate_ = true;

    //
    // noise control fields
    //
    ImGui::PushID("##noise_voronoi");
    if (ImGui::DragInt("Image width", &width))
    {
        bRegenerate_ = true;
        cfg.width = Clamp(width, 8, 8192);
    }

    if (ImGui::DragInt("Image height", &height))
    {
        bRegenerate_ = true;
        cfg.height = Clamp(height, 8, 8192);
    }

    if (ImGui::DragInt("Scale", &scale))
    {
        bRegenerate_ = true;
        if (scale > 0) cfg.scale = scale;
    }

    if (ImGui::DragInt("Seed", &seed))
    {
        bRegenerate_ = true;
        cfg.seed = seed;
    }

    if (ImGui::DragFloat("Power mid", &cfg.powerMid, 0.05f))
        bRegenerate_ = true;

    if (ImGui::DragFloat("Power final", &cfg.powerFinal, 0.001f))
        bRegenerate_ = true;
    
    if (ImGui::Checkbox("Inverse color", &bInverseColor))
    {
        bRegenerate_ = true;
        cfg.bInverseColor = bInverseColor;
    }
        

    if (ImGui::Button("Save image as..."))
    {
        bRegenerate_ = true;
        bSaveImage_ = true;
    }


    ImGui::PopID();

    // 
    // generate a noise texture if need
    //
    if (bRegenerate_)
    {
        cvector<float> noiseMap;
        cvector<uint32> pixels;

        noiseMap.resize(cfg.width * cfg.height);
        pixels.resize(cfg.width * cfg.height);

        const float invScale = 1.0f / cfg.scale;

        // randomization control
        srand(cfg.seed);
        const float offsetX = (float)(rand() % 200000 - 100000);
        const float offsetY = (float)(rand() % 200000 - 100000);

        // generate a noise
        for (int y = 0; y < cfg.height; ++y)
        {
            for (int x = 0; x < cfg.width; ++x)
            {
                const float sx = (float)x * invScale + offsetX;
                const float sy = (float)y * invScale + offsetY;
                noiseMap[y * cfg.width + x] = Noise::Voronoi(sx, sy, cfg);
            }
        }

        // create a new empty texture
        if (*pTex)
            DestroyTexture(*pTex);

        *pTex = CreateTexture(cfg.width, cfg.height);
        if (*pTex == nullptr)
        {
            LogErr(LOG, "can't create a noise (voronoi) texture");
            return;
        }

        // convert pixels from grayscale (0.0, 1.0) into RGBA [0,255]
        if (cfg.bInverseColor)
        {
            for (index i = 0; i < pixels.size(); ++i)
            {
                const float gray = 1.0f - clampf(noiseMap[i], 0.0f, 1.0f);
                const uint8 c = (uint8)(gray * 255.0f);
                pixels[i] = RGBATOU32(c, c, c, 255);
            }
        }
        else
        {
            for (index i = 0; i < pixels.size(); ++i)
            {
                const float gray = clampf(noiseMap[i], 0.0f, 1.0f);
                const uint8 c = (uint8)(gray * 255.0f);
                pixels[i] = RGBATOU32(c, c, c, 255);
            }
        }

        // update our empty texture with some content
        const uint bytesPerPixel = 4;
        if (!UpdateTexture(*pTex, NULL, pixels.data(), cfg.width * bytesPerPixel))
        {
            LogErr(LOG, "can't update a noise (voronoi) texture");
            DestroyTexture(*pTex);
            *pTex = nullptr;
            return;
        }

        // save generated texture onto the disk
        if (bSaveImage_)
        {
            bSaveImage_ = false;
            const uint8* data = (const uint8*)pixels.data();
            const int bpp = bytesPerPixel * 8;  // bits per pixel

            SaveAsImage(data, cfg.width, cfg.height, bpp);
        }

        bRegenerate_ = false;
    }

    // visualize our texture
    if (*pTex)
    {
        const ImVec2 imgSz((float)cfg.width, (float)cfg.height);
        ImGui::Image((ImTextureID)(*pTex)->pTex, imgSz);
    }
}

//---------------------------------------------------------
// helper for generation of a Perlin Noise map
//---------------------------------------------------------
void CreatePerlinMap(const Noise::PerlinConfig& cfg, cvector<float>& heightMap)
{
    assert(cfg.width > 0 && cfg.height > 0);

    heightMap.resize(cfg.width * cfg.height);

    // prevent division by 0 or wrong scale value
    float scale = 0.0001f;
    if (cfg.scale > 0.0f)
        scale = 1.0f / cfg.scale;

    // randomization control
    srand(cfg.seed);

    // make octaves to be unique
    assert(cfg.octaves <= Noise::MAX_OCTAVES);
    Vec2 octaveOffsets[Noise::MAX_OCTAVES];

    for (int i = 0; i < (int)cfg.octaves; ++i)
    {
        octaveOffsets[i].x = (float)(rand() % 200000 - 100000);
        octaveOffsets[i].y = (float)(rand() % 200000 - 100000);
    }

    // map normalization factors
    float minVal = FLT_MAX;
    float maxVal = FLT_MIN;

    // generate map
    for (int row = 0; row < cfg.height; ++row)
    {
        for (int col = 0; col < cfg.width; ++col)
        {

#if 1
            float sum = 0.0f;
            float freq = 1.0f;
            float amp = 1.0f;

            if (cfg.bUseFirstTypeGen)
            {
                for (uint32_t oct = 0; oct < cfg.octaves; ++oct)
                {
                    const float sx = col * scale * freq + octaveOffsets[oct].x;
                    const float sy = row * scale * freq + octaveOffsets[oct].y;
                    sum += Noise::Perlin(sx, sy);
                    freq *= cfg.lacunarity;
                }
            }
            else
            {
                for (uint32_t oct = 0; oct < cfg.octaves; ++oct)
                {
                    const float sx = col * scale * freq + octaveOffsets[oct].x;
                    const float sy = row * scale * freq + octaveOffsets[oct].y;

                    sum += Noise::Perlin2(sx, sy) * amp;
                    freq *= cfg.lacunarity;
                    amp *= cfg.persistence;
                }
            }
#else

            float sum = Noise::PerlinFBM((float)col, (float)row, cfg);

#endif

            heightMap[row * cfg.width + col] = sum;

            // used for normalization
            if (sum < minVal)
                minVal = sum;

            if (sum > maxVal)
                maxVal = sum;
        }
    }

    // normalization of the map
    if ((maxVal != minVal) && cfg.bNormalize)
    {
        const float invRange = 1.0f / (maxVal - minVal);

        for (int i = 0; i < heightMap.size(); ++i)
            heightMap[i] = (heightMap[i] - minVal) * invRange;
    }
    else
    {
        // at least we must have values in range [0, 1]
        for (int i = 0; i < heightMap.size(); ++i)
            heightMap[i] = clampf(heightMap[i], 0.0f, 1.0f);
    }
}

//---------------------------------------------------------
// 1. open a file dialog window to choose a saving location
// 2. save input raw pixels as an image into the disk
//---------------------------------------------------------
void SaveAsImage(const uint8* data, const int width, const int height, const int bpp)
{
    assert(data);
    assert(width > 0 && height > 0);
    assert(bpp >= 8);

    const char* basePath = FileSys::GetBasePath();
    char defaultPath[256];

    // prepare a default path to maps dir
    snprintf(defaultPath, sizeof(defaultPath), "%s%s", basePath, pathToImgDir);
    FileSys::ToBackSlash(defaultPath);

    const char* fullpath = tinyfd_saveFileDialog("Save Image", defaultPath, 0, NULL, NULL);
    if (fullpath)
    {
        Image img;
        img.SaveAsBMP(fullpath, data, width, height, bpp);
    }
}