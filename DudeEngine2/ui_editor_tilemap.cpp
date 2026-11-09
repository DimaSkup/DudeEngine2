/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   ui_editor_tilemap.cpp
    Desc:       functional for creation and editing of the maps (backgrouns of levels)
    Created:    13.05.2026 by DimaSkup
\**********************************************************************************/
#include "ui_editor_tilemap.h"
#include "math_helpers.h"
#include "ui_facade_engine.h"
#include "ui_ed_state.h"
#include "file_sys.h"
#include "log.h"
#include "game_state.h"
#include "texture.h"
#include "macros.h"

#include <tinyfiledialogs/tinyfiledialogs.h>  // for using OS filesystem dialog window
#include <stdio.h>
#include <imgui.h>
#include <math.h>



//---------------------------------------------------------
// default construct
//---------------------------------------------------------
UI_EditorTilemap::UI_EditorTilemap()
{
}

//---------------------------------------------------------
// draw tilemap editor onto the screen
//---------------------------------------------------------
void UI_EditorTilemap::Render()
{
    assert(g_pFacadeEngine);

    const ImGuiChildFlags childFlags = ImGuiChildFlags_None;
    const ImGuiWindowFlags  wndFlags = ImGuiWindowFlags_NoMove;
    
    if (ImGui::BeginChild("TilemapEditor", ImVec2(0, 0), childFlags, wndFlags))
    {
        //
        // draw buttons for manipulations with a map
        //
        DrawWndCreateMap();

        if (ImGui::Button("Save map"))
            g_pFacadeEngine->SaveMap();

        if (ImGui::Button("Save map as..."))
            SaveMapAs();

        if (ImGui::Button("Load map"))
            LoadMap();
        
        if (ImGui::Button("Change tilemap"))
            ChangeTilemap();

        DrawWndResizeMap();
        PrintMapInfo();

        //---------------------------------------

        // update the number of tiles icons
        const TexID tilemapId = g_pFacadeEngine->GetMapTexId();
        const Vec2i  numTiles = g_pFacadeEngine->GetTexNumTiles(tilemapId);

        if (numTilesCols_ != numTiles.x ||
            numTilesRows_ != numTiles.y)
        {
            numTilesCols_    = numTiles.x;
            numTilesRows_    = numTiles.y;
            numItems_        = numTiles.x * numTiles.y;
            bUpdateEdLayout_ = true;
        }

        const float availWidth = ImGui::GetContentRegionAvail().x;

        // if we changed the width of the editor's window
        if (prevAvailWidth_ != availWidth)
        {
            bUpdateEdLayout_ = true;
            prevAvailWidth_ = availWidth;
        }

        if (bUpdateEdLayout_)
        {
            bUpdateEdLayout_ = false;
            UpdateEdLayout(availWidth);
        }

        // setup start position for rendering
        ImVec2 startPos = ImGui::GetCursorScreenPos();
        ImVec2 padding = { layoutOuterPadding_, layoutOuterPadding_ };

        startPos += padding;
        ImGui::SetCursorScreenPos(startPos);

        // Submit a 0x0 item to "claim" this space and extend the window boundaries
        ImGui::Dummy(ImVec2(0, 0));

        DrawIcons(startPos.x, startPos.y);
        ComputeIconsZooming(startPos.x, startPos.y, availWidth);
    }
    ImGui::EndChild();
}

//---------------------------------------------------------
// update layout of the tilemaps editor when we modify window size
//---------------------------------------------------------
void UI_EditorTilemap::UpdateEdLayout(float availWidth)
{
    layoutItemSpacing_ = (float)iconSpacing_;

    // when not stretching: allow extending into right-most spacing
    if (bStretchSpacing_ == false)
        availWidth += floorf(layoutItemSpacing_ * 0.5f);

    layoutItemSizeX_ = floorf(iconSize_);
    layoutItemSizeY_ = layoutItemSizeX_;

    // calc number of icons rows and columns
    numLayoutCols_ = Max((int)(availWidth / (layoutItemSizeX_ + layoutItemSpacing_)), 1);
    numLayoutRows_ = (numItems_ + numLayoutCols_ - 1) / numLayoutCols_;

    // when stretching: allocate remaining space to more spacing. Round before division, so item spacing may be non-integer
    if (bStretchSpacing_ && (numLayoutCols_ > 1))
        layoutItemSpacing_ = floorf(availWidth - layoutItemSizeX_ * numLayoutCols_) / numLayoutCols_;

    layoutItemStepX_         = layoutItemSizeX_ + layoutItemSpacing_;
    layoutItemStepY_         = layoutItemSizeY_ + layoutItemSpacing_;
    layoutSelectableSpacing_ = Max(floorf(layoutItemSpacing_) - iconHitSpacing_, 0.0f);
    layoutOuterPadding_      = floorf(layoutItemSpacing_ * 0.5f);
}

