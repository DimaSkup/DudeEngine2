/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: sprite.cpp
\**********************************************************************************/
#include "sprite.h"

#include "../types.h"
#include "component.h"
#include "transform.h"        // component
#include "../r_list.h"        // render list
#include "../game_state.h"
#include "../log.h"
#include "../entity.h"
#include "../macros.h"
#include "../asset_mgr.h"
#include <assert.h>


//-----------------------------------------------------
// names of animation directions and animation types
//-----------------------------------------------------
static const char* s_AnimationDirNames[] =
{
    "up",
    "right",
    "down",
    "left",
    "no dir",
};

static const char* s_AnimationTypeNames[] =
{
    "idle",
    "move",
    "attack",
    "death",
    "damage",
    "block"
};

//-----------------------------------------------------
// default constructor
//-----------------------------------------------------
Sprite::Sprite() : Sprite(INVALID_TEX_ID, false)
{
    STATIC_ASSERT(NUM_ANIM_DIRS == ARRAY_SIZE(s_AnimationDirNames));
    STATIC_ASSERT(NUM_ANIM_TYPES == ARRAY_SIZE(s_AnimationTypeNames));
}

//-----------------------------------------------------
// used to clone the component
//-----------------------------------------------------
Sprite::Sprite(const Sprite* s)
{
    assert(s);

    spriteFlip_ = s->spriteFlip_;
    animations_ = s->animations_;

    PlayAnimation(ANIM_TYPE_IDLE);
    SetAnimDir(s->currAnimDir_);
    SetColor(s->color_);

    sizeof(Animation);

    srcRect_ = s->srcRect_;

    bAnimated_      = s->bAnimated_;
    bFixedOnScreen_ = s->bFixedOnScreen_;
    bShow_          = s->bShow_;
}

//-----------------------------------------------------
// static sprite (no animations, no directions)
//-----------------------------------------------------
Sprite::Sprite(const TexID texId, const bool bFixedOnScreen) :
    spriteFlip_(R_FLIP_NONE),
    pTransform_(nullptr),
    rotationCenter_(0,0),
    rotationAngle_(0.0f),
    bAnimated_(false),
    bFixedOnScreen_(bFixedOnScreen),
    bShow_(true),
    animIndex_(0),
    zIndex_(0)
{
    if (texId == INVALID_TEX_ID)
        LogErr(LOG, "invalid texture id (0)");

    // by default we use the whole texture, but later we can setup it to use only some part
    SetSrcRectSize(g_AssetMgr.GetTexSize(texId));

    // technically we MUST HAVE at least one animation (but one frame, and no directions);
    const int animSpeedMs = 0;
    const int numFrames = 1;
    const bool bLoopedAnim = true;
    const Animation anim = AssetMgr::InitAnim(texId, animSpeedMs, numFrames, bLoopedAnim);
    animations_.emplace(ANIM_TYPE_IDLE, anim);

    // setup textures and color
    //texId_ = texId;
    PlayAnimation(ANIM_TYPE_IDLE);
    SetAnimDir(ANIM_NODIR);
    SetColor(255, 255, 255, 255);
}

//-----------------------------------------------------
// animated sprite: one animation type, no directions
//-----------------------------------------------------
Sprite::Sprite(const Animation& anim, const bool bFixedOnScreen) :
    spriteFlip_(R_FLIP_NONE),
    rotationCenter_(0, 0),
    rotationAngle_(0.0f),
    bFixedOnScreen_(bFixedOnScreen),
    bAnimated_(true),
    bShow_(true),
    zIndex_(0)
{
    if (anim.texId == INVALID_TEX_ID)
        LogErr(LOG, "invalid texture id (0)");

    // by default we use the whole texture, but later we can setup it to use only some part
    SetSrcRectSize(g_AssetMgr.GetTexSize(anim.texId));

    animations_.emplace(ANIM_TYPE_IDLE, anim);

    // setup texture and color
    SetAnimDir(ANIM_NODIR);
    PlayAnimation(ANIM_TYPE_IDLE);
    SetColor(255, 255, 255, 255);
}

