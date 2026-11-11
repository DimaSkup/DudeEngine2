/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: asset_mgr.cpp
    Created:  by DimaSkup
\**********************************************************************************/
#include "asset_mgr.h"
#include "log.h"
#include "types.h"
#include "upng.h"
#include "mem_helpers.h"
#include "texture.h"
#include "file_sys.h"
#include "texture.h"
#include "sound.h"
#include "file.h"       // helpers for reading files

#include <assert.h>

//---------------------------------------------------------
// internal static
//---------------------------------------------------------
static TexID s_LastTexId = 0;
static AnimID s_LastAnimId = 0;
static SoundID s_LastSoundId = 0;

static const char* s_InvalidWpnName = "invalid_wpn";

//---------------------------------------------------------
// global instance
//---------------------------------------------------------
AssetMgr g_AssetMgr;

//---------------------------------------------------------
// print an input byte in binary form
//---------------------------------------------------------
void PrintByte(const uint8 n)
{
    for (int i = 7; i >= 0; i--)
        printf("%d", (n >> i) & 1);
}

//---------------------------------------------------------
// default constructor and destructor
//---------------------------------------------------------
AssetMgr::AssetMgr()
{
    Sound::InitSoundEngine();
}

AssetMgr::~AssetMgr()
{
    ClearData();
}

//---------------------------------------------------------
// create "default/invalid" assets
//---------------------------------------------------------
bool AssetMgr::Init()
{
    // add a "dummy" animation
    TexID animTexId = INVALID_TEX_ID;
    uint16 frameTimeMs = 1;
    uint8 numFrames = 1;
    bool bLooped = true;

    // check size to prevent fuck up when we use the "invalid" texture to map texturing
    const TexID invalidTexId = LoadTexture("invalid", "assets/images/common/invalid.png");
    const Vec2i invalidTexSz = g_AssetMgr.GetTexSize(invalidTexId);
    assert(invalidTexSz.w == MAP_TILE_SIZE && invalidTexSz.h == MAP_TILE_SIZE);

    AddAnim("dummy_animation", animTexId, frameTimeMs, numFrames, bLooped);
    LoadSound("dummy", "assets/sounds/dummy.wav");
    LoadWeapons("assets/common/weapons.cfg");

    LoadCommonAssets();

    return true;
}

//---------------------------------------------------------
// Desc:  clean up all the data, release memory
//---------------------------------------------------------
void AssetMgr::ClearData()
{
    // release textures
    for (Texture* pTex : textures_)
        DestroyTexture(pTex);

    // release paths to textures
    for (char* path : texPaths_)
        SafeDeleteArr(path);

    // release names of animations 
    for (char* animName : animNames_)
        SafeDeleteArr(animName);

    // release all the sounds
    for (Sound* pSound : sounds_)
        SafeDelete(pSound);

    // release all the sounds paths
    for (char* path : soundPaths_)
        SafeDeleteArr(path);

    // purge all textures related arrays
    texIds_.purge();
    texNames_.purge();
    texPaths_.purge();
    textures_.purge();
    texAttrs_.purge();

    // purge all animations related arrays
    animIds_.purge();
    animations_.purge();
    animNames_.purge();

    // purge all sounds related arrays
    soundIds_.purge();
    sounds_.purge();
    soundNames_.purge();
    soundPaths_.purge();
    soundAttrs_.purge();

    // reset ids
    s_LastTexId = 0;
    s_LastAnimId = 0;
    s_LastSoundId = 0;

    memset(wpnNames_, 0, sizeof(wpnNames_));
    memset(weapons_, 0, sizeof(weapons_));
    numWeapons_ = 0;

    // check stuff to prevent fuck up
    assert(texIds_.empty());
    assert(texNames_.empty());
    assert(textures_.empty());
    assert(texPaths_.empty());
    assert(texAttrs_.empty());

    assert(animIds_.empty());
    assert(animations_.empty());
    assert(animNames_.empty());

    assert(soundIds_.empty());
    assert(sounds_.empty());
    assert(soundPaths_.empty());
    assert(soundNames_.empty());
    assert(soundAttrs_.empty());

    assert(s_LastTexId == 0);
    assert(s_LastAnimId == 0);
    assert(s_LastSoundId == 0);
}


//=============================================================================
// TEXTURES
//=============================================================================

