#include "r_debug.h"
#include "entity.h"
#include "vec.h"
#include "r_main.h"
#include "game_state.h"
#include "entity_mgr.h"
#include "asset_mgr.h"
#include "ui_text_mgr.h"
#include "components/transform.h"
#include "components/collider.h"
#include "components/player.h"
#include "components/enemy.h"
#include "components/trigger.h"


//---------------------------------------------------------
// render debugging stuff
//---------------------------------------------------------
void DbgRender::Render(
    const cvector<Entity*> sceneObjects, 
    const cvector<EntityID>& visibleTriggers,
    const cvector<EntityID>& visibleEnemies)
{
    // if we're in the game menu we render only dbg text
    if (g_GameState.bGameMode && g_GameState.bGamePaused)
    {
        if (g_GameState.bShowDebugInfo)
            RenderDbgInfo();
        return;
    }


    if (g_GameState.bShowColliders)
        RenderColliders(sceneObjects);

    if (g_GameState.bShowTriggers)
        RenderTriggers(visibleTriggers);

    if (g_GameState.bShowScreenDbgGrid)
        RenderScreenGrid();

    if (g_GameState.bShowAttackRange)
        RenderAttackRanges(visibleEnemies);

    // text info
    if (g_GameState.bShowDebugInfo)
        RenderDbgInfo();
}

//---------------------------------------------------------
// render debug info (text) onto the screen (when in game mode)
//---------------------------------------------------------
void DbgRender::RenderDbgInfo()
{
    static int fps = 0;
    static float timer = 0;

    timer += g_GameState.deltaTime;

    // update fps each 0.1 sec
    if (timer > 0.1f)
    {
        timer = 0;
        fps = (int)floorf(1.0f / g_GameState.deltaTime);
    }


    g_Text.SetFontType(FONT_TYPE_DEBUG);
    g_Text.SetTextColor(255, 255, 255, 255);

    const int fontHeight = g_Text.GetFontHeight(FONT_TYPE_DEBUG);

    g_Text.SetPos(0, 0);
    g_Text.Draw(L"Fps: %d", fps);

    g_Text.SetPos(0, fontHeight);
    g_Text.Draw(L"Frame time: %.2f ms", g_GameState.deltaTime * 1000.0f);

    g_Text.SetPos(0, fontHeight*2);
    g_Text.Draw(L"Mouse: %d:%d", g_GameState.mousePos.x, g_GameState.mousePos.y);

    g_Text.SetFontType(FONT_TYPE_GAME);
}

//---------------------------------------------------------
// render a debug grid onto the screen
//---------------------------------------------------------
void DbgRender::RenderScreenGrid()
{
    const Vec2i wndSz = g_GameState.wndSize;
    const int quarterW = wndSz.w >> 2;
    const int quarterH = wndSz.h >> 2;

    g_Render.SetDrawColor(0, 255, 0, 255);

    // vertical lines
    g_Render.DrawLine(quarterW * 1, 0, quarterW * 1, wndSz.h);  // left
    g_Render.DrawLine(quarterW * 2, 0, quarterW * 2, wndSz.h);  // center
    g_Render.DrawLine(quarterW * 3, 0, quarterW * 3, wndSz.h);  // right

    // horizontal
    g_Render.DrawLine(0, quarterH * 1, wndSz.w, quarterH * 1);  // top
    g_Render.DrawLine(0, quarterH * 2, wndSz.w, quarterH * 2);  // center
    g_Render.DrawLine(0, quarterH * 3, wndSz.w, quarterH * 3);  // bottom
}

//---------------------------------------------------------
// visualize attack ranges of the player and enemies
//---------------------------------------------------------
void DbgRender::RenderAttackRanges(const cvector<EntityID>& enemies)
{
    //
    // draw player's attack range
    //
    const Entity* pPlayer = g_EnttMgr.GetPlayer();
    Vec2i          center = ToVec2i(pPlayer->GetComponent<Transform>()->GetCenter());
    const WeaponID  wpnId = pPlayer->GetComponent<Player>()->GetCurrWeaponId();
    int            radius = g_AssetMgr.GetWpnById(wpnId)->attackRadius;

    // adjust position to be in screen coords
    center = Vec2iSub(center, g_GameState.camPos);
    
    g_Render.SetDrawColor(255, 0, 0, 255);
    g_Render.DrawCircle(center.x, center.y, radius);

    //
    // visualize enemies' attack ranges
    //
    for (const EntityID id : enemies)
    {
        const Entity* pEnemy = g_EnttMgr.GetEntt(id);
        assert(pEnemy);

        center = ToVec2i(pEnemy->GetComponent<Transform>()->GetCenter());
        radius = pEnemy->GetComponent<Enemy>()->GetAttackRadius();

        // adjust position to be in screen coords
        center = Vec2iSub(center, g_GameState.camPos);

        g_Render.DrawCircle(center.x, center.y, radius);
    }
}

