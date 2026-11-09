/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: lua_lib_game.cpp
    Desc:     a library for using in Lua to work with game's common stuff
              (like game states, camera, etc.)
    Created:  08.08.2026 by DimaSkup
\**********************************************************************************/
#include "lua_lib_game.h"
#include "game_state.h"
#include "game.h"
#include <assert.h>
#include <stdlib.h>


// the library name
static const char* s_LibName = "game";

inline Game* GetGame()
{
    assert(g_GameState.pGame && "a ptr to the game instance == NULL");
    return (Game*)g_GameState.pGame;
}

//---------------------------------------------------------
// return a name of the current game level
//---------------------------------------------------------
static int GetCurrLevelName(lua_State* L)
{
    assert(L);

    const char* levelName = GetGame()->GetCurrLevelName();
    assert(levelName && levelName[0] != '\0');

    lua_pushstring(L, levelName);
    return 1;   // number of results
}

//---------------------------------------------------------
// setup the camera's shake intensity (in range [0,1])
//---------------------------------------------------------
static int SetCameraShake(lua_State* L)
{
    assert(L);
    g_GameState.camShake = (float)luaL_checknumber(L, 1);
    return 0;   // number of results
}

//---------------------------------------------------------
// get a random position within visible part of the scene/world
//---------------------------------------------------------
static int GetRandPosInVisWorld(lua_State* L)
{
    assert(L);

    int px = rand() % g_GameState.camSize.w + g_GameState.camPos.x;
    int py = rand() % g_GameState.camSize.h + g_GameState.camPos.y;

    lua_pushinteger(L, px);
    lua_pushinteger(L, py);
    return 2;   // number of results
}

//---------------------------------------------------------
// at the beginning of the next frame we will load another game level by name
//---------------------------------------------------------
static int WantLoadLevel(lua_State* L)
{
    assert(L);
    const char* levelName = luaL_checkstring(L, 1);
    GetGame()->WantLoadLevel(levelName);
    return 0;   // number of results
}

//---------------------------------------------------------
// convert a world position into screen coords
//---------------------------------------------------------
static int WorldToScreen(lua_State* L)
{
    assert(L);
    const float px = (float)luaL_checknumber(L, 1);
    const float py = (float)luaL_checknumber(L, 2);
    const Vec2i p = GetGame()->WorldToScreen(Vec2(px, py));

    lua_pushinteger(L, p.x);
    lua_pushinteger(L, p.y);
    return 2;   // number of results
}

//---------------------------------------------------------
// return an identifier of entity related to game event
// (for instance: entity has activated a trigger so we will heal/damage this entity)
//---------------------------------------------------------
static int GetEventDataEnttId(lua_State* L)
{
    assert(L);
    const EventData* pData = GetGame()->GetEventMgr()->GetEventData();
    assert(pData);
    lua_pushinteger(L, pData->enttId);
    return 1;   // number of results
}

//---------------------------------------------------------
// when trigger runs some event we want to know an ID of this trigger (entity)
//---------------------------------------------------------
static int GetEventDataTriggeredBy(lua_State* L)
{
    assert(L);
    const EventData* pData = GetGame()->GetEventMgr()->GetEventData();
    assert(pData);
    lua_pushinteger(L, pData->triggeredBy);
    return 1;   // number of results
}

//---------------------------------------------------------
// return 2 floats (vec2) related to game event
// (for instance: entity is moving so we want to know its direction)
//---------------------------------------------------------
static int GetEventDataVec2(lua_State* L)
{
    assert(L);
    const EventData* pData = GetGame()->GetEventMgr()->GetEventData();
    assert(pData);
    lua_pushnumber(L, pData->fx);
    lua_pushnumber(L, pData->fy);
    return 2;   // number of results
}

//---------------------------------------------------------

static int GetGameTime(lua_State* L)
{
    assert(L);
    lua_pushnumber(L, g_GameState.gameTime);
    return 1;   // number of results
}

//---------------------------------------------------------

static int GetDeltaTime(lua_State* L)
{
    assert(L);
    lua_pushnumber(L, g_GameState.deltaTime);
    return 1;   // number of results
}

//---------------------------------------------------------
// the library functions
//---------------------------------------------------------
static const struct luaL_Reg s_LibFunx[] =
{
    {"GetCurrLevelName",     GetCurrLevelName},

    {"SetCameraShake",       SetCameraShake},
    {"GetGameTime",          GetGameTime},
    {"GetDeltaTime",         GetDeltaTime},
    {"GetRandPosInVisWorld", GetRandPosInVisWorld},
    {"WantLoadLevel",        WantLoadLevel},       
    {"WorldToScreen",        WorldToScreen},

    // get event related data
    {"GetEventDataEnttId",      GetEventDataEnttId},
    {"GetEventDataTriggeredBy", GetEventDataTriggeredBy},
    {"GetEventDataVec2",        GetEventDataVec2},

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
void LuaLoadGameLib(lua_State* L)
{
    assert(L);
    luaL_requiref(L, s_LibName, LibRequire, 1);
    lua_pop(L, 1);  // pop the library from the stack
}