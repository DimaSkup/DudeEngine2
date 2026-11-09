/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: lua_lib_sys.cpp
    Desc:     a library for using in Lua to work with system/common functional
              (like logger)
    Created:  25.08.2026 by DimaSkup
\**********************************************************************************/
#include "lua_lib_sys.h"
#include "log.h"
#include "game_state.h"
#include <assert.h>


// the library name
static const char* s_LibName = "sys";

//---------------------------------------------------------
// print a usual log-message into a console and log-file
//---------------------------------------------------------
static int LuaLogMsg(lua_State* L)
{
    assert(L);
    const char* msg = luaL_checkstring(L, 1);
    SetConsoleColor(LOG_GREEN);
    LogMsg("[LUA]: %s", msg);
    SetConsoleColor(LOG_RESET);
    return 0;   // number of results
}

//---------------------------------------------------------
// debug log-message
//---------------------------------------------------------
static int LuaLogDbg(lua_State* L)
{
    assert(L);
    const char* msg = luaL_checkstring(L, 1);
    SetConsoleColor(LOG_RESET);
    LogMsg("[LUA]: %s", msg);
    return 0;   // number of results
}

//---------------------------------------------------------
// error log-message
//---------------------------------------------------------
static int LuaLogErr(lua_State* L)
{
    assert(L);
    const char* msg = luaL_checkstring(L, 1);
    SetConsoleColor(LOG_RED);
    LogMsg("[LUA] (ERROR): %s", msg);
    return 0;   // number of results
}

//---------------------------------------------------------
// get percentage of input values relatively to the window size
//---------------------------------------------------------
static int GetWndPercent(lua_State* L)
{
    assert(L);

    const float x = (float)luaL_checknumber(L, 1);
    const float y = (float)luaL_checknumber(L, 2);

    const float percentX = x / (float)g_GameState.wndSize.w * 100.0f;
    const float percentY = y / (float)g_GameState.wndSize.h * 100.0f;

    lua_pushnumber(L, percentX);
    lua_pushnumber(L, percentY);
    return 2;   // number of results
}

//---------------------------------------------------------
// the library functions
//---------------------------------------------------------
static const struct luaL_Reg s_LibFunx[] =
{
    {"Log",             LuaLogErr},     // sys.Log() is by default for error messages
    {"LogMsg",          LuaLogMsg},     // for info messages
    {"LogDbg",          LuaLogDbg},     // for debug messages
    {"GetWndPercent",   GetWndPercent},
    {NULL, NULL},
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
// register the lib for input Lua-state
//---------------------------------------------------------
void LuaLoadSysLib(lua_State* L)
{
    assert(L);
    luaL_requiref(L, s_LibName, LibRequire, 1);
    lua_pop(L, 1);  // pop the library from the stack
}