//---------------------------------------------------------
// visualize triggers
//---------------------------------------------------------
void DbgRender::RenderTriggers(const cvector<EntityID>& enttsIds)
{
    // correction of positioning
    const Vec2i offset = { -g_GameState.camPos.x, -g_GameState.camPos.y };

    // apply the same rendering params for all the triggers
    g_Render.SetRenderDrawBlendMode(BLENDMODE_BLEND);
    g_Render.SetDrawColor(0, 0, 255, 128);

    // visualize each trigger
    for (const EntityID id : enttsIds)
    {
        Entity* pEntt = g_EnttMgr.GetEntt(id);
        assert(pEntt);
        assert(pEntt->HasComponent<Trigger>());
        assert(pEntt->HasComponent<Collider>());

        const Collider* c = pEntt->GetComponent<Collider>();

        //
        // render a box
        //
        if (c->GetType() == COLLIDER_BOX)
        {
            const Recti box = c->GetBoxInWorld();

            // endpoints (already screen coords)
            const int l = box.x0 + offset.x;
            const int r = box.x1 + offset.x;
            const int t = box.y0 + offset.y;
            const int b = box.y1 + offset.y;

            g_Render.DrawFillRect(l, t, r, b);
        }

        //
        // render a circle
        //
        else if (c->GetType() == COLLIDER_CIRCLE)
        {
            const CircleFixed circle = c->GetCircleInWorld();
            const int cx = circle.cx + offset.x;
            const int cy = circle.cy + offset.y;

            g_Render.DrawCircle(cx, cy, circle.r);
        }
        else
        {
            LogErr(LOG, "unknown type of collider (entt [id: %d, name: %s], collider: %d", id, pEntt->GetName(), c->GetType());
        }

    }

    g_Render.SetRenderDrawBlendMode(BLENDMODE_NONE);
}

//---------------------------------------------------------
// visualize entities colliders' edges
//---------------------------------------------------------
void DbgRender::RenderColliders(const cvector<Entity*>& entts)
{
    // correction of positioning
    const Vec2i offset = { -g_GameState.camPos.x, -g_GameState.camPos.y };

    for (Entity* pEntt : entts)
    {
        if (!pEntt->HasComponent<Collider>())
            continue;

        const Collider* c = pEntt->GetComponent<Collider>();

        //
        // render a box
        //
        if (c->GetType() == COLLIDER_BOX)
        {
            const Recti box = c->GetBoxInWorld();

            // endpoints (already screen coords)
            const int l = box.x0 + offset.x;
            const int r = box.x1 + offset.x;
            const int t = box.y0 + offset.y;
            const int b = box.y1 + offset.y;
            
            g_Render.SetDrawColor(255, 255, 0, 255);
            g_Render.DrawRect(l, t, r, b);

            // render min and max POINTS of box
            const int sz = 3;

            g_Render.SetDrawColor(255, 0, 0, 255);
            g_Render.DrawFillRect(l-sz, t-sz, l+sz, t+sz);   // min

            g_Render.SetDrawColor(0, 0, 255, 255);
            g_Render.DrawFillRect(r-sz, b-sz, r+sz, b+sz);   // max
        }
        
        //
        // render a circle
        //
        else if (c->GetType() == COLLIDER_CIRCLE)
        {
            const CircleFixed circle = c->GetCircleInWorld();
            const int cx = circle.cx + offset.x;
            const int cy = circle.cy + offset.y;

            g_Render.SetDrawColor(255, 255, 0, 255);
            g_Render.DrawCircle(cx, cy, circle.r);
        }

        else
            LogErr(LOG, "unknown type of collider: %d", (int)c->GetType());
    }
}
