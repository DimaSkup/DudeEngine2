/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: keyboard.h
    Desc:     a component for keyboard control

    Created:  by DimaSkup
\**********************************************************************************/
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../macros.h"
#include "../input.h"
#include "../game_state.h"
#include "../event_mgr.h"

#include "component.h"


//-----------------------------------------------
// enumeration of keys by assignment
//-----------------------------------------------
enum eKeyTypes
{
    KEY_PLAYER_UP,
    KEY_PLAYER_DOWN,
    KEY_PLAYER_RIGHT,
    KEY_PLAYER_LEFT,

    NUM_KEY_TYPES
};

//-----------------------------------------------
// names of keys by assignment
//-----------------------------------------------
static const char* s_KeyTypeNames[] =
{
    "up", "down", "right", "left"
};

//-----------------------------------------------
// component
//-----------------------------------------------
class Keyboard : public Component
{
public:
    Keyboard()
    {
        STATIC_ASSERT(NUM_KEY_TYPES == ARRAY_SIZE(s_KeyTypeNames));
    }

     Keyboard(
        const char* upKey,
        const char* leftKey,
        const char* downKey,
        const char* rightKey)
    {
        STATIC_ASSERT(NUM_KEY_TYPES == ARRAY_SIZE(s_KeyTypeNames));

        // check input args
        assert(upKey    && upKey[0] != '\0');
        assert(leftKey  && leftKey[0] != '\0');
        assert(downKey  && downKey[0] != '\0');
        assert(rightKey && rightKey[0] != '\0');

        // string -> keycode
        keys_[KEY_PLAYER_UP]    = GetKeycodeByName(upKey);
        keys_[KEY_PLAYER_LEFT]  = GetKeycodeByName(leftKey);
        keys_[KEY_PLAYER_DOWN]  = GetKeycodeByName(downKey);
        keys_[KEY_PLAYER_RIGHT] = GetKeycodeByName(rightKey);

        // check results
        assert(keys_[KEY_PLAYER_UP]    != KEY_MAX);
        assert(keys_[KEY_PLAYER_LEFT]  != KEY_MAX);
        assert(keys_[KEY_PLAYER_DOWN]  != KEY_MAX);
        assert(keys_[KEY_PLAYER_RIGHT] != KEY_MAX);
    }

    //-----------------------------------------------------
    
     virtual const char* GetComponentName() const override
     {
         return s_ComponentNames[COMPONENT_KEYBOARD];
     }

     virtual eComponentType GetComponentType() const override
     {
         return COMPONENT_KEYBOARD;
     }
    
    //-----------------------------------------------------
    
    virtual void Init()   override {}
    virtual void Render() override {}
    
    //-----------------------------------------------------

    virtual void Update(const float dt) override
    {
        // process player keyboard input only when in the game mode
        if (!g_GameState.bGameMode)
            return;

        const uint8* keysStates = GetKeyboardState();
        Vec2 vel = { 0, 0 };
        EventData data;
        data.enttId = pOwner_->GetId();

        // handle movement
        if (keysStates[keys_[KEY_PLAYER_UP]])      vel.y = -1;
        if (keysStates[keys_[KEY_PLAYER_RIGHT]])   vel.x = +1;
        if (keysStates[keys_[KEY_PLAYER_DOWN]])    vel.y = +1;
        if (keysStates[keys_[KEY_PLAYER_LEFT]])    vel.x = -1;


        // we're moving now
        if (Vec2Dot(vel) > 0)
        {
            data.fx = vel.x;
            data.fy = vel.y;
            EventMgr::PostEvent("entt_keyboard_move", &data);
        }
        else
        {
            EventMgr::PostEvent("entt_nomove", &data);
        }
    }

    //-----------------------------------------------------

    eKeyCode GetKeyCodeByType(const eKeyTypes type) const
    {
        if (type >= 0 && type < NUM_KEY_TYPES)
            return keys_[type];

        return KEY_MAX;
    }

private:
    eKeyCode keys_[NUM_KEY_TYPES];       // keys binding
};

#endif