/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   ui_facade_engine.h
    Desc:       facade (it is an OOP pattern) of the engine for using in UI
    Created:    13.05.2026 by DimaSkup
\**********************************************************************************/
#include "ui_facade_engine.h"
#include "game.h"
#include "map.h"
#include "asset_mgr.h"
#include "entity_mgr.h"
#include "ui_text_mgr.h"
#include "game_state.h"
#include "components/sprite.h"
#include "components/movement.h"

#include "ui_ed_state.h"

#include "log.h"
#include <assert.h>


//---------------------------------------------------------
// global instance
//---------------------------------------------------------
UI_FacadeEngine* g_pFacadeEngine;

//---------------------------------------------------------
// constructor
//---------------------------------------------------------
UI_FacadeEngine::UI_FacadeEngine(Game* pGame) :
    pGame_(pGame)
{
    assert(pGame);
}

//---------------------------------------------------------
// handle selection (picking) of entity by ID
//---------------------------------------------------------
void UI_FacadeEngine::SelectEntt(const EntityID id)
{
    g_EditorState.selectedEnttId = id;

    // if we deselect 
    if (id == INVALID_ENTT_ID)
    {
        ShowEditorCursor(false);
        return;
    }

    // transform the "outliner" (red rectangle) to highlight the selected entity
    Vec2 pos = { 0,0 };
    Vec2i localSz = { 0,0 };
    int scale = 0;

    GetEnttTransform(id, pos, localSz, scale);
    UpdateEditorCursorTransform(pos, localSz, scale);

    ShowEditorCursor(g_EditorState.bShowEditorCursor);
}

//---------------------------------------------------------
// show/hide cursor in the editor (red square) used to highlight current selection
//---------------------------------------------------------
void UI_FacadeEngine::ShowEditorCursor(const bool showHide)
{
    if (!g_EnttMgr.HasEntt("editor_cursor"))
        return;

    Sprite* pSpr = GetEditorCursor()->GetComponent<Sprite>();
    assert(pSpr && "your editor cursor must have a Sprite component!");
    pSpr->SetShow(showHide);
}

//---------------------------------------------------------
// get a name of the current game level
//---------------------------------------------------------
const char* UI_FacadeEngine::GetGameLevelName() const
{
    return pGame_->GetCurrLevelName();
}

//---------------------------------------------------------
// return the number of rendered items related to the layer
//---------------------------------------------------------
uint UI_FacadeEngine::GetNumRenderItems(const eLayerType layer) const
{
    return pGame_->GetNumRenderItems(layer);
}

//---------------------------------------------------------
// create a new game level
//---------------------------------------------------------
void UI_FacadeEngine::CreateLevel(const char* levelName)
{
    pGame_->CreateLevel(levelName);
}

//---------------------------------------------------------
// save level changes
//---------------------------------------------------------
void UI_FacadeEngine::SaveLevel()
{
    pGame_->SaveLevel();
}

//---------------------------------------------------------
// load a game level by input name
//---------------------------------------------------------
void UI_FacadeEngine::LoadLevel(const char* levelName)
{
    Game::GameInit gameInit;
    pGame_->ReadGameInitParams("init.cfg", gameInit);
    pGame_->LoadLevel(levelName, gameInit);
}


//*****************************************************************************
// 
//                                EVENTS
// 
//*****************************************************************************
const char* UI_FacadeEngine::GetEventNameById(const EventID id) const
{
    return pGame_->GetEventMgr()->GetEventNameById(id);
}

EventID UI_FacadeEngine::GetEventIdByName(const char* eventName) const
{
    return pGame_->GetEventMgr()->GetEventIdByName(eventName);
}

const cvector<EventID>& UI_FacadeEngine::GetAllEventsIds() const
{
    return pGame_->GetEventMgr()->GetEventsIds();
}

const cvector<EventName>& UI_FacadeEngine::GetAllEventsNames() const
{
    return pGame_->GetEventMgr()->GetEventsNames();
}

//---------------------------------------------------------
// get a ptr to "editor cursor" (a red square used for selection highlighting)
//---------------------------------------------------------
Entity* UI_FacadeEngine::GetEditorCursor()
{
    return g_EnttMgr.GetEntt("editor_cursor");
}