//---------------------------------------------------------
// draw a grid of tiles texture icons
// (each icon represents a part of a tilemap / atlas)
//---------------------------------------------------------
void UI_EditorTilemap::DrawIcons(float startPosX, float startPosY)
{
    ImGui::PushStyleColor(ImGuiCol_Header, { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 1,1,1,1 });

    const int     numCols = numLayoutCols_;
    const ImVec2 iconSize = { iconSize_, iconSize_ };
    const ImVec2 itemSize = { layoutItemSizeX_, layoutItemSizeY_ };

    const float du = 1.0f / numTilesCols_;
    const float dv = 1.0f / numTilesRows_;

    // current texture atlas for the map
    const Texture* pTexTilemap = g_pFacadeEngine->GetTexTilemap();


    ImGuiListClipper clipper;
    clipper.Begin(numLayoutRows_, layoutItemStepY_);

    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
        {
            // calc min/max idxs for the current row of icons
            const int itemMinIdx = row * numCols;
            const int itemMaxIdx = Min(itemMinIdx + numCols, numItems_);

            // render a row of tiles icons
            for (int itemIdx = itemMinIdx; itemIdx < itemMaxIdx; ++itemIdx)
            {
                ImGui::PushID(itemIdx);

                // position of current item
                const float itemOffsetX = (itemIdx % numCols) * layoutItemStepX_;
                const float itemOffsetY = row * layoutItemStepY_;
                const ImVec2    iconPos = { startPosX + itemOffsetX, startPosY + itemOffsetY };

                ImGui::SetCursorScreenPos(iconPos);
                ImGui::SetNextItemSelectionUserData(itemIdx);

                // check if we see a texture icon
                if (!ImGui::IsRectVisible(itemSize))
                {
                    ImGui::PopID();
                    continue;
                }

                //
                // render icon
                //
                char buf[32];
                snprintf(buf, sizeof(buf), "##tex_item %d", itemIdx);

                bool bChooseAnother = false;
                bool bSelected = (itemIdx == selectedItemIdx_);

                // we can select a texture when click on some icon
                // ALSO: render a border around this icon
                if (ImGui::Selectable(buf, bSelected, ImGuiSelectableFlags_None, itemSize))
                {
                    selectedItemIdx_ = itemIdx;
                }

                // draw background (so we will have a border around the icon)
                const ImVec2 boxMin = { iconPos.x - 1, iconPos.y - 1 };
                const ImVec2 boxMax = { boxMin.x + itemSize.x + 2, boxMin.y + itemSize.y + 2 };
                const ImU32 bgColor = ImGui::GetColorU32({ 0,0,0,1 });

                ImGui::GetWindowDrawList()->AddRectFilled(boxMin, boxMax, bgColor);

                // draw texture image
                ImGui::SetCursorScreenPos(iconPos);

                int horizIdx = itemIdx % numTilesCols_;
                int verticIdx = itemIdx / numTilesCols_;

                ImVec2 uv0 = { du * horizIdx , dv * verticIdx };
                ImVec2 uv1 = { du + uv0.x ,    dv + uv0.y };

                // clamp texture coords (to prevent ImGui fucking up)
                uv0.x = clampf(uv0.x, 0, 1);
                uv1.x = clampf(uv1.x, 0, 1);
                uv0.y = clampf(uv0.y, 0, 1);
                uv1.y = clampf(uv1.y, 0, 1);

                // draw icon
                ImGui::Image((ImTextureID)pTexTilemap->pTex, iconSize, uv0, uv1);

                // draw label (just an index)
                const ImVec2 labelPos = { boxMin.x, boxMax.y - ImGui::GetFontSize() };
                const ImU32 labelColor = ImGui::GetColorU32(ImGuiCol_Text);

                char label[16];
                snprintf(label, sizeof(label), "%d", itemIdx);
                ImGui::GetWindowDrawList()->AddText(labelPos, labelColor, label);

                ImGui::PopID();
            }
        }
    }

    clipper.End();
    ImGui::PopStyleColor(3);
}