//---------------------------------------------------------
// load textures for the level (read its declarations from a file by input path)
//---------------------------------------------------------
void AssetMgr::LoadTextures(const char* filePath, const eAssetAttr attrMask)
{
    assert(filePath && filePath[0] != '\0');

    FILE* fp = fopen(filePath, "r");
    if (!fp)
    {
        LogErr(LOG, "%s%s", ERR_OPEN_FILE, filePath);
        return;
    }

    char buf[128];
    char texName[MAX_LEN_TEX_NAME];
    char texPath[MAX_LEN_FILE_PATH];
    int count = 0;

    // skip comments section
    while (fgets(buf, sizeof(buf), fp))
    {
        if (buf[0] != ';')
            break;
    }
    
    // check if we read valid file
    if (strcmp(buf, "textures {\n") != 0)
    {
        fclose(fp);
        LogErr(LOG, "you opened not a textures file: %s", filePath);
        return;
    }

    // read in textures declarations: [tex_name => tex_path]
    while (fgets(buf, sizeof(buf), fp))
    {
        // we've done
        if (buf[0] == '}')
            continue;
        
        if (sscanf(buf, "\t%s %s", texName, texPath) != 2)
        {
            LogErr(LOG, "can't parse a texture decl str: %s", buf);
            continue;
        }

        LoadTexture(texName, texPath);
        texAttrs_.back() |= attrMask;
    }

    fclose(fp);
}

//---------------------------------------------------------
// load all the common assets in a separate way
//---------------------------------------------------------
void AssetMgr::LoadCommonAssets() 
{
    g_AssetMgr.LoadTextures  ("assets/common/textures.cfg",   ASSET_ATTR_COMMON);
    g_AssetMgr.LoadSounds    ("assets/common/sounds.cfg",     ASSET_ATTR_COMMON);
    g_AssetMgr.LoadAnimations("assets/common/animations.cfg");
}

//---------------------------------------------------------
// save all the common assets in a separate way
//---------------------------------------------------------
void AssetMgr::SaveCommonAssets() 
{
    SaveTexturesHelper("assets/common/textures.cfg", ASSET_ATTR_COMMON);
    SaveSoundsHelper  ("assets/common/sounds.cfg",   ASSET_ATTR_COMMON);
    SaveAnimations    ("assets/common/animations.cfg");
}

//---------------------------------------------------------
// save metadata of textures specific for this level into a file
//---------------------------------------------------------
void AssetMgr::SaveTextures(const char* filepath)
{
    SaveTexturesHelper(filepath, ASSET_ATTR_NONE);
}

//---------------------------------------------------------
// save metadata of sounds specific for this level into a file
//---------------------------------------------------------
void AssetMgr::SaveSounds(const char* filepath) const
{
    SaveSoundsHelper(filepath, ASSET_ATTR_NONE);
}

//---------------------------------------------------------
// private helper for saving texture assets info into a file
//---------------------------------------------------------
void AssetMgr::SaveTexturesHelper(const char* filepath, const eAssetAttr attrMask) const
{
    assert(filepath && filepath[0] != '\0');

    FILE* fp = fopen(filepath, "w");
    if (!fp)
    {
        LogErr(LOG, "%s%s", ERR_OPEN_FILE, filepath);
        return;
    }

    fputs(";===============================================\n", fp);
    fputs("; register textures;\n", fp);
    fputs("; order of params : [texture_name  texture_path]\n", fp);
    fputs(";===============================================\n", fp);
    fputs("textures {\n", fp);

    // start from 1 to prevent saving of "invalid" texture
    for (index i = 1; i < texNames_.size(); ++i)
    {
        // save only textures specific for this level
        if (texAttrs_[i] & attrMask)
        {
            fprintf(fp, "\t%-32s%s\n", texNames_[i].name, texPaths_[i]);
        }
    }

    fputc('}', fp);
    fclose(fp);
}

//---------------------------------------------------------
// private helper for saving sounds info into a file
//---------------------------------------------------------
void AssetMgr::SaveSoundsHelper(const char* filepath, const eAssetAttr attrMask) const
{
    assert(filepath && filepath[0] != '\0');

    FILE* fp = fopen(filepath, "w");
    if (!fp)
    {
        LogErr(LOG, "%s%s", ERR_OPEN_FILE, filepath);
        return;
    }

    fputs(";===============================================\n", fp);
    fputs("; register sounds;\n", fp);
    fputs("; order of params : [sound_name  sound_filepath  sound_volume]\n", fp);
    fputs(";===============================================\n", fp);
    fputs("sounds {\n", fp);

    // start from 1 to prevent saving of "invalid" texture
    for (index i = 1; i < soundNames_.size(); ++i)
    {
        // save only textures specific for this level
        if (soundAttrs_[i] & attrMask)
        {
            fprintf(fp, "\t%-16s %-32s  %.2f\n", 
                soundNames_[i].name, 
                soundPaths_[i], 
                sounds_[i]->GetVolume());
        }
    }

    fputc('}', fp);

    DumpSounds();

    fclose(fp);
}

//---------------------------------------------------------
// is input path is relative to the working dir and the file is in assets directory?
//---------------------------------------------------------
bool AssetMgr::IsAssetsDirRelPath(const char* filepath) const
{
    assert(filepath && filepath[0] != '\0');
    return (strncmp(filepath, pathToAssetsDir, strlen(pathToAssetsDir)) == 0);
}

