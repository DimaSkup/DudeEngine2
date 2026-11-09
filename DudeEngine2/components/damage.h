/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: damage.h

    Created:  30.05.2026 by DimaSkup
\**********************************************************************************/
#ifndef DAMAGE_H
#define DAMAGE_H

#include "component.h"
#include <assert.h>


//-----------------------------------------------
// Component: attack related stuff
//-----------------------------------------------
class Damage : public Component
{
public:

    Damage(const Damage* d) : cooldownSec_(0)
    {
        assert(d);
        
        cooldownMaxSec_ = d->cooldownMaxSec_;
        damage_         = d->damage_;
    }

    //-----------------------------------------------------

    Damage(const float attackCooldownMaxSec,
           const float damage) 
        :
        cooldownSec_(0),
        cooldownMaxSec_(attackCooldownMaxSec),
        damage_(damage)
    {
        assert(attackCooldownMaxSec > 0);
        assert(damage > 0);
    }

    //-----------------------------------------------------

    virtual const char* GetComponentName() const override
    {
        return s_ComponentNames[COMPONENT_DAMAGE];
    }

    virtual eComponentType GetComponentType() const override
    {
        return COMPONENT_DAMAGE;
    }

    //-----------------------------------------------------

    virtual void Init() override {};
    virtual void Render() override {};

    //-----------------------------------------------------

    virtual void Update(const float dt)
    {
        cooldownSec_ -= dt;
    }

    //-----------------------------------------------------
    // after attack we wait some time (prevent attacking each frame)
    //-----------------------------------------------------
    void SetCooldownToMax()
    {
        cooldownSec_ = cooldownMaxSec_;
    }

    //-----------------------------------------------------
    // is used to prevent immediate attack by enemies when they are just come into attack range
    //-----------------------------------------------------
    void SetCooldownBeforeFirstAttack()
    {
        cooldownSec_ = cooldownMaxSec_*0.2f;
    }

    //-----------------------------------------------------
    // getters
    //-----------------------------------------------------
    float GetCurrCooldownSec() const { return cooldownSec_; }
    float GetCooldownMaxSec()  const { return cooldownMaxSec_; }
    float GetDamage()          const { return damage_; }

private:
    float cooldownSec_;         // time to wait (if <= 0 then we can attack)
    float cooldownMaxSec_;
    float damage_;
};

#endif
