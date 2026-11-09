/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: ui_game.cpp
    Desc:     functional for in-game user interface
    Created:  18.08.2026 by DimaSkup
\**********************************************************************************/
#include "ui_game.h"
#include "game_state.h"
#include "input.h"
#include "asset_mgr.h"
#include "entity_mgr.h"
#include "r_main.h"
#include "macros.h"
#include "ui_text_mgr.h"
#include "collision.h"

#include "components/sprite.h"
#include "components/health.h"


//---------------------------------------------------------
// forward declaration of private handlers
//---------------------------------------------------------
bool HandleMenuBtnContinue();
bool HandleMenuBtnQuit();

//---------------------------------------------------------
// process input events when we are in the game menu
//---------------------------------------------------------
void UI_Game::ProcessInputGameMenu()
{
    assert(g_GameState.bGamePaused == true);

    InputEvent e;

    while (InputGetEvent(&e))
    {
        if (e.type == EVENT_KEYDOWN)
        {
            if (e.keyboard.code == KEY_ESCAPE && !e.keyboard.bRepeat)
                g_GameState.bGamePaused = false;
        }
        else if (e.type == EVENT_MOUSE)
        {
            switch (e.mouse.type)
            {
                case MOUSE_MOVE:
                {
                    g_GameState.mousePos = { e.mouse.x, e.mouse.y };
                    break;
                }
            }
        }
    }
}

//---------------------------------------------------------
// update UI elements over the time
//---------------------------------------------------------
void UI_Game::Update(const float dt)
{
    UpdatePlayerHpBar();
}

//---------------------------------------------------------
// update width of the player's hp bar to have length according to health
//---------------------------------------------------------
void UI_Game::UpdatePlayerHpBar()
{
    return;

    Entity* player = g_EnttMgr.GetPlayer();
    Entity* pHpBar = g_EnttMgr.GetEntt("player_hp");
    Transform* t = pHpBar->GetComponent<Transform>();

    Vec2i sz = t->GetLocalSize();
    static float fullWidth = (float)sz.w;
    const float dx = fullWidth * 0.01f;
    const int hp = player->GetComponent<Health>()->GetHP();

    sz.w = (int)(dx * (float)hp);

    // HACK to prevent fucking crash: any stuff on screen must have at least 1 pixel
    if (sz.w <= 0) 
        sz.w = 1;

    t->SetLocalSize(sz);
}

//---------------------------------------------------------
// render UI elements when we are on the scene
//---------------------------------------------------------
void UI_Game::RenderSceneUI()
{
    const char* items[] = {
        "ui_bar_player_main",
        "ui_bar_player_statues",
        "ui_ico_item_book",
        "ui_ico_item_sword",
        "ui_ico_item_staff_fire",
        "ui_ico_item_map",
    };


    for (int i = 0; i < ARRAY_SIZE(items); ++i)
    {
        Entity* pEntt = g_EnttMgr.GetEntt(items[i]);
        Sprite* pSpr = pEntt->GetComponent<Sprite>();
        assert(pSpr != nullptr);

        pSpr->SetShow(true);
        pSpr->Render();
        pSpr->SetShow(false);
    }
}

