/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: lua_lib_game.h
    Desc:     a library for using in Lua to work with game's common stuff 
              (like game states, camera, etc.)
    Created:  08.08.2026 by DimaSkup
\**********************************************************************************/
#ifndef LUA_LIB_GAME_H
#define LUA_LIB_GAME_H

#include "lua_stuff.h"

// load the game library
void LuaLoadGameLib(lua_State* L);

#endif 
