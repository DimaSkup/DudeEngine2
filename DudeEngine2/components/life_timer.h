/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: life_timer.h
    Desc:     entity with this component will die after some amount of time

    Created:  27.07.2026 by DimaSkup
\**********************************************************************************/
#ifndef LIFE_TIMER_H
#define LIFE_TIMER_H

#include "component.h"
#include <assert.h>

//-------------------------------------
// component
//-------------------------------------
class LifeTimer : public Component
{
private:
    float lifeTime_;     // life duration of entity, when <= 0 it will die
    float currTime_;     // time before death (is reduced each frame)

public:

    // for copying components's data (when cloning entities)
    LifeTimer(const LifeTimer* t)
    {
        assert(t);
        lifeTime_ = t->lifeTime_;
        currTime_ = t->currTime_;
    }

    // init the component
    LifeTimer(const float t) : lifeTime_(t), currTime_(t)
    {
        assert(t > 0 && "invalid lifetime");
    }

    //-----------------------------------------------------

    virtual const char* GetComponentName() const override
    {
        return s_ComponentNames[COMPONENT_LIFE_TIMER];
    }

    virtual eComponentType GetComponentType() const override
    {
        return COMPONENT_LIFE_TIMER;
    }
    
    //-----------------------------------------------------

    virtual void Init() override {}
    virtual void Render() override {}
    
    //-----------------------------------------------------
    
    virtual void Update(const float dt)
    {
        currTime_ -= dt;
    }
    
    //-----------------------------------------------------
    
    float GetFullLifeTime() const
    {
        return lifeTime_;
    }
    
    //-----------------------------------------------------

    float GetRemainLifeTime() const
    {
        return currTime_;
    }
};

#endif
