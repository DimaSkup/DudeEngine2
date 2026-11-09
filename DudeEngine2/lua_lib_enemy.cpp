/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: lua_lib_enemy.cpp
    Desc:     a library for using in Lua to work with enemies
    Created:  26.07.2026 by DimaSkup
\**********************************************************************************/
#include "lua_lib_enemy.h"
#include "types.h"
#include "entity_mgr.h"
#include "components/enemy.h"
#include "components/health.h"
#include "components/damage.h"
#include "components/name.h"
#include <assert.h>


// the library name
static const char* s_LibName = "enemy";

//---------------------------------------------------------
// set a state of enemy by ID
//---------------------------------------------------------
static int SetState(lua_State* L)
{
    assert(L);
    const EntityID id      = (EntityID)luaL_checkinteger(L, 1);
    const eAIState aiState = (eAIState)luaL_checkinteger(L, 2);
    Entity*        pEntt   = g_EnttMgr.GetEntt(id);

    assert(aiState >= 0 && aiState < NUM_AI_STATES);
    assert(pEntt->HasComponent<Enemy>());

    pEntt->GetComponent<Enemy>()->SetState(aiState);
    return 0;   // number of results
}

//---------------------------------------------------------
// check if enemy by ID is currently seeing the main player
//---------------------------------------------------------
static int IsSeePlayer(lua_State* L)
{
    assert(L);

    bool bSeePlayer = false;

    if (!g_GameState.bPlayerInvisibleToEnemy)
    {
        const EntityID enemyId = (EntityID)luaL_checkinteger(L, 1);
        const Entity*  pPlayer = g_EnttMgr.GetEntt(g_EnttMgr.GetPlayerId());
        const Entity*   pEnemy = g_EnttMgr.GetEntt(enemyId);

        const Vec2   playerPos = pPlayer->GetComponent<Transform>()->GetPos();
        const Vec2    enemyPos = pEnemy->GetComponent<Transform>()->GetPos();

        // is the player in visibility range?
        const float sqrDistToPlayer = Vec2SqrDist(Vec2Sub(playerPos, enemyPos));
        const bool   bPlayerInSight = (sqrDistToPlayer < SQR(Enemy::SIGHT_RADIUS));
        const bool      bPlayerDead = pPlayer->GetComponent<Health>()->IsDead();
        
        bSeePlayer = (bPlayerInSight && !bPlayerDead);
    }

    lua_pushboolean(L, bSeePlayer);
    return 1;   // number of results
}

//---------------------------------------------------------
// return a type of enemy by ID
//---------------------------------------------------------
static int GetType(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);

    assert(pEntt->HasComponent<Enemy>());
    const eEnemyType enemyType = pEntt->GetComponent<Enemy>()->GetType();

    lua_pushinteger(L, enemyType);
    return 1;   // number of results
}

//---------------------------------------------------------
// return a type of enemy by ID as a string (name of type)
//---------------------------------------------------------
static int GetTypeAsStr(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);

    assert(pEntt->HasComponent<Enemy>());
    const char* nameOfType = pEntt->GetComponent<Enemy>()->GetTypeAsStr();

    lua_pushstring(L, nameOfType);
    return 1;   // number of results
}

//---------------------------------------------------------
// return a direction of enemy by ID
//---------------------------------------------------------
static int GetDir(lua_State* L)
{
    assert(L);

    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);

    assert(pEntt->HasComponent<Enemy>());
    const Vec2 d = pEntt->GetComponent<Enemy>()->GetDir();

    lua_pushnumber(L, d.x);
    lua_pushnumber(L, d.y);
    return 2;   // number of results
}

//---------------------------------------------------------
// get idle time of enemy by ID
//---------------------------------------------------------
static int GetIdleTime(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);

    assert(pEntt->HasComponent<Enemy>());
    const float idleTime = pEntt->GetComponent<Enemy>()->GetIdleTime();
    lua_pushnumber(L, idleTime);
    return 1;   // number of results
}

