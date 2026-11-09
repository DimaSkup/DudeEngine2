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
#ifndef UI_FACADE_ENGINE_H
#define UI_FACADE_ENGINE_H

#include "types.h"
#include "vec.h"
#include "cvector.h"

//---------------------------------------------------------
// forward declarations (pointer use only)
//---------------------------------------------------------
class Game;
class Entity;
struct Texture;

//---------------------------------------------------------
// class name:  UI_FacadeEngine
//---------------------------------------------------------
class UI_FacadeEngine
{
public:
    UI_FacadeEngine(Game* pGame);

    // editor/debugging
    void         SelectEntt(const EntityID id);
    void         ShowEditorCursor(const bool showHide);
    Entity*      GetEditorCursor();
    void         UpdateEditorCursorTransform(const Vec2 pos, 
                                             const Vec2i localSize, 
                                             const int scale);

    // game
    const char*  GetGameLevelName() const;
    uint         GetNumRenderItems(const eLayerType layer) const;
    void         CreateLevel(const char* levelName);
    void         SaveLevel();
    void         LoadLevel(const char* levelName);

    // events
    const char*  GetEventNameById(const EventID id) const;
    EventID      GetEventIdByName(const char* eventName) const;

    const cvector<EventID>&   GetAllEventsIds() const;
    const cvector<EventName>& GetAllEventsNames() const;

    // entity
    EntityID     GetEnttId(const char* enttName) const;
    void         SetShowEntt(const EntityID id, const bool showHide);
    Color        GetEnttColor(const EntityID id) const;
    void         SetEnttColor(const EntityID id, const Color c);

    void         GetEnttTransform(const EntityID id, Vec2& pos, Vec2i& sz, int& scale) const;
    Vec2         GetEnttCenter(const EntityID id) const;

    void         SetEnttPos(const EntityID id, const Vec2 pos);
    void         SetEnttVel(const EntityID id, const Vec2 vel);
    void         SetEnttLocalSize(const EntityID id, const Vec2i sz);
    void         SetEnttScale(const EntityID id, const int scale);

    Vec2         GetEnttPos(const EntityID id) const;
    Vec2i        GetEnttLocalSize(const EntityID id) const;
    int          GetEnttScale(const EntityID id) const;

    void         SetEnttVelocity(const EntityID id, const Vec2 vel);
    void         GetEnttVelocity(const EntityID id, Vec2& vel);

    const cvector<Entity*>& GetEnttsByLayer(eLayerType layer) const;
    bool         IsShowLayer(eLayerType layer) const;
    void         SetShowLayer(eLayerType layer, bool showHide);

    // text / locales
    const char*  GetCurrLocale() const;
    const char** GetAvailLocales() const;
    size_t       GetNumAvailLocales() const;
    void         SetLocale(const char* loc) const;

    // camera
    void         FocusCameraOnEntt(const EntityID id);


    // textures functional
    TexID        LoadTexture(const char* textureName, const char* texturePath);
    const char*  GetTexName(const TexID id) const;

    TexID        GetTexId(const char* texName) const;
    TexID        GetMapTexId() const;

    Texture*     GetTexTilemap() const;
    Texture*     GetTex(const TexID id) const;
    Vec2i        GetTexNumTiles(const TexID id) const;
    Vec2i        GetTexSize(const TexID id) const;


    // map functional
    const char*  GetMapName() const;
    void         SetMapTexture(const TexID texId);
    void         SetMapTile(Vec2i tileIdx, Vec2i cellIdx);
    bool         CreateNewMap(const char* mapName, int mapCellsX, int mapCellsY);
    void         SaveMap();
    void         SaveMapAs(const char* path);
    void         LoadMap(const char* mapPath);
    void         ResizeMap(Vec2i numCells);
    Vec2i        GetMapNumCells() const;

private:
    Game*   pGame_;
};

//---------------------------------------------------------
// global instance
//---------------------------------------------------------
extern UI_FacadeEngine* g_pFacadeEngine;

#endif