//---------------------------------------------------------
// Desc:  is input path is full but points to a file from the assets directory?
// Args:  filepath   -- full path to file
//        outRelPath -- if file is in assets dir we save here a relative path (relatively to the working dir)
//---------------------------------------------------------
bool AssetMgr::IsAssetsDirFullPath(const char* filepath, char* outRelPath) const
{
    assert(filepath && filepath[0] != '\0');
    assert(outRelPath);

    outRelPath[0] = '\0';
    FileSys::GetRelPath(filepath, outRelPath);

    if (outRelPath[0] == '\0')
        return false;

    return IsAssetsDirRelPath(outRelPath);
}

//---------------------------------------------------------
// Desc:  load and create a new texture
// Args:  texName  - string key for texture
//        filePath - path to texture file 
// Ret:   id of the new texture
//---------------------------------------------------------
TexID AssetMgr::LoadTexture(const char* texName, const char* filePath)
{
    assert(texName && texName[0] != '\0');
    assert(filePath && filePath[0] != '\0');
    assert(strlen(texName) < MAX_LEN_TEX_NAME);

    // prevent loading a texture with the same name
    if (HasTex(texName))
    {
        return GetTexId(texName);
    }

    const char* path = filePath;
    char relPath[256];

    // assure that input file is in the assets directory
    if (!IsAssetsDirRelPath(filePath))
    {
        if (!IsAssetsDirFullPath(filePath, relPath))
        {
            LogErr(LOG, "SOSI BIBU! You can load textures only from assets directory");
            return INVALID_TEX_ID;
        }
        path = relPath;
    }
    
    // load texture from file
    Texture* pTex = LoadTexture(filePath);
    if (!pTex)
    {
        LogErr(LOG, "can't load a texture (%s) from file (%s)", texName, filePath);
        return INVALID_TEX_ID;
    }

    // set ID
    TexID texId = s_LastTexId++;
    texIds_.push_back(texId);

    // set name
    texNames_.push_back(TextureName());
    strcpy(texNames_.back().name, texName);

    // set texture
    textures_.push_back(pTex);

    // save a path to texture file
    const size_t len = strlen(path);
    char* texPath = new char[len + 1];
    strcpy(texPath, path);
    texPath[len] = '\0';
    texPaths_.push_back(texPath);

    // setup asset attributes (no attrs by default)
    texAttrs_.push_back(0);

    return texId;
}

//---------------------------------------------------------
// Desc:  load a new texture from a file
//---------------------------------------------------------
Texture* AssetMgr::LoadTexture(const char* filepath)
{
    assert(filepath && filepath[0] != '\0');

    // load a png texture image from the file by filename
    upng_t* pPngTex = upng_new_from_file(filepath);
    if (!pPngTex)
    {
        LogErr(LOG, "can't load a texture from file: %s", filepath);
        return nullptr;
    }

    // check if we have some specific errors
    upng_decode(pPngTex);
    upng_error errCode = upng_get_error(pPngTex);
    if (errCode != UPNG_EOK)
    {
        const char* errMsgs[9] = {
            { "success (no error)" },                       // UPNG_EOK
            { "memory allocation failed" },                 // UPNG_ENOMEM
            { "resource not found (file missing)" },        // UPNG_ENOTFOUND
            { "image data does not have a PNG header" },    // UPNG_ENOTPNG
            { "image data is not a valid PNG image" },      // UPNG_EMALFORMED
            { "critical PNG chunk type is not supported" }, // UPNG_EUNSUPPORTED
            { "image interlacing is not supported" },       // UPNG_EUNINTERLACED
            { "image color format is not supported" },      // UPNG_EUNFORMAT
            { "invalid parameter to method call" }          // UPNG_EPARAM
        };
      
        LogErr(LOG, "png texture error \n\t"
                    "err_code: %d \n\t"
                    "err_msg: %s\n\t"
                    "file: %s", 
                    (int)errCode, errMsgs[errCode], filepath);
        return nullptr;
    }

    // get data
    uint32* pixels = (uint32*)upng_get_buffer(pPngTex);
    uint texWidth  = upng_get_width(pPngTex);
    uint texHeight = upng_get_height(pPngTex);
    uint bpp       = upng_get_bpp(pPngTex);

    if (!pixels)
    {
        LogErr(LOG, "can't get pixels buffer for png image: %s", filepath);
        upng_free(pPngTex);
        return nullptr;
    }

    if (texWidth == 0 || texHeight == 0)
    {
        LogErr(LOG, "got zero texture dimensions (w: %u,  h: %u) of image: ", texWidth, texHeight, filepath);
        upng_free(pPngTex);
        return nullptr;
    }

    if (bpp != 32)
    {
        LogErr(LOG, "we support only 32-bits png images (your bpp: %u)", bpp);
        upng_free(pPngTex);
        return nullptr;
    }

    Texture* pTex = CreateTexture(texWidth, texHeight);
    if (!pTex)
    {
        LogErr(LOG, "can't create a texture from file: %s", filepath);
        upng_free(pPngTex);
        return nullptr;
    }

    // enable alpha blending
    SetTextureBlendMode(pTex, BLENDMODE_BLEND);
    //SetTextureBlendMode(pTex, BLENDMODE_NONE);

    // update a texture with raw pixels
    const uint bytesPerPixel = 4;
    if (!UpdateTexture(pTex, NULL, pixels, texWidth * bytesPerPixel))
    {
        LogErr(LOG, "can't update a texture (during loading of file: %s)", filepath);
        upng_free(pPngTex);
        DestroyTexture(pTex);
        return nullptr;
    }

    // free raw pixels data
    upng_free(pPngTex);
    return pTex;
}