//-----------------------------------------------------
// for animated sprites with a multiple animation types + different directions
//-----------------------------------------------------
Sprite::Sprite(
    const Animation& idle,
    const Animation& move,
    const Animation& attack,
    const Animation& death,
    const Animation& damage,
    const Animation& block,
    const bool bFixedOnScreen)
    : 
    spriteFlip_(R_FLIP_NONE),
    rotationCenter_(0, 0),
    rotationAngle_(0.0f),
    bFixedOnScreen_(bFixedOnScreen),
    bAnimated_(true),
    bShow_(true),
    zIndex_(0)
{
    animations_.emplace(ANIM_TYPE_IDLE,   idle);
    animations_.emplace(ANIM_TYPE_MOVE,   move);
    animations_.emplace(ANIM_TYPE_ATTACK, attack);
    animations_.emplace(ANIM_TYPE_DEATH,  death);
    animations_.emplace(ANIM_TYPE_DAMAGE, damage);
    animations_.emplace(ANIM_TYPE_BLOCK,  block);

    // by default we use the whole texture, but later we can setup it to use only some part
    SetSrcRectSize(g_AssetMgr.GetTexSize(idle.texId));

    SetAnimDir(ANIM_DOWN);
    PlayAnimation(ANIM_TYPE_IDLE);
    SetColor(255, 255, 255, 255);
}

//---------------------------------------------------------
// 
//---------------------------------------------------------
void Sprite::SetAnimPreset(const eAnimPreset preset)
{
    animIndex_ = 0;

    if (preset == ANIM_PRESET_NO_ANIM)
    {
        bAnimated_ = false;

        for (int i = 0; i < NUM_ANIM_TYPES; ++i)
        {
            const eAnimType type = (eAnimType)i;

            if (type != ANIM_TYPE_IDLE)
                animations_.erase(type);
        }
    }
    else if (preset == ANIM_PRESET_SINGLE_ANIM)
    {
        bAnimated_ = true;
    }
    else if (preset == ANIM_PRESET_MULTIPLE_ANIM)
    {
        bAnimated_ = true;

        Animation anim;
        anim.texId = INVALID_TEX_ID;
        anim.speed = 100;
        anim.numFrames = 1;
        anim.bLooped = true;

        animations_.emplace(ANIM_TYPE_MOVE,   anim);
        animations_.emplace(ANIM_TYPE_ATTACK, anim);
        animations_.emplace(ANIM_TYPE_DEATH,  anim);
        animations_.emplace(ANIM_TYPE_DAMAGE, anim);
        animations_.emplace(ANIM_TYPE_BLOCK,  anim);
    }
}

//-----------------------------------------------------
// setup direction of animated sprite (up, right, down, left OR no_direction)
//-----------------------------------------------------
void Sprite::SetAnimDir(const eAnimDir direction)
{
    assert(direction >= 0 && direction < NUM_ANIM_DIRS);

    currAnimDir_ = direction;

    if (direction == ANIM_NODIR)
        animIndex_ = 0;
    else
        animIndex_ = direction;   // use direction enum value as index (vertical offset of animation frames)
}
    
//-----------------------------------------------------
// switch to animation by input type
//-----------------------------------------------------
void Sprite::PlayAnimation(const eAnimType animType)
{
    assert(animType >= 0 && animType < NUM_ANIM_TYPES);
        
    // do we have animations of this type?
    auto it = animations_.find(animType);
    assert(it != animations_.end());

    // switch animation
    pCurrAnimation_ = &it->second;
    currAnimType_ = animType;
    currAnimFrame_ = 0;
    currFrameTime_ = 0.0f;

    const Vec2i sz = {srcRect_.x1-srcRect_.x0, srcRect_.y1-srcRect_.y0};
    srcRect_.x0 = 0;
    srcRect_.x1 = sz.w;
    srcRect_.y0 = animIndex_ * sz.h;
    srcRect_.y1 = srcRect_.y0 + sz.h;
}
    
//-----------------------------------------------------
// setup a texture for animation of type 
// (it is supposed that we use the same texture for multiple animations, 
// OR at least textures of THE SAME SIZE)
//-----------------------------------------------------
void Sprite::SetTex(const eAnimType type, const TexID texId)
{
    assert(type >= 0 && type < NUM_ANIM_TYPES);
    animations_[type].texId = texId;
}

