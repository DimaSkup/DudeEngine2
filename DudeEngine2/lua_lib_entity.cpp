/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: lua_lib_entity.cpp
    Desc:     a library for using in Lua to work with entities
    Created:  25.07.2026 by DimaSkup
\**********************************************************************************/
#include "lua_lib_entity.h"
#include "types.h"
#include "entity_mgr.h"
#include "game_state.h"
#include "game.h"
#include "asset_mgr.h"

// component (of Entity-Component)
#include "components/transform.h"
#include "components/sprite.h"
#include "components/enemy.h"
#include "components/health.h"
#include "components/damage.h"
#include "components/player.h"
#include "components/life_timer.h"
#include "components/trigger.h"
#include "components/name.h"

#include <assert.h>


// the library name
static const char* s_LibName = "entity";


//---------------------------------------------------------
// return a ptr to the game instance
//---------------------------------------------------------
inline Game* GetGame()
{
    assert(g_GameState.pGame && "a ptr to the game instance == NULL");
    return (Game*)g_GameState.pGame;
}

//---------------------------------------------------------
// push Vec2 (two separate floats) onto Lua stack
//---------------------------------------------------------
inline void LuaPushVec2(lua_State* L, const Vec2& v)
{
    lua_pushnumber(L, v.x);
    lua_pushnumber(L, v.y);
}

//---------------------------------------------------------
// push Vec2i (two separate integers) onto Lua stack
//---------------------------------------------------------
inline void LuaPushVec2i(lua_State* L, const Vec2i& v)
{
    lua_pushinteger(L, v.x);
    lua_pushinteger(L, v.y);
}

//---------------------------------------------------------
// get an ID of entity by name
//---------------------------------------------------------
static int GetId(lua_State* L)
{
    assert(L);
    const char* name = luaL_checkstring(L, 1);
    const EntityID id = g_EnttMgr.GetEntt(name)->GetId();

    lua_pushinteger(L, id);
    return 1;   // number of results
}

//---------------------------------------------------------
// get a name of entity by ID
//---------------------------------------------------------
static int GetName(lua_State* L)
{
    assert(L);
    const EntityID   id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);
    const Name*   pName = pEntt->GetComponent<Name>();

    // no name is assigned to this entity
    if (!pName)
        lua_pushstring(L, "$invalid_name");
    else
        lua_pushstring(L, pName->GetName());

    return 1;   // number of results
}

//---------------------------------------------------------
// Desc:  spawn a new entity based on prefab (by name)
// Ret:   identifier of this entity
//---------------------------------------------------------
static int Spawn(lua_State* L)
{
    assert(L);
    const char* prefabName = luaL_checkstring(L, 1);

#if DEBUG || _DEBUG
    if (!g_EnttMgr.HasPrefab(prefabName))
    {
        LogErr(LOG, "can't spawn an entity (no prefab by name: %s)", prefabName);
        return 0;   // number of results
    }
#endif

    const Entity* pPrefab = g_EnttMgr.GetPrefab(prefabName);
    Entity*         pEntt = g_EnttMgr.Instantiate(pPrefab);

    lua_pushinteger(L, pEntt->GetId());
    return 1;   // number of results
}

//---------------------------------------------------------
// set entity by ID to remove (will be removed at the beginning of the next frame)
//---------------------------------------------------------
static int SetToRemove(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    g_EnttMgr.SetToRemove(id);
    return 0;   // number of results
}

//---------------------------------------------------------
// check if entity by ID is already dead
//---------------------------------------------------------
static int IsDead(lua_State* L)
{
    assert(L);
    const EntityID enttId = (EntityID)luaL_checkinteger(L, 1);
    const Entity*   pEntt = g_EnttMgr.GetEntt(enttId);

    assert(pEntt->HasComponent<Health>());
    const Health* pHealth = pEntt->GetComponent<Health>();

    lua_pushboolean(L, pHealth->IsDead());
    return 1;   // number of results
}

//---------------------------------------------------------
// Desc:  check if entity by ID is enemy (has the Enemy component)
// Ret:   true if enemy
//---------------------------------------------------------
static int IsEnemy(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);

    lua_pushboolean(L, pEntt->HasComponent<Enemy>());
    return 1;   // number of results
}

