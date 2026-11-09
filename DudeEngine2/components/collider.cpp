/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: collider.cpp
    Desc:     collider (physical boundaries) of the entity
              (not necessarily the same size as visible object)
    Created:  20.08.2026 by DimaSkup
\**********************************************************************************/
#include "collider.h"
#include "../log.h"
#include "../macros.h"
#include "../entity.h"
#include "transform.h"
//#include <string.h>


//-----------------------------------------------------
// default constructor
//-----------------------------------------------------
Collider::Collider() : Collider(Vec2i{0,0}, Vec2i{100,100})
{
    STATIC_ASSERT(NUM_COLLIDER_TYPES == ARRAY_SIZE(s_ColliderTypesNames));
}

//-----------------------------------------------------
// for cloning
//-----------------------------------------------------
Collider::Collider(const Collider* c)
{
    assert(c);

    if (c->type_ == COLLIDER_BOX)
        box_ = c->box_;

    else if (c->type_ == COLLIDER_CIRCLE)
        circle_ = c->circle_;

    else
        assert(0 && "dude, you fucked up somewhere with a type of your collider");

    type_   = c->type_;
    bSolid_ = c->bSolid_;
}

//-----------------------------------------------------
// for BOX colliders
//-----------------------------------------------------
Collider::Collider(Vec2i pos, Vec2i size) : bSolid_(true)
{
    if (size.w <= 0)
    {
        LogErr(LOG, "invalid input width (%d), set to default (%d)", size.w, DEFAULT_COLLIDER_BOX_SIZE);
        size.w = DEFAULT_COLLIDER_BOX_SIZE;
    }
    if (size.h <= 0)
    {
        LogErr(LOG, "invalid input height (%d), set to default (%d)", size.h, DEFAULT_COLLIDER_BOX_SIZE);
        size.h = DEFAULT_COLLIDER_BOX_SIZE;
    }

    assert(size.w > 0 && size.h > 0);
    type_ = COLLIDER_BOX;
    SetBoxPos(pos.x, pos.y);
    SetBoxSize(size.w, size.h);
}

//-----------------------------------------------------
// for CIRCLE colliders
//-----------------------------------------------------
Collider::Collider(Vec2i center, int radius) : bSolid_(true)
{
    if (radius <= 0)
    {
        LogErr(LOG, "invalid input radius (%d), set to default (%d)", radius, DEFAULT_COLLIDER_CIRCLE_RADIUS);
        radius = DEFAULT_COLLIDER_CIRCLE_RADIUS;
    }

    type_ = COLLIDER_CIRCLE;
    circle_.cx = center.x;
    circle_.cy = center.y;
    circle_.r  = radius;
}
    
//-----------------------------------------------------
    
void Collider::SetBoxPos(const int px, const int py)
{
    const Vec2i sz = GetBoxSize();
    box_.x0 = px;
    box_.x1 = px + sz.w;
    box_.y0 = py;
    box_.y1 = py + sz.h;
}
    
//-----------------------------------------------------

void Collider::SetBoxSize(int w, int h)
{
    if (w > 0 && h > 0)
    {
        box_.x1 = box_.x0 + w;
        box_.y1 = box_.y0 + h;
        return;
    }

    // we have invalid width or height
    const char* errMsg = "invalid input %s (%d), set to default (%d)";

    if (w <= 0)
    {
        LogErr(LOG, errMsg, "width", w, DEFAULT_COLLIDER_BOX_SIZE);
        h = DEFAULT_COLLIDER_BOX_SIZE;
    }
    if (h <= 0)
    {
        LogErr(LOG, errMsg, "height", h, DEFAULT_COLLIDER_BOX_SIZE);
        h = DEFAULT_COLLIDER_BOX_SIZE;
    }
}

//-----------------------------------------------------
// return a collider type by input name
//-----------------------------------------------------
eColliderType Collider::GetTypeByName(const char* name)
{
    assert(name && name[0] != '\0');

    for (int i = 0; i < NUM_COLLIDER_TYPES; ++i)
    {
        if (strcmp(s_ColliderTypesNames[i], name) == 0)
            return eColliderType(i);
    }

    return NUM_COLLIDER_TYPES;
}

//-----------------------------------------------------

void Collider::SetType(eColliderType type)
{
    // switch to BOX
    if (type == COLLIDER_BOX)
    {
        type_ = type;
        int sz = circle_.r;
        int halfSz = sz >> 1;
        SetBoxPos(circle_.cx-halfSz, circle_.cy-halfSz);
        SetBoxSize(sz, sz);
    }

    // switch to CIRCLE
    else if (type == COLLIDER_CIRCLE)
    {
        type_ = type;
        Vec2i p      = GetBoxPos();
        Vec2i sz     = GetBoxSize();
        Vec2i halfSz = Vec2iDiv(sz, 2);
        Vec2i center = Vec2iAdd(p, halfSz);
        int radius   = (int)Vec2Length(ToVec2(halfSz));

        SetCenter(center.x, center.y);
        SetRadius(radius);
    }

    else
    {
        LogErr(LOG, "invalid collider type: %d", (int)type);
    }
}

//-----------------------------------------------------

Recti Collider::GetBoxInWorld(void) const
{
    const Vec2i p = ToVec2i(pOwner_->GetComponent<Transform>()->GetPos());

    // x0 x1 y0 y1
    return { box_.x0 + p.x, box_.x1 + p.x, box_.y0 + p.y, box_.y1 + p.y };
}

//-----------------------------------------------------

CircleFixed Collider::GetCircleInWorld(void) const
{
    const Vec2i p = ToVec2i(pOwner_->GetComponent<Transform>()->GetPos());

    // centerX, centerY, radius
    return { circle_.cx + p.x, circle_.cy + p.y, circle_.r };
}