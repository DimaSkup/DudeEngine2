/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: lua_stuff.h
    Desc:     functional for work with Lua
    Created:  18.07.2026 by DimaSkup
\**********************************************************************************/
#ifndef LUA_STUFF_H
#define LUA_STUFF_H


//---------------------------------------------------------
// include Lua headers and link its libs
//---------------------------------------------------------
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#ifdef _WIN32
#pragma comment (lib, "include/lua/liblua55.a")
#endif

//---------------------------------------------------------
// forward declaration (pointer use only)
//---------------------------------------------------------
struct EnumValue;

//---------------------------------------------------------
// FUNCTIONS
//---------------------------------------------------------
void LuaErr(lua_State* L, const char* fmt, ...);
void LuaRunInterpreter(lua_State* L);

void LuaLoadScript(lua_State* L, const char* path);
void LuaCall(lua_State* L, const char* func);
void LuaCallVa(lua_State* L, const char* func, const char* sig, ...);
void LuaBind(lua_State* L, const char* funcName, lua_CFunction funcPtr);
void LuaRegEnumeration(lua_State* L, const EnumValue* enumArr, const int count);


#endif