//---------------------------------------------------------
// Desc:  when we transform a selected entity (in editor) 
//        we also have to update the editor's cursor as well
//---------------------------------------------------------
void UI_FacadeEngine::UpdateEditorCursorTransform(
    const Vec2 pos,
    const Vec2i localSize,
    const int scale)
{
    Entity* pCursor = GetEditorCursor();
    assert(pCursor);

    SetEnttPos      (pCursor->GetId(), pos);
    SetEnttLocalSize(pCursor->GetId(), localSize);
    SetEnttScale    (pCursor->GetId(), scale);
}

//---------------------------------------------------------
// get an id of entt by input name
//---------------------------------------------------------
EntityID UI_FacadeEngine::GetEnttId(const char* enttName) const 
{
    return g_EnttMgr.GetEntt(enttName)->GetId();
}

//---------------------------------------------------------
// enable/disable rendering of entity by input id
//---------------------------------------------------------
void UI_FacadeEngine::SetShowEntt(const EntityID id, const bool showHide)
{
    Entity* pEntt = g_EnttMgr.GetEntt(id);
    assert(pEntt);

    if (!pEntt->HasComponent<Sprite>())
    {
        LogErr(LOG, "entt (id: %d, name: %s) has no Sprite component!", (int)id, pEntt->GetName());
        return;
    }

    pEntt->GetComponent<Sprite>()->SetShow(showHide);
}

//---------------------------------------------------------
// get current color of entt by input id
//---------------------------------------------------------
Color UI_FacadeEngine::GetEnttColor(const EntityID id) const 
{
    const Entity* pEntt = g_EnttMgr.GetEntt(id);
    assert(pEntt);

    if (!pEntt->HasComponent<Sprite>())
    {
        LogErr(LOG, "entt (id: %d, name: %s) doesn't have a Sprite component!", (int)id, pEntt->GetName());
        return Color{ 0 };
    }
        
    return pEntt->GetComponent<Sprite>()->GetColor();
}

//---------------------------------------------------------
// set current color of entt by input id
//---------------------------------------------------------
void UI_FacadeEngine::SetEnttColor(const EntityID id, const Color c) 
{
    Entity* pEntt = g_EnttMgr.GetEntt(id);
    assert(pEntt);

    if (!pEntt->HasComponent<Sprite>())
    {
        LogErr(LOG, "entt (id: %d, name: %s) has no Sprite component!", (int)id, pEntt->GetName());
        return;
    }

    return pEntt->GetComponent<Sprite>()->SetColor(c);
}

//---------------------------------------------------------
// get transformation of entt by id: position, original size, scale in world
//---------------------------------------------------------
void UI_FacadeEngine::GetEnttTransform(
    const EntityID id, 
    Vec2& pos, 
    Vec2i& sz, 
    int& scale) const
{
    Entity* pEntt = g_EnttMgr.GetEntt(id);
    assert(pEntt && pEntt->HasComponent<Transform>());

    const Transform* t = pEntt->GetComponent<Transform>();
    pos   = t->GetPos();
    sz    = t->GetLocalSize();
    scale = t->GetScale();
}

//---------------------------------------------------------
// get entity's center position in the world
//---------------------------------------------------------
Vec2 UI_FacadeEngine::GetEnttCenter(const EntityID id) const
{
    Entity* pEntt = g_EnttMgr.GetEntt(id);
    assert(pEntt && pEntt->HasComponent<Transform>());

    return pEntt->GetComponent<Transform>()->GetCenter();
}

//---------------------------------------------------------
// setup a position of entity by ID
//---------------------------------------------------------
void UI_FacadeEngine::SetEnttPos(const EntityID id, const Vec2 p)
{
    // if outside of the world
    if (p.x < 0 || p.x >= g_GameState.fWorldSize.w || p.y < 0 || p.y >= g_GameState.fWorldSize.h)
        return;

    Entity* pEntt = g_EnttMgr.GetEntt(id);
    assert(pEntt && pEntt->HasComponent<Transform>());

    pEntt->GetComponent<Transform>()->SetPos(p);
}

//---------------------------------------------------------
// setup a velocity of entity by ID
//---------------------------------------------------------
void UI_FacadeEngine::SetEnttVel(const EntityID id, const Vec2 vel)
{
    Entity* pEntt = g_EnttMgr.GetEntt(id);
    assert(pEntt && pEntt->HasComponent<Movement>());
    
    pEntt->GetComponent<Movement>()->SetVelocity(vel.x, vel.y);
}

