/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: lua_lib_sys.h
    Desc:     a library for using in Lua to work with system/common functional
              (like logger)
    Created:  25.08.2026 by DimaSkup
\**********************************************************************************/
#ifndef LUA_LIB_SYS_H
#define LUA_LIB_SYS_H

#include "lua_stuff.h"

// loads the system library
void LuaLoadSysLib(lua_State* L);

#endif