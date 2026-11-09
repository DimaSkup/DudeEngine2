/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: ui_game.h
    Desc:     functional for in-game user interface
    Created:  18.08.2026 by DimaSkup
\**********************************************************************************/
#ifndef UI_GAME_H
#define UI_GAME_H

#include "rect.h"
#include "types.h"

//---------------------------------------------------------
// forward declaration (pointer use only)
//---------------------------------------------------------
struct Texture;

//---------------------------------------------------------
// class name:  UI_Game
//---------------------------------------------------------
class UI_Game
{
public:
    struct Button
    {
        Recti    srcRect;       // which part of a texture is used to sample a button
        Recti    dstRect;       // where and which size render a button
        Color    color;         // color of a button
        Texture* pTexUp;        // basic texture for a button
        Texture* pTexHover;     // is used when the mouse hovers a button
        Texture* pTexDown;      // is used when we clicked on a button
    };

public:
    void ProcessInputGameMenu();
    void Update(const float dt);

    void RenderSceneUI();
    void RenderGameMenu();
    void RenderMouseCursor();

private:
    void UpdatePlayerHpBar();
    void RenderMenuButton(const char* key, const Button& btn, bool(*funcCallback)(void));
};

#endif