//---------------------------------------------------------
// render the game main menu (when we press the ESC key)
//---------------------------------------------------------
void UI_Game::RenderGameMenu()
{
    //
    // render background
    //
    TexID  texId = g_AssetMgr.GetTexId("ui_game_menu_bg");
    Texture* tex = g_AssetMgr.GetTex(texId);

    Recti srcRect = { 0, tex->w, 0, tex->h };
    Recti dstRect = g_Render.GetWndRecti();
    Color   color = { (uint32)RGBATOU32(255, 255, 255, 255) };
    g_Render.DrawRect(tex, srcRect, dstRect, color, 0, NULL, 0.0f);

    //
    // render buttons
    //
    int px = 1000;
    int py = 200;

    g_Text.SetFontType(FONT_TYPE_GAME);
    const int offsetY = g_Text.GetFontHeight(FONT_TYPE_GAME) + 32;

    const TexID texIdBtnUp    = g_AssetMgr.GetTexId("ui_game_menu_btn");
    const TexID texIdBtnHover = g_AssetMgr.GetTexId("ui_game_menu_btn_hover");
    const TexID texIdBtnDown  = g_AssetMgr.GetTexId("ui_game_menu_btn_pressed");

    Texture* texBtnUp         = g_AssetMgr.GetTex(texIdBtnUp);
    Texture* texBtnHover      = g_AssetMgr.GetTex(texIdBtnHover);
    Texture* texBtnDown       = g_AssetMgr.GetTex(texIdBtnDown);

    tex = texBtnUp;

    // check if all kinds of button textures have the same size
    assert((texBtnUp->w == texBtnHover->w) && (texBtnUp->h == texBtnHover->h));
    assert((texBtnUp->w == texBtnDown->w) && (texBtnUp->h == texBtnDown->h));
    
    srcRect = { 0, tex->w, 0, tex->h };
    dstRect = { px - 24, px + 400, py, py + offsetY - 16 };

    g_Text.SetShadowOffset(3, 3);
    g_Text.SetShadowColor(22, 29, 30, 255);
    g_Text.SetTextColor(255, 255, 255, 255);

    Button btn;
    btn.srcRect   = srcRect;
    btn.dstRect   = dstRect;
    btn.color     = color;
    btn.pTexUp    = texBtnUp;
    btn.pTexHover = texBtnHover;
    btn.pTexDown  = texBtnDown;

    RenderMenuButton("$continue", btn, HandleMenuBtnContinue);
    btn.dstRect.y0 += offsetY;
    btn.dstRect.y1 += offsetY;

    RenderMenuButton("$new_game", btn, HandleMenuBtnContinue);
    btn.dstRect.y0 += offsetY;
    btn.dstRect.y1 += offsetY;

    RenderMenuButton("$save_game", btn, HandleMenuBtnContinue);
    btn.dstRect.y0 += offsetY;
    btn.dstRect.y1 += offsetY;

    RenderMenuButton("$load_game", btn, HandleMenuBtnContinue);
    btn.dstRect.y0 += offsetY;
    btn.dstRect.y1 += offsetY;

    RenderMenuButton("$options", btn, HandleMenuBtnContinue);
    btn.dstRect.y0 += offsetY;
    btn.dstRect.y1 += offsetY;

    RenderMenuButton("$quit", btn, HandleMenuBtnQuit);

    RenderMouseCursor();
}

//---------------------------------------------------------
// render a button in the game main menu
//---------------------------------------------------------
void UI_Game::RenderMenuButton(
    const char* key,
    const UI_Game::Button& btn,
    bool(*funcCallback)(void))    // handler for the button pressing
{
    assert(key && key[0] != '\0');
    assert(btn.pTexUp);
    assert(btn.pTexHover);
    assert(btn.pTexDown);
    assert(funcCallback);

    const Mouse& mouse = GetMouseState();

    const int textAtX = btn.dstRect.x0 + 24;
    const int textAtY = btn.dstRect.y0;

    g_Text.SetPos(textAtX, textAtY);

    Texture* tex = btn.pTexUp;

    // is mouse cursor over the button?
    if (CollisionRectPoint(btn.dstRect, Vec2i(mouse.px, mouse.py)))
    {
        // handle releasing the button
        if (mouse.button[MOUSE_L].bReleased)
        {
            funcCallback();
            tex = btn.pTexUp;
        }

        // handle pressing the button
        else if (mouse.button[MOUSE_L].bPressed || mouse.button[MOUSE_L].bDown)
        {
            tex = btn.pTexDown;
            g_Text.SetPos(textAtX + 1, textAtY + 3);
        }
        else
        {
            tex = btn.pTexHover;
        }
    }

    // draw a button: background texture + text over it
    g_Render.DrawRect(tex, btn.srcRect, btn.dstRect, btn.color, 0, NULL, 0.0f);
    g_Text.Draw(key);
}

//---------------------------------------------------------
// draw a mouse cursor
//---------------------------------------------------------
void UI_Game::RenderMouseCursor()
{
    const TexID  texId   = g_AssetMgr.GetTexId("ui_mouse_cursor");
    Texture*     tex     = g_AssetMgr.GetTex(texId);
    Recti        srcRect = { 0, tex->w, 0, tex->h };
    const Mouse& m       = GetMouseState();
    Recti        dstRect = { m.px, m.px + tex->w, m.py, m.py + tex->h };
    Color        color   = { (uint32)RGBATOU32(255, 255, 255, 255) };
    const uint32 flip    = 0;
    const Vec2i* center  = NULL;    // no specific center of rotation (used a default one)
    const float  angle   = 0.0f;    // rotation angle

    g_Render.DrawRect(tex, srcRect, dstRect, color, flip, center, angle);
}

//---------------------------------------------------------
// private handlers for game UI events
//---------------------------------------------------------
bool HandleMenuBtnContinue()
{
    g_GameState.bGamePaused = false;
    return true;
}

bool HandleMenuBtnQuit()
{
    g_GameState.bRunning = false;
    return true;
}