//---------------------------------------------------------
// Desc:  if we zoom in/out (Ctrl+mouse_when when over the editor),
//        we need to recompute icons sizes and related stuff
// Args:  startPos   - cursor screen position (for ImGui drawing)
//        availWidth - width of the editor window
//---------------------------------------------------------
void UI_EditorTilemap::ComputeIconsZooming(
    float startPosX, 
    float startPosY, 
    float availWidth)
{
    ImGuiIO& io = ImGui::GetIO();

    // zooming with CTRL+Wheel
    if (ImGui::IsWindowAppearing())
        zoomWheelAccum_ = 0.0f;

    // if we need handle zooming
    bool bZoomingActivated = 
        ImGui::IsWindowHovered() &&
        io.MouseWheel != 0.0f &&
        ImGui::IsKeyDown(ImGuiMod_Ctrl) &&
        ImGui::IsAnyItemActive() == false;

    if (!bZoomingActivated)
        return;
    
    zoomWheelAccum_ += io.MouseWheel;

    if (fabsf(zoomWheelAccum_) < 1.0f)
        return;

    // calculate hovered item index from mouse location
    const float hoveredItemNX = (io.MousePos.x - startPosX + layoutItemSpacing_ * 0.5f) / layoutItemStepX_;
    const float hoveredItemNY = (io.MousePos.y - startPosY + layoutItemSpacing_ * 0.5f) / layoutItemStepY_;
    const int  hoveredItemIdx = ((int)hoveredItemNY * numLayoutCols_) + (int)hoveredItemNX;

    // zoom
    iconSize_ *= powf(1.1f, zoomWheelAccum_);
    iconSize_ = clampf(iconSize_, 16.0f, 512.0f);
    zoomWheelAccum_ -= (int)zoomWheelAccum_;
    UpdateEdLayout(availWidth);

    // manipulate scroll so that we will land at the same Y location of currently hovered item
    float hoveredItemRelPosY = ((float)(hoveredItemIdx / numLayoutCols_) + fmodf(hoveredItemNY, 1.0f));
    hoveredItemRelPosY *= layoutItemStepY_;
    hoveredItemRelPosY += ImGui::GetStyle().WindowPadding.y;

    float mouseLocalY = io.MousePos.y - ImGui::GetWindowPos().y;
    ImGui::SetScrollY(hoveredItemRelPosY - mouseLocalY);
}

