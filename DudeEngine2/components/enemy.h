/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: enemy.h

    Created:  24.05.2026 by DimaSkup
\**********************************************************************************/
#ifndef ENEMY_H
#define ENEMY_H

#include "../vec.h"
#include "../game_state.h"
#include "../math_helpers.h"
#include "../macros.h"
#include "../entity.h"
#include "component.h"
#include "transform.h"
#include <stdlib.h>
#include <math.h>
#include <assert.h>


//---------------------------------------------------------
// enemies types
//---------------------------------------------------------
enum eEnemyType : uint8
{
    ENEMY_TYPE_BAT = 0,
    ENEMY_TYPE_SKELETON,
    ENEMY_TYPE_SPIDER,
    
    NUM_ENEMY_TYPES,
};

static const char* s_EnemyTypeNames[] = {
    "bat",
    "skeleton",
    "spider"
};

//---------------------------------------------------------
// enumeration of AI states
//---------------------------------------------------------
enum eAIState
{
    AI_IDLE,      // default state 
    AI_MOVE,      // walk, run, etc.
    AI_ATTACK,
    AI_DEATH,     // dies
    AI_DAMAGE,    // takes damage
    AI_BLOCK,     // block income attacks
    AI_CHASE,     // chase the player

    NUM_AI_STATES
};

static const char* s_AIStateNames[] = {
    "idle",
    "move",
    "attack",
    "death",
    "damage",
    "block",
    "chase"
};

//-------------------------------------
// component
//-------------------------------------
class Enemy : public Component
{
    
public:
    static const int ENEMY_SPEED = 150;
    static const int SIGHT_RADIUS = 500;
    static const int ATTACK_RADIUS = 70;

public:
    // for cloning the component
    Enemy(const Enemy* pEnemy) : 
        currStateAI_(eAIState(0)),
        moveTo_{ 0,0 },
        idlingTimer_(-1)
    {
        assert(pEnemy);
        type_ = pEnemy->type_;
    }

    // for component initialization
    Enemy(const eEnemyType type) :
        currStateAI_(eAIState(0)),
        moveTo_{0,0},
        idlingTimer_(-1)
    {
        STATIC_ASSERT(NUM_AI_STATES == ARRAY_SIZE(s_AIStateNames));
        assert(type >= 0 && type < NUM_ENEMY_TYPES);
        type_ = type;
    }

    //-----------------------------------------------------

    virtual const char* GetComponentName() const override
    {
        return s_ComponentNames[COMPONENT_ENEMY];
    }

    virtual eComponentType GetComponentType() const override
    {
        return COMPONENT_ENEMY;
    }
    
    //-----------------------------------------------------

    virtual void Init() override {}
    virtual void Render() override {}

    //-----------------------------------------------------

    static inline Vec2 RandDir()
    {
        const float angle = ((float)rand() / (float)RAND_MAX) * (float)M_2PI;
        return Vec2{ cosf(angle), sinf(angle) };
    }

    //-----------------------------------------------------

    virtual void Update(const float dt)
    {
        idlingTimer_ -= dt;
    }

    //-----------------------------------------------------

    inline float GetIdleTime() const
    {
        return idlingTimer_;
    }

    //-----------------------------------------------------

    inline void SetMoveTo(Vec2 to)
    {
        Vec2Clamp(to, { 0,0 }, g_GameState.fWorldSize);   // clamp to world boundings
        moveTo_ = to;
        idlingTimer_ = 0;
    }

    //-----------------------------------------------------
    
    inline bool IsIdling() const
    {
        return (idlingTimer_ > 0.0f);
    }
    
    //-----------------------------------------------------
    
    inline void SetIdlingForMs(const float ms)
    {
        idlingTimer_ = ms * 0.001f;
    }
    
    //-----------------------------------------------------

    inline Vec2 GetMoveTo() const
    {
        return moveTo_;
    }

    //-----------------------------------------------------

    inline bool IsInDestPoint() const
    {
        const Vec2 p = pOwner_->GetComponent<Transform>()->GetPos();
        return (Vec2Dot(Vec2Sub(p, moveTo_)) < 100.0f);
    }

    //-----------------------------------------------------
    
    inline void SetState(eAIState state)
    {
        assert(state >= 0 && state < NUM_AI_STATES);
        currStateAI_ = state;
    }

    //-----------------------------------------------------
    
    inline Vec2 GetVelocity(void) const
    {
        return Vec2Scale(dir_, ENEMY_SPEED);
    }

    //-----------------------------------------------------
    
    inline void SetDir(const Vec2 dir)
    {
        dir_ = dir;
    }

    inline Vec2 GetDir(void) const
    {
        return dir_;
    }
    
    //-----------------------------------------------------

    inline eAIState GetCurrState() const
    {
        return currStateAI_;
    }

    //-----------------------------------------------------
    
    inline const char* GetCurrStateName() const
    {
        return s_AIStateNames[currStateAI_];
    }
    
    //-----------------------------------------------------

    inline int GetAttackRadius() const
    {
        return ATTACK_RADIUS;
    }

    //-----------------------------------------------------
    // define an enemy type by its typename
    //-----------------------------------------------------
    static eEnemyType GetTypeByTypeName(const char* typeName)
    {
        assert(typeName && typeName[0] != '\0');

        for (int i = 0; i < NUM_ENEMY_TYPES; ++i)
        {
            if (strcmp(typeName, s_EnemyTypeNames[i]) == 0)
                return eEnemyType(i);
        }

        return NUM_ENEMY_TYPES;
    }

    //-----------------------------------------------------
    // get typename of enemy by its type
    //-----------------------------------------------------
    const char* GetTypeAsStr() const
    {
        assert(type_ >= 0 && type_ < NUM_ENEMY_TYPES);
        return s_EnemyTypeNames[type_];
    }

    //-----------------------------------------------------

    eEnemyType GetType() const { return type_; }


private:
    eAIState   currStateAI_;
    Vec2       moveTo_;                // target point
    Vec2       dir_;                   // movement direction (if <0,0> then enemy don't move)
    float      idlingTimer_;           // enemy will be in IDLE for this number of seconds
    eEnemyType type_;                  // which kind of enemies does this eneity belongs to
    bool       bActive_;
};

#endif