//---------------------------------------------------------
// Desc:  get a name of texture by id
//---------------------------------------------------------
const char* AssetMgr::GetTexName(const TexID texId) const
{
    return texNames_[GetTexIdx(texId)].name;
}

//---------------------------------------------------------
// Desc:  return a path to file of texture by id
//---------------------------------------------------------
const char* AssetMgr::GetTexPath(const TexID texId) const
{
    return texPaths_[GetTexIdx(texId)];
}

//---------------------------------------------------------
// Desc:  get texture id by input name
//---------------------------------------------------------
TexID AssetMgr::GetTexId(const char* texName)
{
    return texIds_[GetTexIdx(texName)];
}

//---------------------------------------------------------
// Desc:  is there any texture by input id/name?
//---------------------------------------------------------
bool AssetMgr::HasTexture(const TexID id) const
{
    return texIds_.binary_search(id);
}

//---------------------------------------------------------

bool AssetMgr::HasTexture(const char* texName) const
{
    if (!texName || texName[0] == '\0')
    {
        LogErr(LOG, ERR_EMPTY_NAME);
        return false;
    }

    for (index i = 0; i < texNames_.size(); ++i)
    {
        if (strcmp(texNames_[i].name, texName) == 0)
            return true;
    }

    return false;
}

//---------------------------------------------------------
// Desc:  get a ptr to texture by input id
// NOTE:  in any case we get a VALID ptr, but in case when we don't have
//        such a texture we get a ptr to "invalid" (checkboard) texture
//---------------------------------------------------------
Texture* AssetMgr::GetTex(const TexID id)
{
    return textures_[GetTexIdx(id)];
}

Texture* AssetMgr::GetTex(const TexID id) const
{
    return textures_[GetTexIdx(id)];
}

//---------------------------------------------------------
// Desc:  get a ptr to texture by input name
// NOTE:  in any case we get a VALID ptr, but in case when we don't have
//        such a texture we get a ptr to "invalid" (checkboard) texture
//---------------------------------------------------------
Texture* AssetMgr::GetTex(const char* texName)
{
    return textures_[GetTexIdx(texName)];
}

//---------------------------------------------------------
// Desc:  get an index of the texture by id
//---------------------------------------------------------
index AssetMgr::GetTexIdx(const TexID texId) const
{
    const index idx = texIds_.get_idx(texId);

    if (texIds_.is_valid_index(idx))
        return idx;
    
    LogErr(LOG, "no texture by id: %d", (int)texId);
    return 0;
}

//---------------------------------------------------------
// Desc:  get an index of the texture by name
//---------------------------------------------------------
index AssetMgr::GetTexIdx(const char* texName) const
{
    assert(texName && texName[0] != '\0');

    for (index i = 0; i < texNames_.size(); ++i)
    {
        if (strcmp(texName, texNames_[i].name) == 0)
            return i;
    }

    LogErr(LOG, "no texture by name: %s", texName);
    return 0;
}

//---------------------------------------------------------
// check if we already have a texture by name
//---------------------------------------------------------
bool AssetMgr::HasTex(const char* texName) const
{
    assert(texName && texName[0] != '\0');

    for (index i = 0; i < texNames_.size(); ++i)
    {
        if (strcmp(texName, texNames_[i].name) == 0)
            return true;
    }

    return false;
}

//---------------------------------------------------------
// get texture width and height
//---------------------------------------------------------
Vec2i AssetMgr::GetTexSize(const TexID id) const
{
    Texture* pTex = GetTex(id);
    assert(pTex);
    return Vec2i{ pTex->w, pTex->h };
}


//=============================================================================
// ANIMATIONS
//=============================================================================

