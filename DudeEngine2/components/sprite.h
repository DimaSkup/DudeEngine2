/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: sprite.h

    Created:  by DimaSkup
\**********************************************************************************/
#ifndef SPRITE_COMPONENT_H
#define SPRITE_COMPONENT_H

#include "../types.h"
#include "../r_list.h"        // render list
#include "component.h"
#include "transform.h"        // component
#include <map>


//---------------------------------------------------------
// direction for animation
//---------------------------------------------------------
enum eAnimDir : uint8
{
    ANIM_UP,
    ANIM_RIGHT,
    ANIM_DOWN,
    ANIM_LEFT,

    ANIM_NODIR,     // no directions

    NUM_ANIM_DIRS,
};

//---------------------------------------------------------
// animations types
//---------------------------------------------------------
enum eAnimType : uint8
{
    ANIM_TYPE_IDLE,        // default state (is also used for a "single" animation type)
    ANIM_TYPE_MOVE,        // for movement animations
    ANIM_TYPE_ATTACK,      // for attack animations
    ANIM_TYPE_DEATH,       // for death animations
    ANIM_TYPE_DAMAGE,      // for animations of getting damage
    ANIM_TYPE_BLOCK,       // for animations of blocking attacks

    NUM_ANIM_TYPES
};

//---------------------------------------------------------

enum eAnimPreset
{
    ANIM_PRESET_NO_ANIM = 0,    // static non-animated sprite
    ANIM_PRESET_SINGLE_ANIM,    // animated sprite: one animation type (by default it is idle)
    ANIM_PRESET_MULTIPLE_ANIM,  // animated sprite: multiple animation types (for different directions and )

    NUM_ANIM_PRESETS,
};

//---------------------------------------------------------

using AnimationsMap = std::map<eAnimType, Animation>;

//---------------------------------------------------------
// a component for 2D sprites
//---------------------------------------------------------
class Sprite : public Component
{
public:
    Sprite();

    // for copying sprite's data
    Sprite(const Sprite* s);

    // non-animated sprite
    Sprite(const TexID texId, const bool bFixedOnScreen);

    // animated sprite: one animation type (idle)
    Sprite(const Animation& anim, const bool bFixedOnScreen);

    // animated sprite: multiple animation types
    Sprite(const Animation& idle,
           const Animation& move,
           const Animation& attack,
           const Animation& death,
           const Animation& damage,
           const Animation& block,
           const bool bFixedOnScreen);

    //-----------------------------------------------------
    // virtual methods
    //-----------------------------------------------------
    virtual const char*    GetComponentName(void) const override;
    virtual eComponentType GetComponentType(void) const override;
    virtual void           Init            (void) override;
    virtual void           Update          (const float dt) override;
    virtual void           Render          (void) override;


    //-----------------------------------------------------
    // setters / getters
    //-----------------------------------------------------
    void             SetAnimPreset(const eAnimPreset preset);
                     
    void             SetAnimDir   (const eAnimDir direction);
    void             PlayAnimation(const eAnimType type);
    void             SetupAnimation(const eAnimType type, const Animation& anim);

    void             SetTex(eAnimType type, TexID texId);
    TexID            GetTex(void) const;
    TexID            GetTex(eAnimType type) const;

    void             SetAnimFrame(const uint frameIdx);
    void             SetColor(uint8 r, uint8 g, uint8 b, uint8 a);
    void             SetSrcRectSize(const Vec2i sz);


    const char*      GetAnimDirName (eAnimDir dir)   const;
    const char*      GetAnimTypeName(eAnimType type) const;
    const char**     GetArrAnimTypeNames(void) const;
    static eAnimType GetAnimType(const char* typeName);

    void             SetZIndex(int zIndex);
    void             SetSrcRectPos(const Vec2i p);

    //-----------------------------------------------------
    // inline setters
    //-----------------------------------------------------
    void SetShow(const bool onOff)           { bShow_ = onOff; }
    void SetColor(const Color c)             { color_ = c; }
    void SetAlpha(const uint8 a)             { color_.a = a; }

    //-----------------------------------------------------
    // inline getters
    //-----------------------------------------------------
    const Recti&         GetSrcRect()           const { return srcRect_; }
    const Recti&         GetDstRect()           const { return dstRect_; }
    const AnimationsMap& GetAnimations()        const { return animations_; }
    eAnimDir             GetCurrAnimDir()       const { return currAnimDir_; }
    eAnimType            GetCurrAnimType()      const { return currAnimType_; }
    Color                GetColor()             const { return color_;  }
    int                  GetNumFrames()         const { return pCurrAnimation_->numFrames; }
    int                  GetAnimationSpeed()    const { return pCurrAnimation_->speed; }
    int8                 GetZIndex()            const { return zIndex_; }
    bool                 HasMultipleAnimTypes() const { return animations_.size() > 1; }
    bool                 IsAnimated()           const { return bAnimated_; }
    bool                 IsFixedOnScreen()      const { return bFixedOnScreen_; }
    bool                 IsShow()               const { return bShow_; }

    const Animation& GetAnimByType(eAnimType type);
    const Animation& GetAnimByType(eAnimType type) const;

private:
    Transform*      pTransform_;            // component

    Recti           srcRect_;               // which part of the texture atlas we will show
    Recti           dstRect_;               // where we will render this sprite (based on entity's transformation)
    AnimationsMap   animations_;            // animations data
    
    // CURRENT
    Animation*      pCurrAnimation_;
    uint            currAnimFrame_;
    float           currFrameTime_;
    uint            animIndex_;             // animation index (which kind of animation is used on the texture atlas)
    Color           color_;                 // tint: (rgb + alpha)

    eAnimDir        currAnimDir_;           // currently selected direction
    eAnimType       currAnimType_;          // currently selected animation
    eRendererFlip   spriteFlip_;
    int8            zIndex_;                // is used to setup drawing order of the sprite within its layer (LAYER_GND_DECAL and LAYER_UI)
    bool            bAnimated_;             // is this sprite animated?
    bool            bFixedOnScreen_;        // is this sprite fixed on the screen when we move?
    bool            bShow_;                 // is this sprite visible?
};

#endif