//---------------------------------------------------------
// setup a local size of entity by ID
//---------------------------------------------------------
void UI_FacadeEngine::SetEnttLocalSize(const EntityID id, const Vec2i sz)
{
    Entity* pEntt = g_EnttMgr.GetEntt(id);
    assert(pEntt && pEntt->HasComponent<Transform>());

    pEntt->GetComponent<Transform>()->SetLocalSize(sz);
}

//---------------------------------------------------------
// setup a scale of entity by ID
//---------------------------------------------------------
void UI_FacadeEngine::SetEnttScale(const EntityID id, const int scale)
{
    Entity* pEntt = g_EnttMgr.GetEntt(id);
    assert(pEntt->HasComponent<Transform>());

    pEntt->GetComponent<Transform>()->SetScale(scale);
}

//---------------------------------------------------------
// return a position in world of entity by input ID
//---------------------------------------------------------
Vec2 UI_FacadeEngine::GetEnttPos(const EntityID id) const
{
    const Entity* pEntt = g_EnttMgr.GetEntt(id);
    const Transform* t = pEntt->GetComponent<Transform>();
    return t->GetPos();
}

//---------------------------------------------------------
// return a local size (without scaling) of entity by input ID
//---------------------------------------------------------
Vec2i UI_FacadeEngine::GetEnttLocalSize(const EntityID id) const
{
    const Entity* pEntt = g_EnttMgr.GetEntt(id);
    const Transform* t = pEntt->GetComponent<Transform>();
    return t->GetLocalSize();
}

//---------------------------------------------------------
// return a scaling in world of entity by input ID
//---------------------------------------------------------
int UI_FacadeEngine::GetEnttScale(const EntityID id) const
{
    const Entity* pEntt = g_EnttMgr.GetEntt(id);
    const Transform* t = pEntt->GetComponent<Transform>();
    return t->GetScale();
}

//---------------------------------------------------------
// set movement velocity of entity by id
//---------------------------------------------------------
void UI_FacadeEngine::SetEnttVelocity(const EntityID id, const Vec2 vel)
{
    Entity* pEntt = g_EnttMgr.GetEntt(id);
    assert(pEntt && pEntt->HasComponent<Movement>());

    pEntt->GetComponent<Movement>()->SetVelocity(vel.x, vel.y);
}

//---------------------------------------------------------
// get movement velocity of entity by id
//---------------------------------------------------------
void UI_FacadeEngine::GetEnttVelocity(const EntityID id, Vec2& vel)
{
    Entity* pEntt = g_EnttMgr.GetEntt(id);
    assert(pEntt && pEntt->HasComponent<Movement>());

    vel = pEntt->GetComponent<Movement>()->GetVelocity();
}

//---------------------------------------------------------
// get an arr of all the entities related to the input layer
//---------------------------------------------------------
const cvector<Entity*>& UI_FacadeEngine::GetEnttsByLayer(eLayerType layer) const
{
    return g_EnttMgr.GetEnttsByLayer(layer);
}

//---------------------------------------------------------
// is layer enabled for rendering?
//---------------------------------------------------------
bool UI_FacadeEngine::IsShowLayer(eLayerType layer) const
{
    return g_EnttMgr.IsShowLayer(layer);
}

//---------------------------------------------------------
// enable/disable rendering of the layer
//---------------------------------------------------------
void UI_FacadeEngine::SetShowLayer(eLayerType layer, bool showHide)
{
    g_EnttMgr.SetShowLayer(layer, showHide);
}


//*****************************************************************************
//                            TEXT / LOCALES
//*****************************************************************************

const char* UI_FacadeEngine::GetCurrLocale() const
{
    return g_Text.GetCurrLocale();
}

const char** UI_FacadeEngine::GetAvailLocales() const
{
    return g_Text.GetAvailLocales();
}

size_t UI_FacadeEngine::GetNumAvailLocales() const
{
    return g_Text.GetNumAvailLocales();
}

void UI_FacadeEngine::SetLocale(const char* loc) const
{
    g_Text.SetLocale(loc);
}


//*****************************************************************************
//                                CAMERA
//*****************************************************************************
void UI_FacadeEngine::FocusCameraOnEntt(const EntityID id)
{
    const Vec2 enttCenter = GetEnttCenter(id);
    const Vec2 camOffset = Vec2Scale(g_GameState.viewportSize, 0.5f);
    g_GameState.camPos = ToVec2i(Vec2Sub(enttCenter, camOffset));
}

