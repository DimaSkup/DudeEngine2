/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: lua_lib_input.cpp
    Desc:     a library for using in Lua to work with input stuff (mouse, keyboard, etc.)
    Created:  25.07.2026 by DimaSkup
\**********************************************************************************/
#include "lua_lib_input.h"
#include "game_state.h"
#include "input.h"
#include <assert.h>


// the library name
static const char* s_LibName = "input";

//---------------------------------------------------------
// return a mouse cursor position on the screen
//---------------------------------------------------------
static int GetMousePos(lua_State* L)
{
    assert(L);
    lua_pushinteger(L, g_GameState.mousePos.x);
    lua_pushinteger(L, g_GameState.mousePos.y);
    return 2;   // number of results
}

//---------------------------------------------------------
// return a mouse cursor position in the game world
//---------------------------------------------------------
static int GetMouseWorldPos(lua_State* L)
{
    assert(L);
    lua_pushinteger(L, g_GameState.mousePosW.x);
    lua_pushinteger(L, g_GameState.mousePosW.y);
    return 2;   // number of results
}

//---------------------------------------------------------
// check if key by input name is currently pressed
//---------------------------------------------------------
static int IsKeyPressed(lua_State* L)
{
    assert(L);

    const char* keyName = luaL_checkstring(L, 1);
    const eKeyCode c = GetKeycodeByName(keyName);
    
    lua_pushboolean(L, IsKeyPressed(c));
    return 1;   // number of results
}

//---------------------------------------------------------
// the library functions
//---------------------------------------------------------
static const struct luaL_Reg s_LibFunx[] =
{
    {"GetMousePos",      GetMousePos},
    {"GetMouseWorldPos", GetMouseWorldPos},
    {"IsKeyPressed",     IsKeyPressed},
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
void LuaLoadInputLib(lua_State* L)
{
    assert(L);
    luaL_requiref(L, s_LibName, LibRequire, 1);
    lua_pop(L, 1);  // pop the library from the stack
}