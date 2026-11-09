/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: lua_stuff.cpp
    Desc:     implementation of functional for work with Lua
    Created:  18.07.2026 by DimaSkup
\**********************************************************************************/
#include "lua_stuff.h"
#include "consts.h"
#include "types.h"
#include "log.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

//---------------------------------------------------------
// call it when we fucked up during working with Lua
//---------------------------------------------------------
void LuaErr(lua_State* L, const char* fmt, ...)
{
    assert(L);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    lua_close(L);
    exit(EXIT_FAILURE);
}

//---------------------------------------------------------
// run a primitive Lua interpreter in the console/terminal
//---------------------------------------------------------
void LuaRunInterpreter(lua_State* L)
{
    assert(L);

    char buf[256];
    int err = 0;

    while (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        err = luaL_loadstring(L, buf) || lua_pcall(L, 0, 0, 0);
        if (err)
        {
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
            lua_pop(L, 1);          // pop error msg from the stack
        }
    }
}

//---------------------------------------------------------
// load and "compile" a script by input path (relatively to the scripts directory)
//---------------------------------------------------------
void LuaLoadScript(lua_State* L, const char* filepath)
{
    assert(L);
    assert(filepath && filepath[0] != '\0');
    
    // generate a script's path relatively to the working dir
    char fullRelPath[128];
    fullRelPath[0] = '\0';
    strcat(fullRelPath, pathToScriptsDir);
    strcat(fullRelPath + sizeof(pathToScriptsDir) - 1, filepath);

    int status = luaL_loadfile(L, fullRelPath);

    if ((status != LUA_OK) || lua_pcall(L, 0, 0, 0))
    {
        const char* errMsg = lua_tostring(L, -1);

        LogErr(LOG, "failed to load LUA script: %s", filepath);

        switch (status)
        {
            case LUA_ERRSYNTAX:
                LuaErr(L, "Syntax error: %s", errMsg);  // syntax error in the file
                break;
            case LUA_ERRMEM:
                LuaErr(L, "Memory error: %s", errMsg);  // memory allocation error
                break;
            case LUA_ERRFILE:
                LuaErr(L, "File error: %s", errMsg);    // file-related error (can't open/read)
                break;
            default:
                LuaErr(L, "Unknown error: %s", errMsg);
        }
    }
}

//---------------------------------------------------------
// Desc:  call a Lua-function with no arguments and no results 
// Args:  func - name of the function to call
//---------------------------------------------------------
void LuaCall(lua_State* L, const char* func)
{
    assert(L);
    assert(func && func[0] != '\0');

    // 1. push function
    lua_getglobal(L, func);

    // 3. call the function
    if (lua_pcall(L, 0, 0, 0) != LUA_OK)  // do the call
        LuaErr(L, "err calling Lua func '%s': %s", func, lua_tostring(L, -1));
}

//---------------------------------------------------------
// Desc:  generic call function (call some function from the Lua script)
// Args:  func - name of the function
//        sig  - signature of the function (args + result)
//        ...  - variadic arguments
//---------------------------------------------------------
void LuaCallVa(lua_State* L, const char* func, const char* sig, ...)
{
    assert(L);
    assert(func && func[0] != '\0');

    va_list va;
    int narg = 0;   // number of arguments
    int nres = 0;   // number of results

    // 1. push function
    lua_getglobal(L, func);
    
    // 2. pushing arguments (repeat for each arg)
    va_start(va, sig);

    for (; *sig; ++narg)
    {
        // check stack space
        luaL_checkstack(L, 1, "too many args on the Lua stack");

        switch (*sig++)
        {
            case 'd':       // double arg
                lua_pushnumber(L, va_arg(va, double));
                break;
            
            case 'i':       // integer arg
            {
                int iVal = va_arg(va, int);
                lua_pushinteger(L, iVal);
                break;
            }
            
            case 's':       // string arg
                lua_pushstring(L, va_arg(va, char*));
                break;
            
            case '>':       // end of args (in signature it's a separator btw args and results)
                goto endargs;

            default:
                LuaErr(L, "invalid option (%c)", *(sig - 1));
        }
    }
endargs:

    // 3. call the function
    nres = (int)strlen(sig);

    if (lua_pcall(L, narg, nres, 0) != LUA_OK)  // do the call
        LuaErr(L, "err calling Lua func '%s': %s", func, lua_tostring(L, -1));

    // 4. get results
    nres = -nres;       // stack index of first result
    while (*sig)
    {
        switch (*sig++)
        {
            case 'd':       // double result
            {
                int isnum;
                double n = lua_tonumberx(L, nres, &isnum);
                if (!isnum)
                    LuaErr(L, "wrong result type (double)");

                *va_arg(va, double*) = n;
                break;
            }

            case 'i':       // int result
            {
                int isnum;
                int n = (int)lua_tointegerx(L, nres, &isnum);
                if (!isnum)
                    LuaErr(L, "wrong result type (int)");

                *va_arg(va, int*) = n;
                break;
            }

            case 's':       // string result
            {
                const char* s = lua_tostring(L, nres);
                if (s == NULL)
                    LuaErr(L, "wrong result type (string)");

                *va_arg(va, const char**) = s;
                break;
            }

            default:
                LuaErr(L, "invalid option (%c)", *(sig - 1));
        }
        nres++;
    }

    va_end(va);
}

//---------------------------------------------------------
// register C/C++ functions
//---------------------------------------------------------
void LuaBind(lua_State* L, const char* funcName, lua_CFunction funcPtr)
{
    assert(L);
    assert(funcName && funcName[0] != '\0');
    assert(funcPtr);

    lua_register(L, funcName, funcPtr);
}

//---------------------------------------------------------
// register a bunch of global integers (enumeration values)
//---------------------------------------------------------
void LuaRegEnumeration(lua_State* L, const EnumValue* enumArr, const int count)
{
    assert(L);
    assert(enumArr);
    assert(count > 0);

    for (int i = 0; i < count; ++i)
    {
        lua_pushinteger(L, enumArr[i].value);   // value of enumeration field
        lua_setglobal(L,   enumArr[i].name);    // key / index / name of global
    }
}