//---------------------------------------------------------
// Desc:  check if entity by ID is the main player
// Ret:   true if player
//---------------------------------------------------------
static int IsPlayer(lua_State* L)
{
    assert(L);
    const EntityID       id = (EntityID)luaL_checkinteger(L, 1);
    const EntityID playerId = g_EnttMgr.GetPlayer()->GetId();

    lua_pushboolean(L, (id == playerId));
    return 1;   // number of results
}

//---------------------------------------------------------
// check if there is an entity by ID
//---------------------------------------------------------
static int HasEntt(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    lua_pushboolean(L, g_EnttMgr.HasEntt(id));
    return 1;   // number of results
}

//---------------------------------------------------------
// set a position (top-left corner) of entity by ID
//---------------------------------------------------------
static int SetPos(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const float px = (float)luaL_checknumber(L, 2);
    const float py = (float)luaL_checknumber(L, 3);
    Entity*     pEntt = g_EnttMgr.GetEntt(id);

    pEntt->GetComponent<Transform>()->SetPos(Vec2(px, py));
    return 0;   // number of results
}


//---------------------------------------------------------
// set a state of entity by ID (idle, attack, dead, etc.)
//---------------------------------------------------------
static int SetState(lua_State* L)
{
    assert(L);
    const EntityID      id = (EntityID)luaL_checkinteger(L, 1);
    const eEnttState state = (eEnttState)luaL_checkinteger(L, 2);

    assert(state >= 0 && state < NUM_ENTT_STATES);
    g_EnttMgr.GetEntt(id)->SetState(state);
    return 0;   // number of results
}

//---------------------------------------------------------
// play animation of entity by ID
//---------------------------------------------------------
static int PlayAnimation(lua_State* L)
{
    assert(L);
    const EntityID    enttId = (EntityID)luaL_checkinteger(L, 1);
    const eAnimType animType = (eAnimType)luaL_checkinteger(L, 2);
    Entity*            pEntt = g_EnttMgr.GetEntt(enttId);

    Sprite* pSpr = pEntt->GetComponent<Sprite>();
    assert(pSpr);
    assert(animType >= 0 && animType < NUM_ANIM_TYPES);

    pSpr->PlayAnimation(animType);
    return 0;   // number of results
}

//---------------------------------------------------------
// get a position of entity by ID
//---------------------------------------------------------
static int GetPos(lua_State* L)
{
    assert(L);
    const EntityID   id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);
    const Vec2        p = pEntt->GetComponent<Transform>()->GetPos();
    
    LuaPushVec2(L, p);
    return 2;   // number of results
}

//---------------------------------------------------------
// get a size of entity in local space (without scaling)
//---------------------------------------------------------
static int GetLocalSize(lua_State* L)
{
    assert(L);
    const EntityID   id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);
    const Vec2i      sz = pEntt->GetComponent<Transform>()->GetLocalSize();

    LuaPushVec2i(L, sz);
    return 2;   // number of results
}

//---------------------------------------------------------
// get a size of entity in world space (with applied scaling)
//---------------------------------------------------------
static int GetWorldSize(lua_State* L)
{
    assert(L);
    const EntityID   id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);
    const Vec2i      sz = pEntt->GetComponent<Transform>()->GetWorldSize();

    LuaPushVec2i(L, sz);
    return 2;   // number of results
}
//---------------------------------------------------------
// get a center of entity by ID
//---------------------------------------------------------
static int GetCenter(lua_State* L)
{
    assert(L);
    const EntityID   id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);
    const Transform*  t = pEntt->GetComponent<Transform>();
    const Vec2        c = t->GetCenter();

    lua_pushnumber(L, c.x);
    lua_pushnumber(L, c.y);
    return 2;   // number of results
}

//---------------------------------------------------------
// get current attack cooldown time for entity by ID
//---------------------------------------------------------
static int GetAttackCooldownSec(lua_State* L)
{
    assert(L);
    const EntityID     id = (EntityID)luaL_checkinteger(L, 1);
    const Entity*   pEntt = g_EnttMgr.GetEntt(id);
    const Damage* pDamage = pEntt->GetComponent<Damage>();

    assert(pDamage);
    const float t = pDamage->GetCurrCooldownSec();

    lua_pushnumber(L, t);
    return 1;   // number of results
}