//---------------------------------------------------------
// generate a random direction for enemy to move 
//---------------------------------------------------------
static int GetRandDirection(lua_State* L)
{
    assert(L);
    const Vec2 dir = Enemy::RandDir();
    lua_pushnumber(L, dir.x);
    lua_pushnumber(L, dir.y);
    return 2;   // number of results
}

//---------------------------------------------------------
// generate a random distance for enemy to move
//---------------------------------------------------------
static int GetRandDistance(lua_State* L)
{
    assert(L);
    lua_pushnumber(L, (double)(rand() & 511));
    return 1;   // number of results
}

//---------------------------------------------------------
// set a movement direction for enemy by ID 
// (if direction is <0,0> then enemy don't move)
//---------------------------------------------------------
static int SetMoveDir(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const float dirX = (float)luaL_checknumber(L, 2);
    const float dirY = (float)luaL_checknumber(L, 3);
    Entity* pEntt = g_EnttMgr.GetEntt(id);

    assert(pEntt->HasComponent<Enemy>());
    pEntt->GetComponent<Enemy>()->SetDir({ dirX, dirY });

    return 0;   // number of results;
}

//---------------------------------------------------------
// set that enemy by ID will move to the point in world
//---------------------------------------------------------
static int SetMoveTo(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const float    px = (float)luaL_checknumber(L, 2);
    const float    py = (float)luaL_checknumber(L, 3);
    Entity*     pEntt = g_EnttMgr.GetEntt(id);

    assert(pEntt->HasComponent<Enemy>());
    pEntt->GetComponent<Enemy>()->SetMoveTo({ px, py });

    return 0;   // number of results
}

//---------------------------------------------------------
// set idling time (input in ms) for enemy by ID
//---------------------------------------------------------
static int SetIdleForMs(lua_State* L)
{
    assert(L);
    const EntityID    id = (EntityID)luaL_checkinteger(L, 1);
    const int idleTimeMs = (int)luaL_checkinteger(L, 2);
    Entity*        pEntt = g_EnttMgr.GetEntt(id);

    assert(pEntt->HasComponent<Enemy>());
    pEntt->GetComponent<Enemy>()->SetIdlingForMs((float)idleTimeMs);

    return 0;   // number of results
}

//---------------------------------------------------------
// check if enemy by ID is already at its destination point (after movement)
//---------------------------------------------------------
static int IsInDestPoint(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);

    assert(pEntt->HasComponent<Enemy>());
    const bool bAtDstPoint = pEntt->GetComponent<Enemy>()->IsInDestPoint();

    lua_pushboolean(L, bAtDstPoint);
    return 1;   // number of results
}

//---------------------------------------------------------
// is the main player in the attack range of enemy by ID
//---------------------------------------------------------
static int IsAbleAttackPlayer(lua_State* L)
{
    assert(L);

    const EntityID       id = (EntityID)luaL_checkinteger(L, 1);
    const Entity*    pEnemy = g_EnttMgr.GetEntt(id);
    const Entity*   pPlayer = g_EnttMgr.GetPlayer();

    const Vec2    playerPos = pPlayer->GetComponent<Transform>()->GetPos();
    const Vec2     enemyPos = pEnemy->GetComponent<Transform>()->GetPos();
    const bool  bPlayerDead = pPlayer->GetComponent<Health>()->IsDead();

    const float sqrDistToPlayer = Vec2SqrDist(&playerPos, &enemyPos);
    const bool  bPlayerInAttack = (sqrDistToPlayer < SQR(Enemy::ATTACK_RADIUS));

    lua_pushboolean(L, (bPlayerInAttack && !bPlayerDead));
    return 1;   // number of results
}

//---------------------------------------------------------
// prevent enemy giving damage to player immediately after becoming able to attack
// (just wait a bit of time)
//---------------------------------------------------------
static int SetCooldownBeforeFirstAttack(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    Entity* pEntt = g_EnttMgr.GetEntt(id);

    assert(pEntt->HasComponent<Damage>());
    pEntt->GetComponent<Damage>()->SetCooldownBeforeFirstAttack();

    return 0;   // number of results
}

