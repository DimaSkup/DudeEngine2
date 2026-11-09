/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: health.h

    Created:  30.05.2026 by DimaSkup
\**********************************************************************************/
#ifndef HEALTH_H
#define HEALTH_H

#include "component.h"
#include "../event_mgr.h"
#include <assert.h>


class Health : public Component
{
public:
    Health(const Health* h)
    {
        assert(h);
        healPerSec_ = h->healPerSec_;
        hp_         = h->hp_;
        maxHP_      = h->maxHP_;
        affectHP_   = h->affectHP_;
    }

    Health(const int hp, const int maxHP, const int healPerSec) : 
        hp_((float)hp),
        maxHP_((float)maxHP),
        healPerSec_(healPerSec)    
    {
        assert(hp > 0);
        assert(maxHP >= hp);
        assert(healPerSec >= 0);

        // affectHP - hp can increase or decrease according to this param
        affectHP_ = (float)healPerSec;
    }

    //-----------------------------------------------------

    virtual const char* GetComponentName() const override
    {
        return s_ComponentNames[COMPONENT_HEALTH];
    }

    virtual eComponentType GetComponentType() const override
    {
        return COMPONENT_HEALTH;
    }

    //-----------------------------------------------------

    virtual void Init() override {};
    virtual void Render() override {};

    //-----------------------------------------------------

    // affect the health during the time (heal or hit if poisoned, etc.)
    virtual void Update(const float dt)
    {
        if (hp_ < maxHP_)
            hp_ += (affectHP_ * dt);
    }

    //-----------------------------------------------------

    int GetHP() const
    {
        return (int)hp_;
    }

    int GetMaxHP() const
    {
        return (int)maxHP_;
    }

    int GetHealPerSec() const
    {
        return healPerSec_;
    }

    void Heal(const float hp)
    {
        assert(hp > 0.0f);
        hp_ += hp;
        if (hp_ > maxHP_)
            hp_ = maxHP_;
    }

    void GiveDamage(const float damage)
    {
        assert(damage > 0.0f);
        hp_ -= damage;
        
        EventData data;
        data.enttId = pOwner_->GetId();

        if (IsDead())
            EventMgr::PostEvent("ENTITY_DIED", &data);
        
        else
            EventMgr::PostEvent("DAMAGE_ENTT", &data);
    }

    bool IsDead() const
    {
        return hp_ <= 0.0f;
    }

private:
    int healPerSec_;    // healing factor per second

    float affectHP_;    // current health change factor (increase/decrease/no_change)
    float maxHP_;
    float hp_;
};

#endif
