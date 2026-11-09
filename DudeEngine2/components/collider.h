/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: collider.h
    Desc:     collider (physical boundaries) of the entity
              (not necessarily the same size as visible object)
    Created:  17.06.2026 by DimaSkup
\**********************************************************************************/
#ifndef COLLIDER_H
#define COLLIDER_H

#include "../circle.h"
#include "../rect.h"
#include "component.h"
#include <assert.h>


//-----------------------------------------------
// constants, enums, static data
//-----------------------------------------------
constexpr int DEFAULT_COLLIDER_BOX_SIZE = 50;
constexpr int DEFAULT_COLLIDER_CIRCLE_RADIUS = 50;

enum eColliderType
{
    COLLIDER_BOX,
    COLLIDER_CIRCLE,
    NUM_COLLIDER_TYPES
};

static const char* s_ColliderTypesNames[] =
{
    "box",
    "circle",
};

//-----------------------------------------------
// component:  Collider
//-----------------------------------------------
class Collider : public Component
{
public:

    Collider();
    Collider(const Collider* c);
    Collider(Vec2i pos, Vec2i size);
    Collider(Vec2i center, int radius);
    
    //-----------------------------------------------------

    virtual const char* GetComponentName() const override 
    { 
        return s_ComponentNames[COMPONENT_COLLIDER]; 
    }

    virtual eComponentType GetComponentType() const override
    {
        return COMPONENT_COLLIDER;
    }

    //-----------------------------------------------------
    
    virtual void Update(const float dt) {};
    virtual void Init() override {};
    virtual void Render() override {};

    //-----------------------------------------------------

    void SetBoxPos(const int px, const int py);
    void SetBoxSize(int width, int height);
    void SetType(eColliderType type);

    //-----------------------------------------------------

    static eColliderType GetTypeByName(const char* name);

    //-----------------------------------------------------
    
    Vec2i GetBoxPos(void) const
    {
        return { box_.x0, box_.y0 };
    }

    //-----------------------------------------------------

    Vec2i GetBoxSize(void) const
    {
        return { box_.x1 - box_.x0, box_.y1 - box_.y0 };
    }

    //-----------------------------------------------------

    const Recti& GetBox(void) const
    {
        return box_;
    }
    
    //-----------------------------------------------------
    
    Recti GetBoxInWorld(void) const;
    CircleFixed GetCircleInWorld(void) const;

    //-----------------------------------------------------
    
    Vec2i GetCenter(void) const
    {
        return { circle_.cx, circle_.cy };
    }

    //-----------------------------------------------------

    CircleFixed GetCircle(void) const
    {
        return circle_;
    }

    //-----------------------------------------------------

    void SetCenter(int cx, int cy)
    {
        circle_.cx = cx;
        circle_.cy = cy;
    }
    
    //-----------------------------------------------------

    int GetRadius(void) const
    {
        return circle_.r;
    }

    //-----------------------------------------------------

    void SetRadius(int r)
    {
        assert(r > 0 && "radius can't be <= 0");
        circle_.r = r;
    }

    //-----------------------------------------------------

    // Args: true - entities can't move through this collider
    void SetSolid(const bool bSolid)
    {
        bSolid_ = bSolid;
    }

    // Ret: true - entities can't move through this collider
    bool IsSolid(void) const
    {
        return bSolid_;
    }

    //-----------------------------------------------------

    const char* GetColliderTypeName(eColliderType type) const
    {
        assert(type >= 0 && type < NUM_COLLIDER_TYPES);
        return s_ColliderTypesNames[type];
    }

    //-----------------------------------------------------

    eColliderType GetType(void) const
    {
        return type_;
    }

private:
    union {
        Recti       box_;
        CircleFixed circle_;
    };

    eColliderType type_;
    bool          bSolid_;      // enable/disable walking through this collider (true - can't move through)
};

#endif