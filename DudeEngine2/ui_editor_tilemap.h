/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   ui_editor_tilemap.h
    Desc:       functional for creation and editing of the maps (backgrouns of levels)
    Created:    13.05.2026 by DimaSkup
\**********************************************************************************/
#ifndef UI_EDITOR_TILEMAP_H
#define UI_EDITOR_TILEMAP_H

#include "vec.h"

class UI_EditorTilemap
{
public:
    UI_EditorTilemap();

    void Render();

    inline Vec2i GetSelectedTileIdx() const
    {
        return { selectedItemIdx_ % numTilesCols_, selectedItemIdx_ / numTilesCols_ };
    }

private:
    void UpdateEdLayout(float availWidth);
    void SaveMapAs();
    void LoadMap();
    void ChangeTilemap();

    void DrawIcons(float startPosX, float startPosY);
    void DrawWndCreateMap();
    void DrawWndResizeMap();
    void PrintMapInfo();

    void ComputeIconsZooming(float startPosX, float startPosY, float availWidth);

private:
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
};

#endif