//-----------------------------------------------------
// setup params of animation by type
//-----------------------------------------------------
void Sprite::SetupAnimation(const eAnimType type, const Animation& anim)
{
    assert(type >= 0 && type < NUM_ANIM_TYPES);
    assert(animations_.contains(type));
    assert(anim.texId != INVALID_TEX_ID);
    assert(anim.numFrames > 0);
    assert(anim.speed > 0);

    animations_[type].texId       = anim.texId;
    animations_[type].speed       = anim.speed; 
    animations_[type].numFrames   = anim.numFrames;
    animations_[type].bLooped     = anim.bLooped;
    animations_[type].bPlayedOnce = false;
}

//-----------------------------------------------------
// return a name / type of this component
//-----------------------------------------------------
const char* Sprite::GetComponentName() const 
{
    return s_ComponentNames[COMPONENT_SPRITE]; 
}

eComponentType Sprite::GetComponentType(void) const
{
    return COMPONENT_SPRITE;
}

//-----------------------------------------------------
// do some additional initialization steps
//-----------------------------------------------------
void Sprite::Init()
{
    pTransform_ = pOwner_->GetComponent<Transform>();
    assert(pTransform_);
}

//-----------------------------------------------------
// called each frame
//-----------------------------------------------------
void Sprite::Update(const float dt)
{
    const Vec2i pos = ToVec2i(pTransform_->GetPos());
    const Vec2i  sz = pTransform_->GetWorldSize();

    dstRect_ = RectiInit(pos, sz);

    // change position of the sprite on screen according 
    // to current position of the main camera
    if (!bFixedOnScreen_)
    {
        dstRect_.x0 -= g_GameState.camPos.x;
        dstRect_.x1 -= g_GameState.camPos.x;
        dstRect_.y0 -= g_GameState.camPos.y;
        dstRect_.y1 -= g_GameState.camPos.y;
    }

    // switch animation frames based on time
    if (bAnimated_)
    {
        currFrameTime_ += dt;

        // play this animation only once
        if (!pCurrAnimation_->bLooped)
        {
            if (pCurrAnimation_->bPlayedOnce)
                return;

            // stop at the last frame
            if (currAnimFrame_ == (pCurrAnimation_->numFrames - 1))
            {
                pCurrAnimation_->bPlayedOnce = true;
                return;
            }
        }
 
        // increment the current frame
        const float frameTimeSec = (float)pCurrAnimation_->speed * 0.001f;

        if (currFrameTime_ > frameTimeSec)
        {
            currFrameTime_ -= frameTimeSec;
            currAnimFrame_++;
            currAnimFrame_ %= pCurrAnimation_->numFrames;

            const Vec2i rectSz = { srcRect_.x1-srcRect_.x0, srcRect_.y1-srcRect_.y0 };

            srcRect_.x0 = (rectSz.w) * currAnimFrame_;
            srcRect_.x1 = srcRect_.x0 + rectSz.w;
            srcRect_.y0 = animIndex_ * rectSz.h;
            srcRect_.y1 = srcRect_.y0 + rectSz.h;
        }
    }
}

//-----------------------------------------------------
// is called each frame
// push this sprite into the render list
//-----------------------------------------------------
void Sprite::Render()
{
    if (!bShow_)
        return;

    const eLayerType layer = pOwner_->GetLayer();

    switch (layer)
    {
        case LAYER_GND_DECAL:       
        case LAYER_ENTITY:     
        case LAYER_UI:              
        {
            PushItemToRender(
                layer,
                pCurrAnimation_->texId,
                srcRect_,
                dstRect_,
                color_,
                spriteFlip_,
                zIndex_);
            break;
        }

        default:
            LogErr(LOG, "invalid layer (%d) for sprite rendering (entt: %s)", pOwner_->GetLayer(), pOwner_->GetName());
    }
}

//-----------------------------------------------------
// get texture ID of the current animation
//-----------------------------------------------------
TexID Sprite::GetTexId() const
{
    assert(pCurrAnimation_);
    return pCurrAnimation_->texId;
}

