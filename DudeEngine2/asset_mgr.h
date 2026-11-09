/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: asset_mgr.h
    Desc:     storage for all the assets (textures, animations, sounds, etc.)
    Created:  by DimaSkup
\**********************************************************************************/
#ifndef ASSET_MGR_H
#define ASSET_MGR_H

#include "types.h"
#include "vec.h"
#include "cvector.h"


//---------------------------------------------------------
// some forward declarations (pointer use only)
//---------------------------------------------------------
struct Texture;
class Sound;

//---------------------------------------------------------
// class name:  AssetMgr
//---------------------------------------------------------
class AssetMgr
{
public:
    enum eAssetAttr : uint8
    {
        ASSET_ATTR_NONE   = (1 << 0),
        ASSET_ATTR_COMMON = (1 << 1),   // asset is level independent and is used for EACH level (for instance: player's textures, sounds, or game UI)
        ASSET_ATTR_REMAIN = (1 << 2),   // asset is level dependent but two levels may use the same asset (by the same name) so an asset with this flag won't be released and loaded anew when we switch level
    };

public:
    AssetMgr();
    ~AssetMgr();

    bool             Init();
    void             ClearData();

    //---------------------------------
    // debug
    //---------------------------------
    void             DumpTextures() const;
    void             DumpAnimations() const;
    void             DumpSounds() const;

    //---------------------------------
    // common assets
    //---------------------------------
    void             LoadCommonAssets();
    void             SaveCommonAssets();
    
    //---------------------------------
    // textures
    //---------------------------------
    void             LoadTextures(const char* texFilePath, const eAssetAttr attrMask);
    void             SaveTextures(const char* texFilePath);

    TexID            LoadTexture(const char* texName, const char* filePath);
    Texture*         GetTex(const TexID id);
    Texture*         GetTex(const TexID id) const;
    Texture*         GetTex(const char* texName);
                 
    TexID            GetTexId(const char* texName);
    const char*      GetTexName(const TexID texId) const;
    const char*      GetTexPath(const TexID texId) const;

    bool             HasTexture(const TexID id) const;
    bool             HasTexture(const char* texName) const;
    bool             HasAnimByName(const char* animName) const;

    Vec2i            GetTexSize(const TexID id) const;

    //---------------------------------
    // animations
    //---------------------------------
    static Animation InitAnim(TexID texId, uint16 speed, uint8 numFrames, bool bLooped);

    void             LoadAnimations(const char* filepath);
    void             SaveAnimations(const char* filepath);

    void             AddAnim(const char* name, 
                                  const TexID texId,
                                  const uint16 speed,
                                  const uint8 numFrames,
                                  const bool bLooped);

    const char*      GetAnimName(const AnimID id) const;
    const Animation& GetAnim(const AnimID id) const;
    const Animation& GetAnim(const char* name) const;

    //---------------------------------
    // sounds
    //---------------------------------
    void             LoadSounds(const char* filepath, const eAssetAttr attrMask);
    void             SaveSounds(const char* filepath) const;

    SoundID          LoadSound(const char* name, const char* path);
    SoundID          GetSoundId(const char* name) const;
                     
    void             PlaySound(const SoundID id);
    void             StopSound(const SoundID id);

    void             PlaySound(const char* soundName);
    void             StopSound(const char* soundName);

    void             SetSoundVolume(const SoundID id, const float volume);
    void             SetSoundLooping(const SoundID id, bool bLooping);

    //---------------------------------
    // weapons
    //---------------------------------
    bool             LoadWeapons(const char* path);
    bool             HasWpnByName(const char* wpnName) const;

    const WeaponID   GetWpnIdByName(const char* wpnName) const;
    const char*      GetWpnNameById(const WeaponID id) const;
    const Weapon*    GetWpnByName(const char* wpnName) const;
    const Weapon*    GetWpnById(const WeaponID wpnId) const;

    void             SetWpnCooldownSec(const WeaponID id, const float sec);
    void             SetWpnAttackRadius(const WeaponID id, const int r);
    void             SetWpnDamage(const WeaponID id, const int damage);

private:
    void             SaveTexturesHelper(const char* filepath, const eAssetAttr attrMask) const;
    void             SaveSoundsHelper  (const char* filepath, const eAssetAttr attrMask) const;

    Texture*         LoadTexture(const char* fileName);
    index            GetTexIdx(const TexID texId) const;
    index            GetTexIdx(const char* texName) const;
    bool             HasTex(const char* texName) const;
                     
    index            GetAnimIdx(const AnimID id) const;
    index            GetAnimIdx(const char* animName) const;

    index            GetSoundIdx(const SoundID id) const;
    index            GetSoundIdx(const char* soundName) const;
                     
    bool             IsAssetsDirRelPath(const char* path) const;
    bool             IsAssetsDirFullPath(const char* fullPath, char* outRelPath) const;

private:
    // textures
    cvector<TexID>          texIds_;
    cvector<TextureName>    texNames_;
    cvector<Texture*>       textures_;
    cvector<char*>          texPaths_;
    cvector<uint8>          texAttrs_;

    // animations
    cvector<AnimID>         animIds_;
    cvector<Animation>      animations_;
    cvector<char*>          animNames_;

    // sounds
    cvector<SoundID>        soundIds_;
    cvector<Sound*>         sounds_;
    cvector<char*>          soundPaths_;
    cvector<SoundName>      soundNames_;
    cvector<uint8>          soundAttrs_;

    // weapons
    WeaponName              wpnNames_[NUM_ALL_WEAPONS];
    Weapon                  weapons_[NUM_ALL_WEAPONS];
    int                     numWeapons_;                // num of registered weapons
};

//---------------------------------------------------------
// global instance
//---------------------------------------------------------
extern AssetMgr g_AssetMgr;


#endif
