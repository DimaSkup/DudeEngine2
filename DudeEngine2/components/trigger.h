/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: trigger.h
    Desc:     a component for triggers 
              (if something intersects a trigger's boundaries then
              we will do some event by a name)

    Created:  by DimaSkup
\**********************************************************************************/
#ifndef TRIGGER_H
#define TRIGGER_H

#include "../types.h"
#include "component.h"

#include "../collision.h"
#include "../entity.h"
#include "collider.h"
#include <assert.h>


enum eTriggerState : uint8
{
    TRIGGER_INACTIVE,
    TRIGGER_ON_ENTER,       // entity just come into this trigger
    TRIGGER_ON_COLLIDE,     // entity is already in the trigger for some time
    TRIGGER_ON_LEAVE,       // entity just left this trigger

    NUM_TRIGGER_STATES
};

enum eTriggerType : uint8
{
    TRIGGER_ONCE,           // trigger can be activated only once
    TRIGGER_MULTIPLE,       // trigger can be activated multiple times

    NUM_TRIGGER_TYPES
};

static const char* s_TriggerTypesNames[] =
{
    "once",
    "multiple"
};

//---------------------------------------------------------
// data used when trigger is activated
//---------------------------------------------------------
struct Triggered
{
    EntityID triggerId;
    EventID  eventId;
};

//---------------------------------------------------------
// initial params for a new trigger
//---------------------------------------------------------
struct TriggerInitParams
{
    EventID      onEnter;
    EventID      onCollide;
    EventID      onLeave;
    eTriggerType type;
    bool         bTriggersOnlyByPlayer;
};

//---------------------------------------------------------
// component:  Trigger
//---------------------------------------------------------
class Trigger : public Component
{
public:

    Trigger()
    {
        STATIC_ASSERT(NUM_TRIGGER_TYPES == ARRAY_SIZE(s_TriggerTypesNames));
    }

    // for cloning
    Trigger(const Trigger* t)
    {
        assert(t);

        onEnter_               = t->onEnter_;
        onCollide_             = t->onCollide_;
        onLeave_               = t->onLeave_;
        type_                  = t->type_;
        state_                 = TRIGGER_INACTIVE;
        bTriggersOnlyByPlayer_ = t->bTriggersOnlyByPlayer_;
    }

    // for initialization
    Trigger(const TriggerInitParams& params) :
        onEnter_(params.onEnter),
        onCollide_(params.onCollide),
        onLeave_(params.onLeave),
        type_(params.type),
        state_(TRIGGER_INACTIVE),
        bTriggersOnlyByPlayer_(params.bTriggersOnlyByPlayer)
    {
        assert(params.type >= 0 && params.type < NUM_TRIGGER_TYPES);
    }

    //-----------------------------------------------------

    virtual const char* GetComponentName() const override 
    { 
        return s_ComponentNames[COMPONENT_TRIGGER];
    }

    virtual eComponentType GetComponentType() const override 
    { 
        return COMPONENT_TRIGGER; 
    }

    //-----------------------------------------------------

    virtual void  Update(const float dt) {}
    virtual void  Init  (void) {}
    virtual void  Render(void) {}

    //-----------------------------------------------------

    EventID       GetOnEnter(void)    const { return onEnter_; }
    EventID       GetOnCollide(void)  const { return onCollide_; }
    EventID       GetOnLeave(void)    const { return onLeave_; }

    void          SetOnEnter  (const EventID id) { onEnter_ = id; }
    void          SetOnCollide(const EventID id) { onCollide_ = id; }
    void          SetOnLeave  (const EventID id) { onLeave_ = id;}

    eTriggerType  GetType(void)  const { return type_; }
    eTriggerState GetState(void) const { return state_; }

    //-----------------------------------------------------

    void SetType(const eTriggerType t)
    {
        assert(t >= 0 && t < NUM_TRIGGER_TYPES);
        type_ = t;
    }

    void SetState(const eTriggerState s)
    {
        assert(s >= 0 && s < NUM_TRIGGER_STATES);
        state_ = s;
    }

    bool IsTriggersOnlyByPlayer(void) const
    {
        return bTriggersOnlyByPlayer_;
    }

    static const char* GetNameByType(const eTriggerType type)
    {
        if (type < 0 || type >= NUM_TRIGGER_TYPES)
        {
            LogErr(LOG, "no name by type: %d", (int)type);
            return "invalid";
        }

        return s_TriggerTypesNames[type];
    }

    static eTriggerType GetTypeByName(const char* nameOfType)
    {
        if (!nameOfType || nameOfType[0] == '\0')
        {
            LogErr(LOG, ERR_EMPTY_NAME);
            return NUM_TRIGGER_TYPES;
        }

        for (int i = 0; i < NUM_TRIGGER_TYPES; ++i)
        {
            if (strcmp(nameOfType, s_TriggerTypesNames[i]) == 0)
                return eTriggerType(i);
        }

        LogErr(LOG, "no type by name: %s", nameOfType);
        return NUM_TRIGGER_TYPES;
    }

private:
    EventID onEnter_;
    EventID onCollide_;
    EventID onLeave_;

    eTriggerType  type_;
    eTriggerState state_;
    bool bTriggersOnlyByPlayer_;
};

#endif