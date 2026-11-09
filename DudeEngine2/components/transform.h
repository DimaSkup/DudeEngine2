/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: transform.h

    Created:  by DimaSkup
\**********************************************************************************/
#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include "../vec.h"
#include "../rect.h"
#include "component.h"

//---------------------------------------------------------
// a TRANSFORM component for Entity-Component
//---------------------------------------------------------
class Transform : public Component
{
public:
    Transform(const Transform* t);
    Transform(float px, float py, int w, int h, int scale);
    Transform(Vec2 pos, Vec2i size, int scale);
    Transform(Vec2i pos, Vec2i size, int scale);

    virtual const char*    GetComponentName() const override;
    virtual eComponentType GetComponentType() const override;

    virtual void        Update(const float dt) override {};
    virtual void        Init()   override {};
    virtual void        Render() override {};

    void                Translate(const Vec2 offset);
    void                SetPos(const Vec2i pos);
    void                SetPos(const Vec2 pos);
    void                SetPos(const float x, const float y);
    void                SetLocalSize(const Vec2i sz);
    void                SetScale(const int scale);

    Vec2                GetPos() const;
    Vec2i               GetPosi() const;
    Vec2i               GetLocalSize() const;
    int                 GetScale() const;

    Rect                GetRect() const;
    Recti               GetRecti() const;
    Vec2i               GetWorldSize() const;
    Vec2                GetCenter() const;

private:
    Vec2 pos_;      // position
    int width_;     // width in local space
    int height_;    // height in local space
    int scale_;     // scale in world space
};

#endif