//---------------------------------------------------------
// (static method)
// Desc:  just init a new animation object and return it (no storing in the mgr yet)
//---------------------------------------------------------
Animation AssetMgr::InitAnim(TexID texId, uint16 speed, uint8 numFrames, bool bLooped)
{
    assert(numFrames > 0);

    Animation anim;
    anim.id          = INVALID_ANIM_ID;
    anim.texId       = texId;
    anim.speed       = speed;
    anim.numFrames   = numFrames;
    anim.bLooped     = bLooped;
    anim.bPlayedOnce = false;

    return anim;
}

//---------------------------------------------------------
// register a new animation which can be accessed by a name
//---------------------------------------------------------
void AssetMgr::AddAnim(
    const char* name,
    const TexID texId,
    const uint16 speed,
    const uint8 numFrames,
    const bool bLooped)
{
    if (!name || name[0] == '\0')
    {
        LogErr(LOG, ERR_EMPTY_NAME);
        return;
    }

    if (HasAnimByName(name))
    {
        LogErr(LOG, "there is already an anim by name: %s", name);
        return;
    }

    // id
    animIds_.push_back(s_LastAnimId);

    // animation params
    const bool bPlayedOnce = false;
    animations_.push_back(Animation{ s_LastAnimId, texId, speed, numFrames, bLooped, bPlayedOnce });

    // name
    const size_t len = strlen(name);
    char* animName = new char[len + 1];
    strcpy(animName, name);
    animNames_.push_back(animName);

    s_LastAnimId++;
}

//---------------------------------------------------------
// save all animations into a file
//---------------------------------------------------------
void AssetMgr::SaveAnimations(const char* filepath)
{
    assert(filepath && filepath[0] != '\0');

    FILE* fp = fopen(filepath, "w");
    if (!fp)
        LogFatal(LOG, "%s%s", ERR_OPEN_FILE, filepath);

    // start from 1 because we don't save the "dummy" animation by index 0
    for (index i = 1; i < animations_.size(); ++i)
    {
        fprintf(fp, "animation {\n");

        fprintf(fp, "\tname        %s\n", animNames_[i]);
        fprintf(fp, "\ttexture     %s\n", GetTexName(animations_[i].texId));
        fprintf(fp, "\tspeed_ms    %d\n", animations_[i].speed);
        fprintf(fp, "\tnum_frames  %d\n", animations_[i].numFrames);
        fprintf(fp, "\tis_looped   %d\n", animations_[i].bLooped);

        fprintf(fp, "}\n\n");
    }

    fclose(fp);
}

//---------------------------------------------------------
// load all animations from a file
//---------------------------------------------------------
void AssetMgr::LoadAnimations(const char* filepath)
{
    assert(filepath && filepath[0] != '\0');

    FILE* fp = fopen(filepath, "r");
    if (!fp)
        LogFatal(LOG, "%s%s", ERR_OPEN_FILE, filepath);

    char buf[128];
    int count = 0;

    while (fgets(buf, sizeof(buf), fp))
    {
        // expect the beginning of a declaration
        if (buf[0] != 'a')
            continue;

        char animName[32];
        char texName[MAX_LEN_TEX_NAME];
        int speedMs = 0;
        int numFrames = 0;
        int bLooped = 1;

        animName[0] = '\0';
        texName[0] = '\0';

        count = fscanf(fp, "\tname %s\n", animName);
        assert(count == 1);

        count = fscanf(fp, "\ttexture %s\n", texName);
        assert(count == 1);

        count = fscanf(fp, "\tspeed_ms %d\n", &speedMs);
        assert(count == 1);

        count = fscanf(fp, "\tnum_frames %d\n", &numFrames);
        assert(count == 1);

        count = fscanf(fp, "\tis_looped %d\n", &bLooped);
        assert(count == 1);

        AddAnim(animName, GetTexId(texName), (uint16)speedMs, (uint8)numFrames, bLooped);
    }

    fclose(fp);
}

//---------------------------------------------------------
// get a name of animation by input id
//---------------------------------------------------------
const char* AssetMgr::GetAnimName(const AnimID id) const
{
    return animNames_[GetAnimIdx(id)];
}

//---------------------------------------------------------
// get an animation by input id
// or get the "dummy" animation if there is no such animation
//---------------------------------------------------------
const Animation& AssetMgr::GetAnim(const AnimID id) const
{
    return animations_[GetAnimIdx(id)];
}

//---------------------------------------------------------
// get an animation by input name
// or get the "dummy" animation if there is no such animation
//---------------------------------------------------------
const Animation& AssetMgr::GetAnim(const char* name) const
{
    assert(name && name[0] != '\0');
    return animations_[GetAnimIdx(name)];
}

//---------------------------------------------------------
// return an index to animation by input id / name
//---------------------------------------------------------
index AssetMgr::GetAnimIdx(const AnimID id) const
{
    const index idx = animIds_.get_idx(id);

    if (idx > 0 && idx < animIds_.size() && animIds_[idx] == id)
        return idx;

    LogErr(LOG, "no anim by id: %d", (int)id);
    return 0;
}