//---------------------------------------------------------
// when we kill an enemy we remove its enemy-specific components, 
// so it won't be considered as enemy anymore and we save computations
//---------------------------------------------------------
static int SetIsNotEnemyAnymore(lua_State* L)
{
    assert(L);

    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    Entity* pEntt = g_EnttMgr.GetEntt(id);

    const Enemy* pEnemy = pEntt->GetComponent<Enemy>();
    const char* typeName = pEnemy->GetTypeAsStr();

    // create a key to localized name of this entity
    char name[MAX_LEN_ENTT_NAME];
    snprintf(name, sizeof(name), "$%s", typeName);
    pEntt->AddComponent<Name>(name);

    // remove components which were neccessary for enemy
    pEntt->RemoveComponent<Enemy>();
    pEntt->RemoveComponent<Health>();
    pEntt->RemoveComponent<Damage>();

    return 0;   // number of results
}

//---------------------------------------------------------
// get direction vector (not normalized) from enemy by ID towards the main player
//---------------------------------------------------------
static int GetDirToPlayer(lua_State* L)
{
    assert(L);

    const EntityID     id = (EntityID)luaL_checkinteger(L, 1);
    const Entity*  pEnemy = g_EnttMgr.GetEntt(id);
    const Entity* pPlayer = g_EnttMgr.GetPlayer();

    const Vec2  playerPos = pPlayer->GetComponent<Transform>()->GetPos();
    const Vec2   enemyPos = pEnemy->GetComponent<Transform>()->GetPos();

    lua_pushnumber(L, playerPos.x - enemyPos.x);
    lua_pushnumber(L, playerPos.y - enemyPos.y);
    return 2;   // number of results
}

//---------------------------------------------------------
// get normalized direction vector from enemy by ID towards the main player
//---------------------------------------------------------
static int GetNormDirToPlayer(lua_State* L)
{
    assert(L);

    const EntityID     id = (EntityID)luaL_checkinteger(L, 1);
    const Entity*  pEnemy = g_EnttMgr.GetEntt(id);
    const Entity* pPlayer = g_EnttMgr.GetPlayer();

    const Vec2  playerPos = pPlayer->GetComponent<Transform>()->GetPos();
    const Vec2  enemyPos  = pEnemy->GetComponent<Transform>()->GetPos();

    Vec2 to = Vec2Sub(playerPos, enemyPos);
    Vec2Normalize(to);

    lua_pushnumber(L, to.x);
    lua_pushnumber(L, to.y);
    return 2;   // number of results
}

//---------------------------------------------------------
// the library functions
//---------------------------------------------------------
static const struct luaL_Reg s_LibFunx[] =
{
    {"SetState",                        SetState},
    {"SetIdleForMs",                    SetIdleForMs},
    {"SetMoveDir",                      SetMoveDir},
    {"SetMoveTo",                       SetMoveTo},
    {"SetCooldownBeforeFirstAttack",    SetCooldownBeforeFirstAttack},
    {"SetIsNotEnemyAnymore",            SetIsNotEnemyAnymore},

    {"IsInDestPoint",                   IsInDestPoint},
    {"IsAbleAttackPlayer",              IsAbleAttackPlayer},
    {"IsSeePlayer",                     IsSeePlayer},

    {"GetType",                         GetType},      
    {"GetTypeAsStr",                    GetTypeAsStr},      

    {"GetDir",                          GetDir},
    {"GetIdleTime",                     GetIdleTime},
    {"GetRandDirection",                GetRandDirection},
    {"GetRandDistance",                 GetRandDistance},
    {"GetDirToPlayer",                  GetDirToPlayer},
    {"GetNormDirToPlayer",              GetNormDirToPlayer},

    
    {NULL, NULL}
};

//---------------------------------------------------------
// create the lib
//---------------------------------------------------------
static int LibRequire(lua_State* L)
{
    assert(L);
    luaL_newlib(L, s_LibFunx);
    return 1;   // number of results
}

//---------------------------------------------------------
// register the lib for the input Lua-state
//---------------------------------------------------------
void LuaLoadEnemyLib(lua_State* L)
{
    luaL_requiref(L, s_LibName, LibRequire, 1);
    lua_pop(L, 1);  // pop the library from the stack
}