//---------------------------------------------------------
// get an entity ID of the main player
//---------------------------------------------------------
static int GetPlayerId(lua_State* L)
{
    assert(L);
    lua_pushinteger(L, g_EnttMgr.GetPlayerId());
    return 1;   // number of results
}

//---------------------------------------------------------
// 1. if entity by ID has a LifeTime component (it will be killed after some time)
//    then we return its full and remained life timings;
// 2. if it hasn't a LifeTime component then return {0.0f, 0.0f}
//---------------------------------------------------------
static int GetLifeTimes(lua_State* L)
{
    assert(L);

    const EntityID       id = (EntityID)luaL_checkinteger(L, 1);
    const Entity*     pEntt = g_EnttMgr.GetEntt(id);
    const LifeTimer* pTimer = pEntt->GetComponent<LifeTimer>();

    if (pTimer)
    {
        lua_pushnumber(L, pTimer->GetFullLifeTime());
        lua_pushnumber(L, pTimer->GetRemainLifeTime());
    }
    else
    {
        lua_pushnumber(L, 0.0);
        lua_pushnumber(L, 0.0);
    }

    return 2;   // number of results
}

//---------------------------------------------------------
// get health points (HP) of entity by ID 
// (if entity doesn't have a Health component then return 0)
//---------------------------------------------------------
static int GetHP(lua_State* L)
{
    assert(L);
    const EntityID   id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);
    const Health*     h = pEntt->GetComponent<Health>();

    if (h)
        lua_pushinteger(L, h->GetHP());
    else
        lua_pushinteger(L, 0);

    return 1;   // number of results
}

//---------------------------------------------------------
// return RGBA color of entity's sprite (all are 0-255 values)
//---------------------------------------------------------
static int GetColor(lua_State* L) 
{ 
    assert(L);

    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);
    const Sprite* pSpr = pEntt->GetComponent<Sprite>();

    assert(pSpr);

    const Color c = pSpr->GetColor();
    lua_pushinteger(L, c.r);
    lua_pushinteger(L, c.g);
    lua_pushinteger(L, c.b);
    lua_pushinteger(L, c.a);
    return 4;   // number of results
}

//---------------------------------------------------------
// add a trigger of type TRIGGER_MULTIPLE to entity by ID
//---------------------------------------------------------
static int AddTriggerMultiple(lua_State* L)
{
    assert(L);

    // get data from LUA
    const EntityID id                 = (EntityID)luaL_checkinteger(L, 1);
    const char* eventOnEnter          = luaL_checkstring(L, 2);
    const char* eventOnCollide        = luaL_checkstring(L, 3);
    const char* eventOnLeave          = luaL_checkstring(L, 4);
    const bool  bTriggersOnlyByPlayer = (bool)lua_toboolean(L, 5);

    // setup initial params for a trigger
    TriggerInitParams params;
    memset(&params, 0, sizeof(params));

    const EventMgr* pEventMgr = GetGame()->GetEventMgr();

    params.onEnter               = pEventMgr->GetEventIdByName(eventOnEnter);
    params.onCollide             = pEventMgr->GetEventIdByName(eventOnCollide);
    params.onLeave               = pEventMgr->GetEventIdByName(eventOnLeave);
    params.type                  = TRIGGER_MULTIPLE;
    params.bTriggersOnlyByPlayer = bTriggersOnlyByPlayer;

    // add a trigger
    Entity* pEntt = g_EnttMgr.GetEntt(id);
    pEntt->AddComponent<Trigger>(params);

    return 0;   // number of results
}

//---------------------------------------------------------
// give damage to entity by ID
//---------------------------------------------------------
static int GiveDamage(lua_State* L)
{
    assert(L);
    const EntityID  id = (EntityID)luaL_checkinteger(L, 1);
    const float damage = (float)luaL_checknumber(L, 2);
    
    Entity* pEntt = g_EnttMgr.GetEntt(id);
    Health* pHealth = pEntt->GetComponent<Health>();

    assert(pHealth);
    pHealth->GiveDamage(damage);

    return 0;                                           // number of results
}