//*****************************************************************************
//                             TEXTURES STUFF
//*****************************************************************************

//---------------------------------------------------------
// get a texture id by input texture name
//---------------------------------------------------------
TexID UI_FacadeEngine::GetTexId(const char* texName) const
{
    return g_AssetMgr.GetTexId(texName);
}

//---------------------------------------------------------
// get a texture id of the map's current tilemap
//---------------------------------------------------------
TexID UI_FacadeEngine::GetMapTexId() const
{
    return pGame_->GetMap()->GetTexId();
}

//---------------------------------------------------------
// get a ptr to the current texture of the tilemap
//---------------------------------------------------------
Texture* UI_FacadeEngine::GetTexTilemap() const
{
    return g_AssetMgr.GetTex(GetMapTexId());
}

//---------------------------------------------------------
// get a ptr to texture by input id
//---------------------------------------------------------
Texture* UI_FacadeEngine::GetTex(const TexID id) const
{
    return g_AssetMgr.GetTex(id);
}

//---------------------------------------------------------
// get number of tiles rows and columns on the MAP'S TEXTURE AT texture atlas 
//---------------------------------------------------------
Vec2i UI_FacadeEngine::GetTexNumTiles(const TexID id) const
{
    return pGame_->GetMap()->GetNumTexTiles();
}

//---------------------------------------------------------
// get width and height of texture by id
//---------------------------------------------------------
Vec2i UI_FacadeEngine::GetTexSize(const TexID id) const
{
    return g_AssetMgr.GetTexSize(id);
}

//---------------------------------------------------------
// Desc:  load and create a new texture from file
// Args:  texName - set as name for this texture
//        texPath - path to texture file
//---------------------------------------------------------
TexID UI_FacadeEngine::LoadTexture(const char* texName, const char* texPath)
{
    return g_AssetMgr.LoadTexture(texName, texPath);
}

//---------------------------------------------------------
// Desc:  get a name of texture by input id
//---------------------------------------------------------
const char* UI_FacadeEngine::GetTexName(const TexID id) const
{
    return g_AssetMgr.GetTexName(id);
}

//---------------------------------------------------------
// setup a texture for the tilemap
//---------------------------------------------------------
void UI_FacadeEngine::SetMapTexture(const TexID texId)
{
    pGame_->GetMap()->SetTexture(texId);
}

//---------------------------------------------------------
// Desc:  setup a texture tile for a map cell
// Args:  tileIdx - index of a tile on texture atlas (by X and Y)
//        cellIdx - index of a map cell in world (by X and Y)
//---------------------------------------------------------
void UI_FacadeEngine::SetMapTile(Vec2i tileIdx, Vec2i cellIdx)
{
    pGame_->GetMap()->SetTileTex(tileIdx.x, tileIdx.y, cellIdx.x, cellIdx.y);
}

//---------------------------------------------------------
// create a new map
//---------------------------------------------------------
bool UI_FacadeEngine::CreateNewMap(const char* mapName, int mapCellsX, int mapCellsY)
{
    return pGame_->GetMap()->CreateNewMap(mapName, mapCellsX, mapCellsY);
}

//---------------------------------------------------------
// save map into a file
//---------------------------------------------------------
void UI_FacadeEngine::SaveMap()
{
    pGame_->GetMap()->SaveMap();
}

void UI_FacadeEngine::SaveMapAs(const char* path)
{
    pGame_->GetMap()->SaveMapAs(path);
}

//---------------------------------------------------------
// load map from a file
//---------------------------------------------------------
void UI_FacadeEngine::LoadMap(const char* mapPath)
{
    pGame_->LoadMap(mapPath);
}

//---------------------------------------------------------
// set map's new size (number of cells by X and Y)
//---------------------------------------------------------
void UI_FacadeEngine::ResizeMap(Vec2i numCells)
{
    pGame_->GetMap()->ResizeMap(numCells);
}

//---------------------------------------------------------
// get current map's params
//---------------------------------------------------------
const char* UI_FacadeEngine::GetMapName() const
{
    return pGame_->GetMap()->GetMapName();
}

//---------------------------------------------------------
// get number of map cells in world (by X and Y)
//---------------------------------------------------------
Vec2i UI_FacadeEngine::GetMapNumCells() const
{
    return pGame_->GetMap()->GetNumCells();
}