//-----------------------------------------------------
// get ID of texture related to animation by input type
//-----------------------------------------------------
TexID Sprite::GetTexId(eAnimType type) const
{ 
    assert(type >= 0 && type < NUM_ANIM_TYPES);
    return animations_.at(type).texId; 
}

//-----------------------------------------------------
// return a name of animation direction
//-----------------------------------------------------
const char* Sprite::GetAnimDirName(eAnimDir dir) const
{
    assert(dir >= 0 && dir < NUM_ANIM_DIRS);
    return s_AnimationDirNames[dir];
}

//-----------------------------------------------------
// return a name of animation type
//-----------------------------------------------------
const char* Sprite::GetAnimTypeName(eAnimType type) const
{
    assert(type >= 0 && type < NUM_ANIM_TYPES);
    return s_AnimationTypeNames[type];
}

//-----------------------------------------------------
// return an array of names for each type of animation
//-----------------------------------------------------
const char** Sprite::GetArrAnimTypeNames(void) const
{
    return s_AnimationTypeNames;
}

//-----------------------------------------------------
// STATIC METHOD
// return an animation by by input name of type
//-----------------------------------------------------
eAnimType Sprite::GetAnimType(const char* typeName)
{
    if (!typeName || typeName[0] == '\0')
    {
        LogErr(LOG, ERR_EMPTY_NAME);
        return NUM_ANIM_TYPES;
    }

    for (int i = 0; i < NUM_ANIM_TYPES; ++i)
    {
        if (strcmp(typeName, s_AnimationTypeNames[i]) == 0)
            return eAnimType(i);
    }

    LogErr(LOG, "no anim of type by name: %s", typeName);
    return NUM_ANIM_TYPES;
}

//-----------------------------------------------------
// setup animation frame index (is used to setup random initial frame when create entities with the same animations)
//-----------------------------------------------------
void Sprite::SetAnimFrame(const uint frameIdx)
{
    if (frameIdx >= pCurrAnimation_->numFrames)
        currAnimFrame_ = 0;

    currAnimFrame_ = frameIdx;
}

//-----------------------------------------------------
// setup color for this sprite
//-----------------------------------------------------
void Sprite::SetColor(uint8 r, uint8 g, uint8 b, uint8 a)
{
    color_.rgba = RGBATOU32(r, g, b, a);
}

//-----------------------------------------------------
// setup sprite size (width and height)
//-----------------------------------------------------
void Sprite::SetSrcRectSize(const Vec2i sz)
{
    assert(sz.w > 0 && sz.h > 0);
    srcRect_.x1 = srcRect_.x0 + sz.w;
    srcRect_.y1 = srcRect_.y0 + sz.h;
}

//-----------------------------------------------------
// setup z-index of sprite (setup the rendering order within LAYER_GND_DECAL and LAYER_UI)
//-----------------------------------------------------
void Sprite::SetZIndex(int zIndex)
{
    // clamp z-index to proper range
    zIndex  = (zIndex < INT8_MIN) ? INT8_MIN : zIndex;
    zIndex_ = (zIndex > INT8_MAX) ? INT8_MAX : zIndex;
}

//-----------------------------------------------------
// setup top-left position (in pixels) on the sprites texture
// (when using texture atlases we have to specify which part of the texture to use,
// if it isn't a texture atlas (full texture is used) then position is <0,0>)
//-----------------------------------------------------
void Sprite::SetSrcRectPos(const Vec2i p)
{ 
    // we have to store the old used width and height
    const Recti r = GetSrcRect();
    const Vec2i sz = { r.x1-r.x0, r.y1-r.y0 };

    srcRect_.x0 = p.x; 
    srcRect_.y0 = p.y; 

    if (sz.w > 0 && sz.h > 0)
        SetSrcRectSize(sz);
}

//-----------------------------------------------------
// get data of animation by input type
//-----------------------------------------------------
const Animation& Sprite::GetAnimByType(const eAnimType type)
{
    assert(type >= 0 && type < NUM_ANIM_TYPES);
    return animations_[type];
}

const Animation& Sprite::GetAnimByType(const eAnimType type) const
{
    assert(type >= 0 && type < NUM_ANIM_TYPES);
    return animations_.at(type);
}