//---------------------------------------------------------
// heal an entity by ID (increase its health/HP)
//---------------------------------------------------------
static int HealBy(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const float healBy = (float)luaL_checknumber(L, 2);

    Entity* pEntt = g_EnttMgr.GetEntt(id);
    Health* pHealth = pEntt->GetComponent<Health>();

    assert(pHealth);
    pHealth->Heal(healBy);

    return 0;   // number of results
}

//---------------------------------------------------------
// update direction of entity animation 
// (rotate entity towards input direction, relatively to entity by itself)
//---------------------------------------------------------
static int UpdateAnimDir(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const float  dirX = (float)luaL_checknumber(L, 2);      // direction by X
    const float  dirY = (float)luaL_checknumber(L, 3);      // direction by Y

    // define direction of animation
    eAnimDir dir = ANIM_NODIR;

    if (fabsf(dirX) > fabsf(dirY))
        dir = (dirX < 0.0f) ? ANIM_LEFT : ANIM_RIGHT;
    else
        dir = (dirY < 0.0f) ? ANIM_UP : ANIM_DOWN;

    Entity* pEntt = g_EnttMgr.GetEntt(id);
    Sprite* pSpr = pEntt->GetComponent<Sprite>();
    assert(pSpr);
    pSpr->SetAnimDir(dir);

    return 0;   // number of results
}

//---------------------------------------------------------
// get attack damage of entity by ID
//---------------------------------------------------------
static int GetWpnDamage(lua_State* L)
{
    assert(L);
    const EntityID   id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);
    double damage = 0;

    // if is player
    if (g_EnttMgr.GetPlayerId() == id)
    {
        const Player* player = pEntt->GetComponent<Player>();
        assert(player);

        // for the player we get damage of its current weapon
        const WeaponID wpnId = player->GetCurrWeaponId();
        damage = (double)g_AssetMgr.GetWpnById(wpnId)->damage;
    }

    // enemy
    else
    {
        const Damage* pDamage = pEntt->GetComponent<Damage>();
        assert(pDamage);
        damage = (double)pDamage->GetDamage();
    }

    lua_pushnumber(L, damage);
    return 1;   // number of results
}

//---------------------------------------------------------
// get attack range of entity's weapon
//---------------------------------------------------------
static int GetWpnRadius(lua_State* L)
{
    assert(L);
    const EntityID   id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);
    double radius = 0;

    // if is player
    if (g_EnttMgr.GetPlayerId() == id)
    {
        const Player* player = pEntt->GetComponent<Player>();
        assert(player);

        // for the player we get attack radius of its current weapon
        const WeaponID wpnId = player->GetCurrWeaponId();
        radius = (double)g_AssetMgr.GetWpnById(wpnId)->attackRadius;
    }

    // enemy
    else
    {
        radius = Enemy::ATTACK_RADIUS;
    }

    lua_pushnumber(L, radius);
    return 1;   // number of results
}

//---------------------------------------------------------
// damage all the enemies within range (circular with center and radius)
//---------------------------------------------------------
static int DamageEnemiesInRange(lua_State* L)
{
    // gather closest enemies and damage them
    cvector<EntityID> enemiesToAttack;
    enemiesToAttack.reserve(16);

    Game* pGame = GetGame();
    const cvector<EntityID>& visEnemies = pGame->GetVisEnemies();

    const float damage          = (float)luaL_checknumber(L, 1);
    const float attackCX        = (float)luaL_checknumber(L, 2);
    const float attackCY        = (float)luaL_checknumber(L, 3);
    const float attackRadius    = (float)luaL_checkinteger(L, 4);

    const Vec2  attackCenter    = { attackCX, attackCY };
    const float attackRadiusSqr = SQR(attackRadius);


    // get only enemies in attack range
    for (index i = 0; i < visEnemies.size(); ++i)
    {
        const EntityID enttId = visEnemies[i];
        const Entity*   pEntt = g_EnttMgr.GetEntt(enttId);
        const Vec2   enemyPos = pEntt->GetComponent<Transform>()->GetCenter();
        const float   sqrDist = Vec2SqrDist(&enemyPos, &attackCenter);

        // if in attack range
        if (sqrDist < attackRadiusSqr)
            enemiesToAttack.push_back(enttId);
    }

    // no enemies to damage
    if (enemiesToAttack.empty())
        return 0;

    // damage enemies by player
    for (const EntityID id : enemiesToAttack)
    {
        Entity* pEntt = g_EnttMgr.GetEntt(id);
        Health* h = pEntt->GetComponent<Health>();

        assert(h);
        h->GiveDamage(damage);
    }

    return 0;   // number of results
}

