/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: lua_lib_sound.cpp
    Desc:     a library for using in Lua to work with sounds
    Created:  25.07.2026 by DimaSkup
\**********************************************************************************/
#include "lua_lib_sound.h"
#include "asset_mgr.h"
#include "log.h"
#include <assert.h>


// the library name
static const char* s_LibName = "sound";

//---------------------------------------------------------
// play a sound by name
//---------------------------------------------------------
static int SoundPlay(lua_State* L)
{
    assert(L);

    // play a sound by name
    if (lua_isstring(L, 1))
    {
        const char* soundName = lua_tostring(L, 1);
        g_AssetMgr.PlaySound(soundName);
    }
    // play a sound by ID
    else if (lua_isinteger(L, 1))
    {
        const SoundID id = (SoundID)lua_tointeger(L, 1);
        g_AssetMgr.PlaySound(id);
    }
    else
    {
        LogErr(LOG, "invalid type on LUA stack");
    }

    return 0;   // number of results
}

//---------------------------------------------------------
// stop a sound by name/id
//---------------------------------------------------------
static int SoundStop(lua_State* L)
{
    assert(L);

    // stop a sound by name
    if (lua_isstring(L, 1))
    {
        const char* soundName = lua_tostring(L, 1);
        g_AssetMgr.StopSound(soundName);
    }
    // stop a sound by ID
    else if (lua_isinteger(L, 1))
    {
        const SoundID id = (SoundID)lua_tointeger(L, 1);
        g_AssetMgr.StopSound(id);
    }
    else
    {
        LogErr(LOG, "invalid type on LUA stack");
    }
    
    return 0;   // number of results
}

//---------------------------------------------------------
// setup looping of a sound by name/id
//---------------------------------------------------------
static int SetLooping(lua_State* L)
{
    assert(L);

    // by name
    if (lua_isstring(L, 1))
    {
        const char* soundName = lua_tostring(L, 1);
        const bool   bLooping = (bool)lua_toboolean(L, 2);
        const SoundID soundId = g_AssetMgr.GetSoundId(soundName);
        g_AssetMgr.SetSoundLooping(soundId, bLooping);
    }
    // by id
    else if (lua_isinteger(L, 1))
    {
        const SoundID    id = (SoundID)lua_tointeger(L, 1);
        const bool bLooping = (bool)lua_toboolean(L, 2);
        g_AssetMgr.SetSoundLooping(id, bLooping);
    }
    else
    {
        LogErr(LOG, "invalid type on LUA stack");
    }

    return 0;   // number of results
}

//---------------------------------------------------------
// the library functions
//---------------------------------------------------------
static const struct luaL_Reg s_LibFunx[] =
{
    {"Play",    SoundPlay},
    {"Stop",    SoundStop},
    {"Looping", SetLooping},
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
void LuaLoadSoundLib(lua_State* L)
{
    luaL_requiref(L, s_LibName, LibRequire, 1);
    lua_pop(L, 1);  // pop the library from the stack
}