//---------------------------------------------------------

index AssetMgr::GetAnimIdx(const char* animName) const
{
    assert(animName && animName[0] != '\0');

    // start from 1 because at 0 we have the "dummy" animation
    for (index i = 1; i < animNames_.size(); ++i)
    {
        const char* name = animNames_[i];
        assert(name && name[0] != '\0');

        if (strcmp(name, animName) == 0)
            return i;
    }

    LogErr(LOG, "no anim by name: %s", animName);
    return INVALID_ANIM_ID;
}

//---------------------------------------------------------
// check if we have an animation by input name
//---------------------------------------------------------
bool AssetMgr::HasAnimByName(const char* animName) const
{
    assert(animName && animName[0] != '\0');

    for (const char* name : animNames_)
    {
        assert(name && name[0] != '\0');
        if (strcmp(name, animName) == 0)
            return true;
    }

    return false;
}


//=============================================================================
// SOUND
//=============================================================================

//---------------------------------------------------------
// load sounds reading its [sound_name => sound_path] from a file
//---------------------------------------------------------
void AssetMgr::LoadSounds(const char* filepath, const eAssetAttr attrMask)
{
    assert(filepath && filepath[0] != '\0');

    FILE* fp = fopen(filepath, "r");
    if (!fp)
    {
        LogErr(LOG, "%s%s", ERR_OPEN_FILE, filepath);
        return;
    }

    char buf[128];
    char name[32];
    char path[64];
    float volume = 1.0f;
    int count = 0;

    // skip comments section
    while (fgets(buf, sizeof(buf), fp))
    {
        if (buf[0] != ';')
            break;
    }

    if (strncmp(buf, "sounds", 6) != 0)
    {
        LogErr(LOG, "sounds decl is expected, but got %s\nfor a file: %s", buf, path);
        fclose(fp);
        return;
    }

    // read sounds declarations 
    while (fgets(buf, sizeof(buf), fp))
    {
        // we've done here
        if (buf[0] == '}')
            break;

        count = sscanf(buf, "\t%s %s %f", name, path, &volume);
        if (count != 3)
        {
            LogErr(LOG, "can't get sound decl from a line: %s", buf);
            continue;
        }

        const SoundID id = LoadSound(name, path);
        if (id == INVALID_SOUND_ID)
            continue;

        // apply the asset attribute(s)
        uint8& attr = soundAttrs_.back();
        attr |= attrMask;

        SetSoundVolume(id, volume);
    }

    printf("%s\n", filepath);
    DumpSounds();

    fclose(fp);
}

//---------------------------------------------------------
// Desc:  load a new sound by input path
// Args:  name - assign it as a name of this sound
//        path - load a sound by this path
// Ret:   unique identifier of this new sound
//---------------------------------------------------------
SoundID AssetMgr::LoadSound(const char* name, const char* path) 
{
    assert(name && name[0] != '\0');
    assert(path && path[0] != '\0');

    const char* errFmt = "can't load a sound (%s) from file: %s (%s)";

    // check the sound name
    if (strlen(name) >= MAX_LEN_SOUND_NAME)
    {
        LogErr(LOG, errFmt, name, path, "name is too long");
        return INVALID_SOUND_ID;
    }

    // create and init the sound obj
    Sound* pSound = new Sound();
    if (!pSound->LoadFromFile(s_LastSoundId, path))
    {
        LogErr(LOG, errFmt, name, path, "wrong file");
        SafeDelete(pSound);
        return INVALID_SOUND_ID;
    }


    // store the sound in the manager
    soundIds_.push_back(s_LastSoundId);
    sounds_.push_back(pSound);
    soundNames_.push_back(SoundName());
    
    char* dstName = soundNames_.back().name;
    strcpy(dstName, name);
    dstName[strlen(name)] = '\0';

    const size_t len = strlen(path);
    char* dstPath = new char[strlen(path) + 1];
    strcpy(dstPath, path);
    dstPath[len] = '\0';
    soundPaths_.push_back(dstPath);

    // no asset attributes by default
    soundAttrs_.push_back(0);

    return s_LastSoundId++;
}  

//---------------------------------------------------------
// return an ID of sound by name
//---------------------------------------------------------
SoundID AssetMgr::GetSoundId(const char* name) const
{
    return soundIds_[GetSoundIdx(name)];
}

//---------------------------------------------------------
// play a sound by ID
//---------------------------------------------------------
void AssetMgr::PlaySound(const SoundID id) 
{
    sounds_[GetSoundIdx(id)]->Play();
}

//---------------------------------------------------------
// stop a sound by ID
//---------------------------------------------------------
void AssetMgr::StopSound(const SoundID id)
{
    sounds_[GetSoundIdx(id)]->Stop();
}

//---------------------------------------------------------
// play a sound by name
//---------------------------------------------------------
void AssetMgr::PlaySound(const char* soundName)
{
    sounds_[GetSoundIdx(soundName)]->Play();
}