//---------------------------------------------------------
// set attack cooldown to max value for entity by ID
// (it prevents attacking each frame)
//---------------------------------------------------------
static int SetAttackCooldownToMax(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    Entity* pEntt = g_EnttMgr.GetEntt(id);

    Damage* pDamage = pEntt->GetComponent<Damage>();
    assert(pDamage);
    pDamage->SetCooldownToMax();

    return 0;   // number of results
}

//---------------------------------------------------------
// return a type of entity's collider
//---------------------------------------------------------
static int GetColliderType(lua_State* L)
{
    assert(L);

    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);

    const Collider* pCollider = pEntt->GetComponent<Collider>();
    assert(pCollider);

    lua_pushinteger(L, pCollider->GetType());
    return 1;   // number of results
}

//---------------------------------------------------------
// setup ability of moving through the collider;
// if "is_solid" set to true then entities can't move through it
//---------------------------------------------------------
static int SetColliderIsSolid(lua_State* L)
{
    assert(L);

    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const bool bSolid = (bool)lua_toboolean(L, 2);

    Entity* pEntt = g_EnttMgr.GetEntt(id);
    Collider* pCollider = pEntt->GetComponent<Collider>();

    assert(pCollider);
    pCollider->SetSolid(bSolid);
    return 0;   // number of results
}

//---------------------------------------------------------
// return a radius of entity's collider;
// if collider's type isn't COLLIDER_CIRCLE just return 0
//---------------------------------------------------------
static int GetColliderRadius(lua_State* L)
{
    assert(L);

    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);

    const Collider* pCollider = pEntt->GetComponent<Collider>();
    assert(pCollider);

    int r = 0;

    if (pCollider->GetType() == COLLIDER_CIRCLE)
        r = pCollider->GetRadius();

    lua_pushinteger(L, r);
    return 1;   // number of results
}

//---------------------------------------------------------
// set a radius of entity's collider;
// if collider's type isn't COLLIDER_CIRCLE we do nothing
//---------------------------------------------------------
static int SetColliderRadius(lua_State* L)
{
    assert(L);

    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const int       r = (int)luaL_checkinteger(L, 2);

    Entity* pEntt = g_EnttMgr.GetEntt(id);
    Collider* pCollider = pEntt->GetComponent<Collider>();
    assert(pCollider);

    if (pCollider->GetType() == COLLIDER_CIRCLE)
        pCollider->SetRadius(r);

    return 0;   // number of results
}

//---------------------------------------------------------
// return a width and height of entity's collider
// if collider's type isn't COLLIDER_BOX just return <0,0>
//---------------------------------------------------------
static int GetColliderBoxSize(lua_State* L)
{
    assert(L);

    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const Entity* pEntt = g_EnttMgr.GetEntt(id);

    const Collider* pCollider = pEntt->GetComponent<Collider>();
    assert(pCollider);

    int w = 0;
    int h = 0;

    if (pCollider->GetType() == COLLIDER_CIRCLE)
    {
        const Vec2i sz = pCollider->GetBoxSize();
        w = sz.w;
        h = sz.h;
    }

    lua_pushinteger(L, w);
    lua_pushinteger(L, h);
    return 2;   // number of results
}

//---------------------------------------------------------
// set a width and height of entity's collider;
// if collider's type isn't COLLIDER_BOX we do nothing
//---------------------------------------------------------
static int SetColliderBoxSize(lua_State* L)
{
    assert(L);

    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const int       w = (int)luaL_checkinteger(L, 2);
    const int       h = (int)luaL_checkinteger(L, 3);

    Entity* pEntt = g_EnttMgr.GetEntt(id);
    Collider* pCollider = pEntt->GetComponent<Collider>();
    assert(pCollider);

    if (pCollider->GetType() == COLLIDER_BOX)
        pCollider->SetBoxSize(w, h);

    return 0;   // number of results
}

