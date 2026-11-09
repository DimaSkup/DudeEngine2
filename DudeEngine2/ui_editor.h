/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   ui_editor.h
    Desc:       editor's main class
    Created:    13.05.2026 by DimaSkup
\**********************************************************************************/
#ifndef UI_EDITOR_H
#define UI_EDITOR_H

#include "types.h"
#include "vec.h"

//---------------------------------------------------------
// forward declarations (pointer use only)
//---------------------------------------------------------
class UI_EditorTilemap;
class UI_EditorEntts;
class UI_NoiseGen;
class Entity;
struct Texture;

//---------------------------------------------------------
// enumeration of editors types
//---------------------------------------------------------
enum eEditorType
{
    EDITOR_SCENE,
    EDITOR_DEBUG,
    EDITOR_TILEMAP,
    EDITOR_PREFABS,
    EDITOR_ENTITIES,
    EDITOR_NOISE,

    NUM_EDITOR_TYPES,
};

//---------------------------------------------------------
// class name:  UI_Editor
//---------------------------------------------------------
class UI_Editor
{
public:
    UI_Editor();
    ~UI_Editor();

    void          Render(Texture* pSceneViewport);
    void          RenderViewport(Texture* pSceneViewport);
    void          RenderEdPanels();
                  
    bool          IsShowEdCursor() const;
    eLayerType    GetSelectableLayer() const;
                  
    void          SetSelectedEntt(const EntityID id);
    void          SetShowEditorCursor(bool showHide);

    bool          IsAnyEnttSelected() const;
    EntityID      GetSelectedEnttId() const;
    const Entity* GetSelectedPrefab() const;

    Entity*       GetCursor();
    void          SetEditorCursorPos(Vec2 p);
    void          SetEditorCursorPos(int px, int py);
    void          ModifyMapTile(Vec2i mouseWorldPos);

    eEditorType   GetActiveEditor() const;
    void          SetGrabbingEntt(const bool onOff);

    bool          IsSceneViewportHovered() const;
    void          ToggleCameraNoClip() const;
    bool          IsCameraNoClip() const;

    bool          IsAnyModalWndOpened() const;

private:
    void          DrawDebugEd();
                  
    void          DrawModalWndCreateLevel();
    void          DrawModalWndLoadLevel();
                  
    void          DrawCommonInfo();
    void          DrawPlayerInfo();
    void          DrawFogControl();
    void          DrawCameraControl();
    void          DrawRenderingControl();
    void          DrawLocalesControl();
                  
    void          DrawEnttsSceneHierarchy();
    void          DrawEnttsSelectableList(Entity** entts, const uint numEntts);

private:
    UI_EditorTilemap* pTilemapEd_;
    UI_EditorEntts*   pEnttsEd_;
    UI_NoiseGen*      pNoiseGen_;
};

#endif