//---------------------------------------------------------
// stop a sound by name
//---------------------------------------------------------
void AssetMgr::StopSound(const char* soundName)
{
    sounds_[GetSoundIdx(soundName)]->Stop();
}

//---------------------------------------------------------
// setup a volume for the sound by ID
//---------------------------------------------------------
void AssetMgr::SetSoundVolume(const SoundID id, const float volume)
{
    sounds_[GetSoundIdx(id)]->SetVolume(volume);
}

//---------------------------------------------------------
// enable/disable a looping for the sound by ID
//---------------------------------------------------------
void AssetMgr::SetSoundLooping(const SoundID id, bool bLooping)
{
    sounds_[GetSoundIdx(id)]->SetLooping(bLooping);
}

//---------------------------------------------------------
// Desc:  return an index of sound by ID
// Ret:   0 if there is no sound by id (index 0 points to the "dummy" sound)
//---------------------------------------------------------
index AssetMgr::GetSoundIdx(const SoundID id) const 
{
    const index i = soundIds_.get_idx(id);

    if (i > 0 && i < soundIds_.size() && soundIds_[i] == id)
        return i;

    LogErr(LOG, "no sound by id: %d", (int)id);
    return 0;
}

//---------------------------------------------------------
// Desc:  return an index of sound by name
// Ret:   0 if there is no sound by id (index 0 points to the "dummy" sound)
//---------------------------------------------------------
index AssetMgr::GetSoundIdx(const char* soundName) const 
{
    if (!soundName || soundName[0] == '\0')
    {
        LogErr(LOG, ERR_EMPTY_NAME);
        return false;
    }

    // skip the "dummy" sound by index 0
    for (index i = 1; i < soundNames_.size(); ++i)
    {
        if (strcmp(soundNames_[i].name, soundName) == 0)
            return i;
    }

    LogErr(LOG, "no sound by name: %s", soundName);
    return 0;
}


//=============================================================================
// WEAPONS
//=============================================================================

//---------------------------------------------------------
// register weapons loadings its data from a file by filepath
//---------------------------------------------------------
bool AssetMgr::LoadWeapons(const char* filepath)
{
    assert(filepath && filepath[0] != '\0');

    char buf[128];
    int count = 0;

    FILE* fp = fopen(filepath, "r");
    if (!fp)
    {
        LogErr(LOG, "%s%s", ERR_OPEN_FILE, filepath);
        return false;
    }


    // skip comments section
    while (fgets(buf, sizeof(buf), fp))
    {
        if (buf[0] != ';')
            break;
    }

    // read weapons declarations
    numWeapons_ = 0;

    do
    {
        // look for beginning of a single decl
        if (strcmp(buf, "weapon {\n") != 0)
            continue;

        if (numWeapons_ >= NUM_ALL_WEAPONS)
        {
            LogErr(LOG, "can't reg one more wpn (too many)");
            continue;
        }

        const WeaponID id = numWeapons_;
        char wpnName[64];
        wpnName[0] = '\0';

        // read in a name of weapon
        FileReadStr(fp, "name:", wpnName);
        
        if (strlen(wpnName) >= MAX_LEN_WEAPON_NAME)
        {
            LogErr(LOG, "too long wpn name (%s), max len is", wpnName, MAX_LEN_WEAPON_NAME-1);
            continue;
        }

        // read in and save weapon's params
        strcpy(wpnNames_[id].name, wpnName);
        weapons_[id].id           = id;
        weapons_[id].cooldownSec  = FileReadFlt(fp, "cooldown_sec:");
        weapons_[id].attackRadius = FileReadInt(fp, "attack_radius:");
        weapons_[id].damage       = FileReadInt(fp, "damage:");

        numWeapons_++;

    } while (fgets(buf, sizeof(buf), fp));

    fclose(fp);
    return true;
}

//---------------------------------------------------------
// get data of a weapon by name
//---------------------------------------------------------
const Weapon* AssetMgr::GetWpnByName(const char* wpnName) const
{
    return GetWpnById(GetWpnIdByName(wpnName));
}

//---------------------------------------------------------
// get data of a weapon by ID
//---------------------------------------------------------
const Weapon* AssetMgr::GetWpnById(const WeaponID id) const
{
    if (id >= numWeapons_)
    {
        LogErr(LOG, "invalid weapon id (%d)", (int)id);
        return &weapons_[0];
    }
    return &weapons_[id];
}

//---------------------------------------------------------
// check that we have a weapon by input name
//---------------------------------------------------------
bool AssetMgr::HasWpnByName(const char* wpnName) const
{
    assert(wpnName && wpnName[0] != '\0');

    for (int i = 0; i < numWeapons_; ++i)
    {
        if (strcmp(wpnNames_[i].name, wpnName) == 0)
            return true;
    }

    return false;
}

