/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: transform.cpp
\**********************************************************************************/
#include "transform.h"
#include "../log.h"
#include "../entity.h"
#include <assert.h>

//---------------------------------------------------------
// used to clone component
//---------------------------------------------------------
Transform::Transform(const Transform* t)
{
    assert(t);

    // init transformation
    pos_    = { 0, 0 };
    width_  = t->width_;
    height_ = t->height_;
    scale_  = t->scale_;
}

//---------------------------------------------------------
// pos - position
//---------------------------------------------------------
Transform::Transform(
    const float posX,
    const float posY,
    const int width,
    const int height,
    const int scale)
{
    // check input args
    if (posX < 0 || posY < 0)
        LogErr(LOG, "invalid initial position (%f:%f)", posX, posY);

    if (width <= 0 || height <= 0)
        LogErr(LOG, "invalid initial size (%dx%d)", width, height);

    if (scale <= 0)
        LogErr(LOG, "invalid initial scale (%d)", scale);

    // setup transformation
    pos_ = { posX, posY };
    width_ = width;
    height_ = height;
    scale_ = scale;
}

//-----------------------------------------------------

Transform::Transform(
    const Vec2 pos,
    const Vec2i size,
    const int scale)
{
    // check input args
    if (pos.x < 0 || pos.y < 0)
        LogErr(LOG, "invalid initial position (%f:%f)", pos.x, pos.y);

    if (size.w <= 0 || size.h <= 0)
        LogErr(LOG, "invalid initial size (%dx%d)", size.w, size.h);

    if (scale <= 0)
        LogErr(LOG, "invalid initial scale (%d)", scale);

    // setup transformation
    pos_ = pos;
    width_ = size.w;
    height_ = size.h;
    scale_ = scale;
}

//-----------------------------------------------------

Transform::Transform(Vec2i pos, Vec2i size, int scale) 
    : Transform::Transform(ToVec2(pos), size, scale)
{
}

//-----------------------------------------------------

void Transform::Translate(const Vec2 offset)
{
    pos_.x += offset.x;
    pos_.y += offset.y;
}

//-----------------------------------------------------

void Transform::SetPos(const Vec2i pos)
{
    pos_ = ToVec2(pos);
}

//-----------------------------------------------------

void Transform::SetPos(const Vec2 pos)
{
    pos_ = pos;
}

//-----------------------------------------------------

void Transform::SetPos(const float x, const float y)
{
    pos_ = { x,y };
}

//-----------------------------------------------------

void Transform::SetLocalSize(const Vec2i sz)
{
    if (sz.w <= 0 || sz.h <= 0)
    {
        LogErr(LOG, "invalid size (%d, %d) for entity (id: %d, name: %s)", sz.w, sz.h, pOwner_->GetId(), pOwner_->GetName());
        return;
    }

    width_ = sz.w;
    height_ = sz.h;
}

//-----------------------------------------------------

void Transform::SetScale(const int scale)
{
    if (scale <= 0)
    {
        LogErr(LOG, "invalid scale (%d) for entity (id: %d, name: %s)", scale, pOwner_->GetId(), pOwner_->GetName());
        return;
    }

    scale_ = scale;
}

//-----------------------------------------------------
// get a name / type of this component
//-----------------------------------------------------
const char* Transform::GetComponentName() const
{
    return s_ComponentNames[COMPONENT_TRANSFORM];
}

eComponentType Transform::GetComponentType() const
{
    return COMPONENT_TRANSFORM;
}

//-------------------------------------------

Rect Transform::GetRect() const
{
    // x0, x1, y0, y1
    return { pos_.x, pos_.x + width_*scale_, pos_.y, pos_.y + height_*scale_ };
}

//-------------------------------------------

Recti Transform::GetRecti() const
{
    const Vec2i p = ToVec2i(pos_);

    // x0, x1, y0, y1
    return { p.x, p.x + width_*scale_, p.y, p.y + height_*scale_ };
}

//-------------------------------------------

Vec2 Transform::GetPos() const
{
    return pos_;
}

//-------------------------------------------

Vec2i Transform::GetPosi() const
{
    return ToVec2i(pos_);
}

//-------------------------------------------

Vec2i Transform::GetLocalSize() const
{
    return { width_, height_ };
}

//-------------------------------------------

int Transform::GetScale() const
{
    return scale_;
}

//-------------------------------------------

Vec2i Transform::GetWorldSize() const
{
    return { width_*scale_, height_*scale_ };
}

//-------------------------------------------

Vec2 Transform::GetCenter() const
{
    const Vec2 sz = ToVec2(GetWorldSize());
    return { pos_.x + (sz.x * 0.5f), pos_.y + (sz.y * 0.5f) };
}