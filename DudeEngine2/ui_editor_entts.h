/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   ui_editor_entts.h
    Desc:       functional for creation and editing of the prefabs and entities
    Created:    13.05.2026 by DimaSkup
\**********************************************************************************/
#ifndef UI_EDITOR_ENTTS_H
#define UI_EDITOR_ENTTS_H

#include "types.h"
#include "vec.h"

//---------------------------------------------------------
// forward declaration (pointer use only)
//---------------------------------------------------------
class Entity;

//---------------------------------------------------------
// class name:  UI_EditorEntts
//---------------------------------------------------------
class UI_EditorEntts
{
public:
    UI_EditorEntts();

    Entity*    GetSelectedPrefab();
    eLayerType GetSelectableLayer() const;

    void RenderPrefabs();
    void RenderEnttData();

private:
    void PrintEnttInfo();

    void DrawWndSelectPrefab(Vec2 startPos);
    void DrawWndAddComponent();
    bool DrawDropdownSelectTriggerEvent(const char* label,
                                        const char* comboId,
                                        EventID& currEventId);

    // draw properties (components) of selected entity
    void DrawTransformProps(Entity* pEntt);
    void DrawMovementProps(Entity* pEntt);
    void DrawSpriteProps(Entity* pEntt);
    void DrawPlayerProps(Entity* pEntt);
    void DrawEnemyProps(Entity* pEntt);
    void DrawHealthProps(Entity* pEntt);
    void DrawDamageProps(Entity* pEntt);
    void DrawColliderProps(Entity* pEntt);
    void DrawTriggerProps(Entity* pEntt);
    void DrawHierarchyProps(Entity* pEntt);

    void UpdateEdLayout(float availWidth);
    void ComputeIconsZooming(Vec2 startPos, float availWidth);

private:
    // we can select (pick) entities related only to this layer
    eLayerType selectOnlyLayer_ = LAYER_ENTITY; 

    Entity* pSelectedPrefab_ = nullptr;

    float prevAvailWidth_ = 0;

    float iconSize_ = 64;
    float iconSpacing_ = 8;
    float layoutItemSpacing_ = 0;

    float layoutItemSizeX_ = 0;
    float layoutItemSizeY_ = 0;
    float layoutItemStepX_ = 0;
    float layoutItemStepY_ = 0;

    float layoutSelectableSpacing_ = 0;
    float layoutOuterPadding_ = 0;
    float zoomWheelAccum_ = 0;

    int selectedItemIdx_ = 0;
    int numLayoutCols_ = 0;
    int numLayoutRows_ = 0;
    int iconHitSpacing_ = 4;      // Increase hit-spacing if you want to make it possible to clear or box-select from gaps. Some spacing is required to able to amend with Shift+box-select. Value is small in Explorer.

    // tilemap info
    int numItems_ = 0;
    int numTilesCols_ = 0;
    int numTilesRows_ = 0;

    bool bUpdateEdLayout_ = false;
    bool bStretchSpacing_ = false;

    bool bSelectSceneObj_ = true;      // allow selection of scene objects (entities of LAYER_ENTITY)
    bool bSelectGndDecals_ = true;     // allow selection of ground decals (entities of LAYER_GND_DECAL)
};


//*****************************************************************************
// INLINE METHODS
//*****************************************************************************

//---------------------------------------------------------
// return a ptr to selected prefab
//---------------------------------------------------------
inline Entity* UI_EditorEntts::GetSelectedPrefab()
{
    return pSelectedPrefab_;
}

//---------------------------------------------------------
// get the only layer which is allowed for selection
//---------------------------------------------------------
inline eLayerType UI_EditorEntts::GetSelectableLayer() const
{
    return selectOnlyLayer_;
}

#endif