//---------------------------------------------------------
// return an ID of a weapon by name
//---------------------------------------------------------
const WeaponID AssetMgr::GetWpnIdByName(const char* name) const
{
    if (!name || name[0] == '\0')
    {
        LogErr(LOG, ERR_EMPTY_NAME);
        return (WeaponID)(-1);
    }

    for (int i = 0; i < numWeapons_; ++i)
    {
        if (strcmp(wpnNames_[i].name, name) == 0)
            return (WeaponID)i;
    }

    LogErr(LOG, "no weapon by name: %s", name);
    return (WeaponID)(-1);
}

//---------------------------------------------------------
// return a name (key) of a weapon by input ID
//---------------------------------------------------------
const char* AssetMgr::GetWpnNameById(const WeaponID id) const
{
    if (id >= numWeapons_)
    {
        LogErr(LOG, "invalid weapon id (%d)", (int)id);
        return s_InvalidWpnName;
    }
    return wpnNames_[id].name;
}

//---------------------------------------------------------
// setup a cooldown (in seconds) for a weapon by ID
//---------------------------------------------------------
void AssetMgr::SetWpnCooldownSec(const WeaponID id, const float sec)
{
    if (sec <= 0)
    {
        LogErr(LOG, "invalid weapon's cooldown (%.2f)", sec);
        return;
    }
    if (id >= numWeapons_)
    {
        LogErr(LOG, "invalid weapon id (%d)", (int)id);
        return;
    }

    weapons_[id].cooldownSec = sec;
}

//---------------------------------------------------------
// setup attack radius for weapon by input ID
//---------------------------------------------------------
void AssetMgr::SetWpnAttackRadius(const WeaponID id, const int r)
{
    if (id >= numWeapons_)
    {
        LogErr(LOG, "invalid weapon id (%d)", (int)id);
        return;
    }
    if (r <= 0 || r >= UINT16_MAX)
    {
        LogErr(LOG, "radius is out of range [0, %d]: %d", UINT16_MAX, r);
        return;
    }

    weapons_[id].attackRadius = (uint16)r;
}

//---------------------------------------------------------
// setup damage for a weapon by input ID
//---------------------------------------------------------
void AssetMgr::SetWpnDamage(const WeaponID id, int damage)
{
    if (id >= numWeapons_)
    {
        LogErr(LOG, "invalid weapon id (%d)", (int)id);
        return;
    }
    if (damage <= 0 || damage >= UINT8_MAX)
    {
        LogErr(LOG, "damage is out of range [0, 255]: %d", damage);
        return;
    }

    weapons_[id].damage = (uint8)damage;
}


//=============================================================================
// DEBUG
//=============================================================================

void AssetMgr::DumpTextures() const
{
    const vsize n = texIds_.size();
    assert((texNames_.size() == n) && (texPaths_.size() == n) && (textures_.size() == n));

    printf("\nDUMP TEXTURES:\n");
    for (index i = 0; i < n; ++i)
    {
        const TexID id   = texIds_[i];
        const char* name = texNames_[i].name;
        const char* path = texPaths_[i];
        const uint8 attr = texAttrs_[i];

        printf("  [%-3d] tex (id: %-4d attr: %08X  name: %-32s path: %s\n", (int)i, id, attr, name, path);
    }
    printf("\n");
}

//---------------------------------------------------------

void AssetMgr::DumpAnimations() const
{
    printf("\nDUMP ANIMATIONS:\n");
    for (index i = 0; i < animations_.size(); ++i)
    {
        const Animation& a = animations_[i];

        printf("\t[%d] anim (id: %d,  name: %s)\n", (int)i, animIds_[i], animNames_[i]);
        printf("\t\ttexture:   (id: %d,  name: %s)\n", a.texId, GetTexName(a.texId));
        printf("\t\tspeed_ms:   %d\n", (int)a.speed);
        printf("\t\tnum_frames: %d\n", (int)a.numFrames);
        printf("\t\tlooped:     %d\n", (int)a.bLooped);
        printf("\n");
    }
}

//---------------------------------------------------------

void AssetMgr::DumpSounds() const
{
    printf("\nDUMP ALL SOUNDS:\n");

    const vsize n = soundIds_.size();
    assert((sounds_.size() == n) && 
           (soundPaths_.size() == n) && 
           (soundNames_.size() == n) && 
           (soundAttrs_.size() == n));

    for (index i = 0; i < n; ++i)
    {
        printf("  [%2d] id %-3d ", (int)i, (int)soundIds_[i]);

        printf("attr ");
        PrintByte(soundAttrs_[i]);

        printf(" name: %-16s path: %-32s vol %.2f\n", 
            soundNames_[i].name,
            soundPaths_[i],
            sounds_[i]->GetVolume());

    }
    printf("\n\n");
}