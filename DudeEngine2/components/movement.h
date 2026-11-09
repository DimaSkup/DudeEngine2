/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: movement.h
    Desc:     a component to make entities move in a particular direction during the time
    Created:  25.06.2026 by DimaSkup
\**********************************************************************************/
#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "component.h"
#include "../vec.h"

class Movement : public Component
{
public:
    Movement();
    Movement(const float vx, const float vy);

    virtual void        Update(const float dt) override;
    virtual void        Init(void) override {}
    virtual void        Render(void) override {}

    void                SetVelocity(const float vx, const float vy);
    Vec2                GetVelocity(void) const;

    //-------------------------------------------

    virtual const char*    GetComponentName() const override;
    virtual eComponentType GetComponentType() const override;

private:
    Vec2 vel_;
};

#endif