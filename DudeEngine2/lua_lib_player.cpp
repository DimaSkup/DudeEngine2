/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: lua_lib_player.cpp
    Desc:     a library for using in Lua to work properties specific to the player
    Created:  26.07.2026 by DimaSkup
\**********************************************************************************/
#include "lua_lib_player.h"
#include "entity_mgr.h"
#include "game_state.h"
#include "asset_mgr.h"
#include "input.h"
#include "components/transform.h"
#include "components/player.h"
#include <assert.h>


// the library name
static const char* s_LibName = "player";

//---------------------------------------------------------
// get an entity ID of the main player
//---------------------------------------------------------
static int GetId(lua_State* L)
{
    assert(L);
    lua_pushinteger(L, g_EnttMgr.GetPlayerId());
    return 1;   // number of results
}

//---------------------------------------------------------
// get a position of the main player in the world
//---------------------------------------------------------
static int GetPos(lua_State* L)
{
    assert(L);
    const Entity* player = g_EnttMgr.GetPlayer();
    const Vec2 p = player->GetComponent<Transform>()->GetPos();

    lua_pushnumber(L, p.x);
    lua_pushnumber(L, p.y);
    return 2;   // number of results
}

//---------------------------------------------------------
// get a distance between the player and input point in the world
//---------------------------------------------------------
static int GetDistTo(lua_State* L)
{
    assert(L);

    const float px = (float)luaL_checkinteger(L, 1);
    const float py = (float)luaL_checkinteger(L, 2);

    const Transform* t = g_EnttMgr.GetPlayer()->GetComponent<Transform>();
    const Vec2       c = t->GetCenter();
    const float   dist = sqrtf(SQR(px - c.x) + SQR(py - c.y));

    lua_pushnumber(L, dist);
    return 1;   // number of results
}

//---------------------------------------------------------
// is the main player currently immortal?
//---------------------------------------------------------
static int IsImmortal(lua_State* L)
{
    assert(L);
    lua_pushboolean(L, g_GameState.bPlayerImmortal);
    return 1;   // number of results
}

//---------------------------------------------------------
// is the main player currently attacking (swaying its sword or smth like that)
//---------------------------------------------------------
static int IsAttacking(lua_State* L)
{
    assert(L);
    const Mouse& m = GetMouseState();
    lua_pushboolean(L, m.button[MOUSE_L].bDown && g_GameState.bGameMode);
    return 1;   // number of results
}

//---------------------------------------------------------
// is the main player currently moving ?
//---------------------------------------------------------
static int IsMoving(lua_State* L)
{
    assert(L);
    const Entity* pPlayer = g_EnttMgr.GetPlayer();
    assert(pPlayer->HasComponent<Player>());

    lua_pushboolean(L, pPlayer->GetComponent<Player>()->IsMoving());
    return 1;   // number of results
}

//---------------------------------------------------------
// set a weapon by input name as a current for the main player
//---------------------------------------------------------
static int SwitchWpn(lua_State* L)
{
    assert(L);

    const char*  wpnName = luaL_checkstring(L, 1);
    const WeaponID wpnId = g_AssetMgr.GetWpnIdByName(wpnName);
    Entity*      pPlayer = g_EnttMgr.GetPlayer();

    pPlayer->GetComponent<Player>()->SetCurrWeapon(wpnId);
    return 0;   // number of results
}

//---------------------------------------------------------
// return a name of the player's current weapon
//---------------------------------------------------------
static int GetCurrWpnName(lua_State* L)
{
    assert(L);
    const Entity* player = g_EnttMgr.GetPlayer();
    const WeaponID id = player->GetComponent<Player>()->GetCurrWeaponId();
    const char* name = g_AssetMgr.GetWpnNameById(id);

    lua_pushstring(L, name);
    return 1;   // number of results
}

//---------------------------------------------------------
// return a player's basic speed
//---------------------------------------------------------
static int GetMoveBasicSpeed(lua_State* L)
{
    assert(L);
    const Entity* player = g_EnttMgr.GetPlayer();
    const float speed = player->GetComponent<Player>()->GetMoveBasicSpeed();

    lua_pushnumber(L, speed);
    return 1;   // number of results
}

//---------------------------------------------------------
// setup player's movement speed
//---------------------------------------------------------
static int SetMoveSpeed(lua_State* L)
{
    assert(L);
    const float speed = (float)luaL_checknumber(L, 1);
    Entity* player = g_EnttMgr.GetPlayer();
    player->GetComponent<Player>()->SetMoveSpeed(speed);
    return 0;   // number of results
}

//---------------------------------------------------------
// setup player's movement direction
//---------------------------------------------------------
static int SetMoveDir(lua_State* L)
{
    assert(L);
    const float dirX = (float)luaL_checknumber(L, 1);
    const float dirY = (float)luaL_checknumber(L, 2);

    Entity* player = g_EnttMgr.GetPlayer();
    player->GetComponent<Player>()->SetMoveDir({ dirX, dirY });
    return 0;   // number of results
}

//---------------------------------------------------------
// the library functions
//---------------------------------------------------------
static const struct luaL_Reg s_LibFunx[] =
{
    { "GetId",              GetId },
    { "GetPos",             GetPos },
    { "GetDistTo",          GetDistTo },
    { "IsImmortal",         IsImmortal },
    { "IsMoving",           IsMoving },
    { "IsAttacking",        IsAttacking },
    { "SwitchWpn",          SwitchWpn },
    { "GetCurrWpnName",     GetCurrWpnName },
    { "GetMoveBasicSpeed",  GetMoveBasicSpeed },
    { "SetMoveSpeed",       SetMoveSpeed },
    { "SetMoveDir",         SetMoveDir },   
    
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
void LuaLoadPlayerLib(lua_State* L)
{
    luaL_requiref(L, s_LibName, LibRequire, 1);
    lua_pop(L, 1);  // pop the library from the stack
}