/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: movement.cpp
\**********************************************************************************/
#include "movement.h"
#include "transform.h"
#include "../entity.h"
#include <assert.h>

//---------------------------------------------------------
// constructors
//---------------------------------------------------------

Movement::Movement() : vel_(50, 50)
{
}

Movement::Movement(const float vx, const float vy) : vel_(vx, vy)
{
}

//---------------------------------------------------------
// get a name / type of this component
//---------------------------------------------------------
const char* Movement::GetComponentName() const
{
    return s_ComponentNames[COMPONENT_MOVEMENT];
}

eComponentType Movement::GetComponentType() const
{
    return COMPONENT_MOVEMENT;
}

//---------------------------------------------------------
// update this component
//---------------------------------------------------------
void Movement::Update(const float dt)
{
    assert(pOwner_);

    const Vec2 offset = Vec2Mul(vel_, dt);
    pOwner_->GetComponent<Transform>()->Translate(offset);
}

//---------------------------------------------------------
// setup velocity
//---------------------------------------------------------
void Movement::SetVelocity(const float vx, const float vy)
{
    vel_ = { vx, vy };
}

//---------------------------------------------------------
// return current velocity
//---------------------------------------------------------
Vec2 Movement::GetVelocity(void) const
{
    return vel_;
}