//---------------------------------------------------------
// render a modal window for setup and creation a new map
//---------------------------------------------------------
void UI_EditorTilemap::DrawWndCreateMap()
{
    if (ImGui::Button("Create map"))
    {
        g_EditorState.bOpenedModalWnd = true;
        ImGui::OpenPopup("Create_map");
    }

    if (!g_EditorState.bOpenedModalWnd)
        return;

    // always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    // open a MODAL window
    if (ImGui::BeginPopupModal("Create_map", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char mapName[MAX_LEN_MAP_NAME]{'\0'};
        static Vec2i numCells = { 0, 0 };

        ImGui::InputText("map name", mapName, sizeof(mapName));
        ImGui::InputInt("cells by x", &numCells.x);
        ImGui::InputInt("cells by y", &numCells.y);

        if (ImGui::Button("Ok"))
        {
            if (!g_pFacadeEngine->CreateNewMap(mapName, numCells.x, numCells.y))
                LogErr(LOG, "can't create a new map");

            // reset fields
            mapName[0] = '\0';
            numCells = { 0,0 };

            g_EditorState.bOpenedModalWnd = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            // reset fields
            mapName[0] = '\0';
            numCells = { 0,0 };

            g_EditorState.bOpenedModalWnd = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

//---------------------------------------------------------
// 1. open dialog window to choose a path to file + filename
// 2. save a map by path
//---------------------------------------------------------
void UI_EditorTilemap::SaveMapAs()
{
    const char* filters[] = { "*.map" };
    const char* basePath = FileSys::GetBasePath();
    char defaultPath[256];

    // prepare a default path to maps dir
    snprintf(defaultPath, sizeof(defaultPath), "%s%s", basePath, pathToMapsDir);
    FileSys::ToBackSlash(defaultPath);

    const char* mapPath = tinyfd_saveFileDialog("Save map as...", 
                                                defaultPath,
                                                ARRAY_SIZE(filters), 
                                                filters, 
                                                NULL);
    if (mapPath)
        g_pFacadeEngine->SaveMapAs(mapPath);
}

//---------------------------------------------------------
// 1. open dialog window to select a file
// 2. load another map from a file
//---------------------------------------------------------
void UI_EditorTilemap::LoadMap()
{
    const char* filters[] = { "*.map" };
    const char* basePath = FileSys::GetBasePath();
    char defaultPath[256];
    int allowMultipleSelects = 0;

    // prepare a default path to maps dir
    snprintf(defaultPath, sizeof(defaultPath), "%s%s", basePath, pathToMapsDir);
    FileSys::ToBackSlash(defaultPath);

    const char* mapPath = tinyfd_openFileDialog("Select map", 
                                                defaultPath, 
                                                ARRAY_SIZE(filters), 
                                                filters, 
                                                "Maps", 
                                                allowMultipleSelects);
    if (mapPath)
        g_pFacadeEngine->LoadMap(mapPath);
}

//---------------------------------------------------------
// 1. open dialog window to select a file
// 2. change a tilemap texture for the current map
//---------------------------------------------------------
void UI_EditorTilemap::ChangeTilemap()
{
    // load a new tilemap texture from the filesystem
    const char* filters[] = { "*.png" };
    const char* basePath = FileSys::GetBasePath();
    char defaultPath[256];

    // prepare a default path to maps dir
    snprintf(defaultPath, sizeof(defaultPath), "%s%s", basePath, pathToImgTilemapsDir);
    FileSys::ToBackSlash(defaultPath);

    const char* fullPath = tinyfd_openFileDialog("Select Image", 
                                                 defaultPath, 
                                                 ARRAY_SIZE(filters), 
                                                 filters, 
                                                 "Image files",
                                                 0);
    if (fullPath)
    {
        // extract file stem to use it as a texture name
        char texName[MAX_LEN_TEX_NAME]{ '\0' };
        FileSys::GetFileStem(fullPath, texName);

        // load texture and set it as tilemap
        const TexID texId = g_pFacadeEngine->LoadTexture(texName, fullPath);
        g_pFacadeEngine->SetMapTexture(texId);
    }
}

//---------------------------------------------------------
// resize the current map (change a number of cells by X and Y)
//---------------------------------------------------------
void UI_EditorTilemap::DrawWndResizeMap()
{
    if (ImGui::Button("Resize map"))
    {
        g_EditorState.bOpenedModalWnd = true;
        ImGui::OpenPopup("Resize_map");
    }

    if (!g_EditorState.bOpenedModalWnd)
        return;


    // always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Resize_map", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static Vec2i numCells = g_pFacadeEngine->GetMapNumCells();

        ImGui::Text("Map name: %s", g_pFacadeEngine->GetMapName());
        ImGui::InputInt("cells by x", &numCells.x);
        ImGui::InputInt("cells by y", &numCells.y);

        if (ImGui::Button("Ok"))
        {
            g_pFacadeEngine->ResizeMap(numCells);           // resize
            numCells = g_pFacadeEngine->GetMapNumCells();   // update fields values

            g_EditorState.bOpenedModalWnd = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            // reset
            numCells = g_pFacadeEngine->GetMapNumCells();
            g_EditorState.bOpenedModalWnd = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

//---------------------------------------------------------
// print some common info about the current map
//---------------------------------------------------------
void UI_EditorTilemap::PrintMapInfo()
{
    const Vec2i numMapCells = g_pFacadeEngine->GetMapNumCells();
    const TexID mapTexId = g_pFacadeEngine->GetMapTexId();

    ImGui::Text("Map info:");
    ImGui::Text("Name: %s", g_pFacadeEngine->GetMapName());
    ImGui::Text("Cells: %dx%d", numMapCells.x, numMapCells.y);
    ImGui::Text("Texture name: %s", g_pFacadeEngine->GetTexName(mapTexId));
}