//---------------------------------------------------------
// if entity by ID has a sprite then we will setup its color
//---------------------------------------------------------
static int SetColor(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const uint8 r = (uint8)luaL_checkinteger(L, 2);
    const uint8 g = (uint8)luaL_checkinteger(L, 3);
    const uint8 b = (uint8)luaL_checkinteger(L, 4);
    const uint8 a = (uint8)luaL_checkinteger(L, 5);

    Entity* pEntt = g_EnttMgr.GetEntt(id);
    Sprite* pSpr = pEntt->GetComponent<Sprite>();

    assert(pSpr);
    pSpr->SetColor(r, g, b, a);

    return 0;   // number of results
}

//---------------------------------------------------------
// if entity by ID has a sprite then we will setup its translucency (alpha)
//---------------------------------------------------------
static int SetAlpha(lua_State* L)
{
    assert(L);
    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const uint8 alpha = (uint8)luaL_checknumber(L, 2);

    Entity* pEntt = g_EnttMgr.GetEntt(id);
    Sprite* pSpr = pEntt->GetComponent<Sprite>();

    assert(pSpr);
    pSpr->SetAlpha(alpha);

    return 0;   // number of results
}

//---------------------------------------------------------
// Desc:  update a direction of entity's animation (4 directions: up, right, down, left)
//---------------------------------------------------------
static int UpdateAnimationDir(lua_State* L)
{
    assert(L);

    const EntityID id = (EntityID)luaL_checkinteger(L, 1);
    const float  dirX = (float)luaL_checknumber(L, 2);
    const float  dirY = (float)luaL_checknumber(L, 3);

    Entity* pEntt = g_EnttMgr.GetEntt(id);
    Sprite* pSpr = pEntt->GetComponent<Sprite>();
    assert(pSpr);

    // define an animation type
    eAnimDir dir = ANIM_NODIR;

    if (fabsf(dirX) > fabsf(dirY))
        dir = (dirX < 0.0f) ? ANIM_LEFT : ANIM_RIGHT;
    else
        dir = (dirY < 0.0f) ? ANIM_UP : ANIM_DOWN;

    pSpr->SetAnimDir(dir);

    return 0;   // number of results
}

//---------------------------------------------------------
// the library functions
//---------------------------------------------------------
static const struct luaL_Reg s_LibFunx[] =
{
    {"GetId",                   GetId},
    {"GetName",                 GetName},        
    {"GetPos",                  GetPos},
    {"GetLocalSize",            GetLocalSize},
    {"GetWorldSize",            GetWorldSize},
    {"GetCenter",               GetCenter},
    {"GetWpnDamage",            GetWpnDamage},
    {"GetWpnRadius",            GetWpnRadius},
    {"GetAttackCooldownSec",    GetAttackCooldownSec},
    {"GetPlayerId",             GetPlayerId},
    {"GetLifeTimes",            GetLifeTimes},
    {"GetHP",                   GetHP},
    {"GetColor",                GetColor},

    {"IsDead",                  IsDead},     
    {"IsEnemy",                 IsEnemy},
    {"IsPlayer",                IsPlayer},
    {"HasEntt",                 HasEntt},

    {"SetPos",                  SetPos},
    {"SetState",                SetState},  
    {"SetColor",                SetColor},
    {"SetAlpha",                SetAlpha},
    {"SetAttackCooldownToMax",  SetAttackCooldownToMax},
    {"UpdateAnimDir",           UpdateAnimDir},
    {"PlayAnimation",           PlayAnimation},

    // collider
    {"GetColliderType",         GetColliderType},
    {"GetColliderRadius",       GetColliderRadius},
    {"SetColliderRadius",       SetColliderRadius},
    {"GetColliderBoxSize",      GetColliderBoxSize},
    {"SetColliderBoxSize",      SetColliderBoxSize},
    {"SetColliderIsSolid",      SetColliderIsSolid},

    {"AddTriggerMultiple",      AddTriggerMultiple},
    {"GiveDamage",              GiveDamage},
    {"HealBy",                  HealBy},
    {"DamageEnemiesInRange",    DamageEnemiesInRange},
    {"Spawn",                   Spawn},
    {"SetToRemove",             SetToRemove},
    {"UpdateAnimationDir",      UpdateAnimationDir},
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
void LuaLoadEntityLib(lua_State* L)
{
    assert(L);
    luaL_requiref(L, s_LibName, LibRequire, 1);
    lua_pop(L, 1);  // pop the library from the stack
}