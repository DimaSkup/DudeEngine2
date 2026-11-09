/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: lua_lib_text.cpp
    Desc:     a library for using in Lua to work with text
    Created:  29.07.2026 by DimaSkup
\**********************************************************************************/
#include "lua_lib_text.h"
#include "ui_text_mgr.h"
#include "game_state.h"
#include <assert.h>


// the library name
static const char* s_LibName = "text";

//---------------------------------------------------------
// all the following text will be drawn with this color (until next change)
//---------------------------------------------------------
static int SetTextColor(lua_State* L)
{
    assert(L);

    // get 4 values for each color channel in range [0, 255]
    const uint8 r = (uint8)luaL_checkinteger(L, 1);
    const uint8 g = (uint8)luaL_checkinteger(L, 2);
    const uint8 b = (uint8)luaL_checkinteger(L, 3);
    const uint8 a = (uint8)luaL_checkinteger(L, 4);

    g_Text.SetTextColor(r, g, b, a);
    return 0;   // number of results
}

//---------------------------------------------------------
// all the following shadows of the text will be drawn with this color
//---------------------------------------------------------
static int SetShadowColor(lua_State* L)
{
    assert(L);

    // get 4 values for each color channel in range [0, 255]
    const uint8 r = (uint8)luaL_checkinteger(L, 1);
    const uint8 g = (uint8)luaL_checkinteger(L, 2);
    const uint8 b = (uint8)luaL_checkinteger(L, 3);
    const uint8 a = (uint8)luaL_checkinteger(L, 4);

    g_Text.SetShadowColor(r, g, b, a);
    return 0;   // number of results
}

//---------------------------------------------------------
// all the following shadows of the text will be drawn with 
// this offset relatively to its text
//---------------------------------------------------------
static int SetShadowOffset(lua_State* L)
{
    assert(L);

    const int offsetX = (int)luaL_checkinteger(L, 1);
    const int offsetY = (int)luaL_checkinteger(L, 2);

    g_Text.SetShadowOffset(offsetX, offsetY);
    return 0;   // number of results
}

//---------------------------------------------------------
// setup fading of the text during N seconds
//---------------------------------------------------------
static int SetFadeSec(lua_State* L)
{
    assert(L);
    const float sec = (float)luaL_checknumber(L, 1);
    g_Text.SetFadeSec(sec);
    return 0;   // number of results
}

//---------------------------------------------------------
// setup a current type of font
//---------------------------------------------------------
static int SetFontType(lua_State* L)
{
    assert(L);
    const eFontType fontType = (eFontType)(luaL_checkinteger(L, 1));
    g_Text.SetFontType(fontType);
    return 0;   // number of results
}

//---------------------------------------------------------
// setup position of text based on input percentage of the screen
//---------------------------------------------------------
static int SetPosPercent(lua_State* L)
{
    assert(L);

    const float percentX = (float)luaL_checknumber(L, 1);
    const float percentY = (float)luaL_checknumber(L, 2);

    assert(percentX >= -100.0f && percentX <= 100.0f);
    assert(percentY >= -100.0f && percentY <= 100.0f);

    const Vec2i wndSz = g_GameState.wndSize;

    const Vec2i offset = ToVec2i({
        (float)wndSz.w * percentX * 0.01f,
        (float)wndSz.h * percentY * 0.01f });

    // if offset is negative then we go in opposite direction (right=>left, bottom=>top)
    Vec2i pos = { 0,0 };
    pos.x = (offset.x >= 0) ? offset.x : (wndSz.w + offset.x);
    pos.y = (offset.y >= 0) ? offset.y : (wndSz.h + offset.y);

    assert(pos.x >= 0 && pos.x <= wndSz.w);
    assert(pos.y >= 0 && pos.y <= wndSz.h);

    g_Text.SetPos(pos.x, pos.y);

    return 0;   // number of results
}

//---------------------------------------------------------
// setup position of text with fixed screen coords (directly in pixels)
//---------------------------------------------------------
static int SetPosFixed(lua_State* L)
{
    assert(L);

    int px = (int)luaL_checkinteger(L, 1);
    int py = (int)luaL_checkinteger(L, 2);

    const Vec2i& wndSz = g_GameState.wndSize;
    assert(px >= -wndSz.w && px <= wndSz.w);
    assert(py >= -wndSz.h && py <= wndSz.h);

    // if position is negative then we go in opposite direction (right=>left, bottom=>top)
    if (px < 0)
        px = wndSz.w + px;

    if (py < 0)
        py = wndSz.h + py;

    g_Text.SetPos(px, py);

    return 0;   // number of results
}

//---------------------------------------------------------
// draw a text by input key (NOTE: push it for rendering, but not immediate drawing)
//---------------------------------------------------------
static int Draw(lua_State* L)
{
    assert(L);
    const char* key = luaL_checkstring(L, 1);
    g_Text.Draw(key);
    return 0;   // number of results
}

//---------------------------------------------------------
// remove all the faded texts (so they won't be rendered as well)
//---------------------------------------------------------
static int ClearFaded(lua_State* L)
{
    g_Text.ClearFadedTexts();
    return 0;   // number of results
}

//---------------------------------------------------------
// setup text on the screen (shadowed, faded, centered, aligned, etc.)
//---------------------------------------------------------
static int PushFlag(lua_State* L) 
{ 
    assert(L); 
    const uint16 mask = (uint16)luaL_checkinteger(L, 1);
    g_Text.PushFlags(mask);
    return 0;   // number of results
}

static int PopFlag(lua_State* L)
{ 
    assert(L); 
    const uint16 mask = (uint16)luaL_checkinteger(L, 1);
    g_Text.PopFlags(mask);
    return 0;   // number of results
}

//---------------------------------------------------------
// flush all the text manipulation flags (no text modifiers will be used)
//---------------------------------------------------------
static int FlushFlags(lua_State* L)
{
    g_Text.FlushFlags();
    return 0;
}

//---------------------------------------------------------
// get a height of the currently used font
//---------------------------------------------------------
static int GetFontHeight(lua_State* L)
{
    assert(L);
    const eFontType type = (eFontType)(luaL_checkinteger(L, 1));
    lua_pushinteger(L, g_Text.GetFontHeight(type));
    return 1;   // number of resutls
}

//---------------------------------------------------------
// get a localized string by input key
//---------------------------------------------------------
static int GetStrByKey(lua_State* L)
{
    assert(L);
    const char* key = luaL_checkstring(L, 1);
    
    return 1;   // number of results
}

//---------------------------------------------------------
// the library functions
//---------------------------------------------------------
static const struct luaL_Reg s_LibFunx[] =
{
    {"SetTextColor",    SetTextColor},
    {"SetShadowColor",  SetShadowColor},
    {"SetShadowOffset", SetShadowOffset},
    {"SetFadeSec",      SetFadeSec},
    {"SetFontType",     SetFontType},
    {"SetPosPercent",   SetPosPercent},
    {"SetPosFixed",     SetPosFixed},

    {"ClearFaded",      ClearFaded},

    {"Draw",            Draw},

    {"PushFlag",        PushFlag},
    {"PopFlag",         PopFlag},
    {"FlushFlags",      FlushFlags},

    {"GetFontHeight",   GetFontHeight},
    {"GetStrByKey",     GetStrByKey},

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
// register the lib for the input Lua-state
//---------------------------------------------------------
void LuaLoadTextLib(lua_State* L)
{
    luaL_requiref(L, s_LibName, LibRequire, 1);
    lua_pop(L, 1);  // pop the lib from the stack
}