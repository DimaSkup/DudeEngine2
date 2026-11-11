/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   ui_editor.cpp
    Desc:       editor's main class
    Created:    13.05.2026 by DimaSkup
\**********************************************************************************/
#include "ui_editor.h"
#include "mem_helpers.h"
#include "math_helpers.h"
#include "ui_facade_engine.h"
#include "ui_editor_tilemap.h"
#include "ui_editor_entts.h"
#include "ui_noise_gen.h"
#include "game_state.h"
#include "entity.h"
#include "texture.h"
#include "collision.h"
#include "file_sys.h"

#include "ui_ed_state.h"

#include <assert.h>
#include <imgui.h>
#include <imgui_internal.h>


//---------------------------------------------------------
// default constructor
//---------------------------------------------------------
UI_Editor::UI_Editor()
{
    memset(&g_EditorState, 0, sizeof(g_EditorState));

    g_EditorState.bShowEditorCursor = true;
    g_EditorState.bGrabbingEntt     = false;
    g_EditorState.bNoClipMode       = false;

    pTilemapEd_ = new UI_EditorTilemap();
    pEnttsEd_   = new UI_EditorEntts();
    pNoiseGen_  = new UI_NoiseGen();
}

//---------------------------------------------------------
// destructor
//---------------------------------------------------------
UI_Editor::~UI_Editor()
{
    SafeDelete(pTilemapEd_);
    SafeDelete(pEnttsEd_);
    SafeDelete(pNoiseGen_);
}

//---------------------------------------------------------
// render editor panels onto the screen
//---------------------------------------------------------
void UI_Editor::Render(Texture* pSceneViewport)
{
    RenderViewport(pSceneViewport);
    RenderEdPanels();
}

//---------------------------------------------------------
// render the game viewport (so we will see the scene)
//---------------------------------------------------------
void UI_Editor::RenderViewport(Texture* pSceneViewport)
{
    assert(pSceneViewport);

    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar);

    const ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImVec2       winMin = window->ContentRegionRect.Min;
    const ImVec2       winMax = window->ContentRegionRect.Max;
    const ImVec2        winSz = ImGui::GetWindowSize();

    // save viewport params for later use in editors
    g_GameState.viewportTopLeft = { winMin.x, winMin.y };
    g_GameState.viewportSize    = { winSz.x, winSz.y };

    // is viewport hovered by mouse cursor?
    const Rect winRect = { winMin.x, winMax.x, winMin.y, winMax.y };
    const Vec2   mouse = ToVec2(g_GameState.mousePos);
    g_EditorState.bSceneViewportHovered = CollisionRectPoint(winRect, mouse);

    // render viewport image
    const ImVec2 imgSz = ImVec2(pSceneViewport->w, pSceneViewport->h);
    ImGui::Image((ImTextureID)pSceneViewport->pTex, imgSz);

    ImGui::End();
}

//---------------------------------------------------------
// render a window with editor's panels
//---------------------------------------------------------
void UI_Editor::RenderEdPanels()
{
    ImGui::Begin("Editor");

    if (ImGui::BeginTabBar("EditorTabs", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Scene"))
        {
            g_EditorState.activeEditor = EDITOR_SCENE;
            DrawEnttsSceneHierarchy();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Debug"))
        {
            g_EditorState.activeEditor = EDITOR_DEBUG;
            DrawDebugEd();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Tilemap"))
        {
            g_EditorState.activeEditor = EDITOR_TILEMAP;
            pTilemapEd_->Render();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Prefabs"))
        {
            g_EditorState.activeEditor = EDITOR_PREFABS;
            pEnttsEd_->RenderPrefabs();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Entity"))
        {
            g_EditorState.activeEditor = EDITOR_ENTITIES;
            pEnttsEd_->RenderEnttData();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Noise"))
        {
            g_EditorState.activeEditor = EDITOR_NOISE;
            pNoiseGen_->Draw();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

//---------------------------------------------------------
// render debug control fields
//---------------------------------------------------------
void UI_Editor::DrawDebugEd()
{
    if (ImGui::BeginChild("Debug"))
    {
        assert(g_pFacadeEngine);

        DrawCommonInfo();
        DrawPlayerInfo();
  
        if (ImGui::TreeNode("Level"))
        {
            DrawModalWndCreateLevel();
            DrawModalWndLoadLevel();

            ImGui::TreePop();
            ImGui::Separator();
        }

        DrawFogControl();
        DrawCameraControl();
        DrawRenderingControl();
        DrawLocalesControl();
    }
    ImGui::EndChild();
}

//---------------------------------------------------------
// enable/disable rendering of the editor cursor (a red box for selection highlighting)
//---------------------------------------------------------
void UI_Editor::SetShowEditorCursor(bool showHide)
{
    g_pFacadeEngine->ShowEditorCursor(showHide);
}

//---------------------------------------------------------
// change editor's selection of entity 
//---------------------------------------------------------
void UI_Editor::SetSelectedEntt(const EntityID id)
{
    g_pFacadeEngine->SelectEntt(id);
}

//---------------------------------------------------------
// is any entity currently selected (picked) in the editor?
//---------------------------------------------------------
bool UI_Editor::IsAnyEnttSelected() const
{
    return g_EditorState.selectedEnttId != INVALID_ENTT_ID;
}

//---------------------------------------------------------
// Desc:  get an ID of entity which is selected in the editor
// NOTE:  if 0 is returned it means that no entity is selected now
//---------------------------------------------------------
EntityID UI_Editor::GetSelectedEnttId() const
{
    return g_EditorState.selectedEnttId;
}

//---------------------------------------------------------
// get a ptr to the currently selected prefab
//---------------------------------------------------------
const Entity* UI_Editor::GetSelectedPrefab() const
{
    return pEnttsEd_->GetSelectedPrefab();
}

//---------------------------------------------------------
// get a ptr to "editor cursor" (a red square used for selection highlighting)
//---------------------------------------------------------
Entity* UI_Editor::GetCursor()
{
    return g_pFacadeEngine->GetEditorCursor();
}

//---------------------------------------------------------
// return a type of the currently active editor (tilemap editor, entts editor, etc.)
//---------------------------------------------------------
eEditorType UI_Editor::GetActiveEditor() const
{
    return (eEditorType)g_EditorState.activeEditor;
}

bool UI_Editor::IsShowEdCursor() const
{
    return g_EditorState.bShowEditorCursor;
}

//---------------------------------------------------------
// enable/disable moving the selected entity using mouse
//---------------------------------------------------------
void UI_Editor::SetGrabbingEntt(const bool onOff)
{
    g_EditorState.bGrabbingEntt = onOff;
}

//---------------------------------------------------------
// Desc:  change a position of the editors cursor
//---------------------------------------------------------
void UI_Editor::SetEditorCursorPos(Vec2 p)
{
    g_pFacadeEngine->SetEnttPos(GetCursor()->GetId(), p);
}

//---------------------------------------------------------

void UI_Editor::SetEditorCursorPos(int px, int py)
{
    SetEditorCursorPos({ (float)px, (float)py });
}

//---------------------------------------------------------
// is the scene viewport currently hovered by mouse?
//---------------------------------------------------------
bool UI_Editor::IsSceneViewportHovered() const
{
    return g_EditorState.bSceneViewportHovered;
}

//---------------------------------------------------------
// enable/disable camera's free fly
//---------------------------------------------------------
void UI_Editor::ToggleCameraNoClip() const
{
    g_EditorState.bNoClipMode = ~g_EditorState.bNoClipMode;
}

//---------------------------------------------------------
// are we in the "no clip" (camera free fly) mode?
//---------------------------------------------------------
bool UI_Editor::IsCameraNoClip() const
{
    return g_EditorState.bNoClipMode;
}

//---------------------------------------------------------
// get the only layer which is allowed for selection
//---------------------------------------------------------
eLayerType UI_Editor::GetSelectableLayer() const
{
    return pEnttsEd_->GetSelectableLayer();
}

//---------------------------------------------------------
// return true if any modal window is currently opened in the editor
//---------------------------------------------------------
bool UI_Editor::IsAnyModalWndOpened() const
{
    return g_EditorState.bOpenedModalWnd;
}

//---------------------------------------------------------
// Desc:  1. render the editor cursor around another map cell according to input pos
//        2. change a tile on the map texture for map cell
//---------------------------------------------------------
void UI_Editor::ModifyMapTile(Vec2i pos)
{
    const Vec2i numCells = g_pFacadeEngine->GetMapNumCells();

    Vec2i cellIdx;
    cellIdx.x = Clamp(pos.x / MAP_CELL_SIZE, 0, numCells.x - 1);
    cellIdx.y = Clamp(pos.y / MAP_CELL_SIZE, 0, numCells.y - 1);

    // setup a tile (texture) for map's cell
    const Vec2i tileIdx = pTilemapEd_->GetSelectedTileIdx();
    g_pFacadeEngine->SetMapTile(tileIdx, cellIdx);
}

//---------------------------------------------------------
// draw engine/game/world state
//---------------------------------------------------------
void UI_Editor::DrawCommonInfo()
{
    GameState&    s = g_GameState;
    const float fps = ImGui::GetIO().Framerate;

    ImGui::Text("Level: %s", g_pFacadeEngine->GetGameLevelName());
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Frame time: %.3f ms", 1000.0f / fps);

    ImGui::Separator();

    ImGui::Text("Render stats:");
    for (int i = 0; i < NUM_LAYERS; ++i)
    {
        const uint numRenderedItems = g_pFacadeEngine->GetNumRenderItems(eLayerType(i));
        ImGui::Text("Rendered %s elems: %u", s_LayerNames[i], numRenderedItems);
    }

    ImGui::Text("Window size: %dx%d",      s.wndSize.w,    s.wndSize.y);
    ImGui::Text("Mouse window pos: %d:%d", s.mousePos.w,   s.mousePos.y);
    ImGui::Text("Mouse world pos: %d:%d",  s.mousePosW.x,  s.mousePosW.y);
    ImGui::Text("Camera world pos: %d:%d", s.camPos.x,     s.camPos.y);
    ImGui::Text("World size: %d:%d",       s.iWorldSize.w, s.iWorldSize.h);

    ImGui::Separator();
}

//---------------------------------------------------------
// draw field for controlling player related props
//---------------------------------------------------------
void UI_Editor::DrawPlayerInfo()
{
    if (ImGui::TreeNode("Player Info"))
    {
        GameState& s = g_GameState;

        ImGui::Text    ("Player pos: %d:%d",          s.iPlayerPos.x, s.iPlayerPos.y);
        ImGui::Checkbox("Player invisible to enemy", &s.bPlayerInvisibleToEnemy);
        ImGui::Checkbox("Player is immortal",        &s.bPlayerImmortal);

        ImGui::TreePop();
        ImGui::Separator();
    }
}

//---------------------------------------------------------
// draw editor's fields to modify the fog state
//---------------------------------------------------------
void UI_Editor::DrawFogControl()
{
    if (ImGui::TreeNode("Fog"))
    {
        const EntityID fogId = g_pFacadeEngine->GetEnttId("fog");

        if (fogId == INVALID_ENTT_ID)
            return;

        ImGui::Checkbox("Show fog", &g_GameState.bShowFog);

        // setup fog color
        Color c = g_pFacadeEngine->GetEnttColor(fogId);
        const ImVec4 fogColor = ImGui::ColorConvertU32ToFloat4(c.rgba);

        if (ImGui::ColorEdit4("Color", (float*)&fogColor))
        {
            c.rgba = ImGui::ColorConvertFloat4ToU32(fogColor);
            g_pFacadeEngine->SetEnttColor(fogId, c);
        }

        // setup position, size, scale
        Vec2 pos = { 0,0 };
        Vec2i sz = { 0,0 };
        Vec2 vel = { 0,0 };
        int scale = 0;

        g_pFacadeEngine->GetEnttTransform(fogId, pos, sz, scale);
        g_pFacadeEngine->GetEnttVelocity(fogId, vel);

        if (ImGui::DragFloat2("position", &pos.x) && pos.x > 0 && pos.y > 0)
            g_pFacadeEngine->SetEnttPos(fogId, pos);

        if (ImGui::DragFloat2("velocity", &vel.x))
            g_pFacadeEngine->SetEnttVelocity(fogId, vel);

        if (ImGui::DragInt2("size", &sz.w) && sz.w > 0 && sz.h > 0)
            g_pFacadeEngine->SetEnttLocalSize(fogId, sz);

        if (ImGui::DragInt("scale", &scale) && scale > 0)
            g_pFacadeEngine->SetEnttScale(fogId, scale);

        ImGui::TreePop();
        ImGui::Separator();
    }
}

//---------------------------------------------------------
// draw fields for setup camera's properties
//---------------------------------------------------------
void UI_Editor::DrawCameraControl() 
{
    GameState& s = g_GameState;

    if (ImGui::TreeNode("Camera"))
    {

        ImGui::Text("Camera pos: %d:%d",           s.camPos.x, s.camPos.y);
        ImGui::Text("Camera size: %d:%d",          s.camSize.w, s.camSize.h);

        ImGui::DragInt2("Camera padding:",        &s.camPadding.x);
        ImGui::DragFloat("Shake increment",       &s.camShakeInc, 0.01f);
        ImGui::DragFloat("Shake max angle",       &s.camShakeMaxAngle, 0.01f);
        ImGui::DragFloat("Shake max offset",      &s.camShakeMaxOffset, 0.01f);
        ImGui::DragFloat("Asymptotic avg weight", &s.camAsymptoticAvgWeight, 0.01f);

        ImGui::Checkbox("Shaking",                &s.bUseCamShaking);
        ImGui::Checkbox("Smooth move",            &s.bUseSmoothCamMove);
        
        ImGui::TreePop();
        ImGui::Separator();
    }
}

//---------------------------------------------------------
// enable/disable rendering of layers and other stuff
//---------------------------------------------------------
void UI_Editor::DrawRenderingControl() 
{
    if (ImGui::TreeNode("Rendering layers and layout"))
    {
        bool bShowEdCursor = g_EditorState.bShowEditorCursor;

        if (ImGui::Checkbox("Show cursor", &bShowEdCursor))
            SetShowEditorCursor(bShowEdCursor);

        ImGui::Checkbox("Show colliders",    &g_GameState.bShowColliders);
        ImGui::Checkbox("Show triggers",     &g_GameState.bShowTriggers);
        ImGui::Checkbox("Show screen grid",  &g_GameState.bShowScreenDbgGrid);
        ImGui::Checkbox("Show attack range", &g_GameState.bShowAttackRange);

        for (int i = 0; i < NUM_LAYERS; ++i)
        {
            char label[32];
            bool bShow = g_pFacadeEngine->IsShowLayer(eLayerType(i));

            snprintf(label, sizeof(label), "Show layer: %s", s_LayerNames[i]);

            if (ImGui::Checkbox(label, &bShow))
                g_pFacadeEngine->SetShowLayer(eLayerType(i), bShow);
        }

        ImGui::TreePop();
        ImGui::Separator();
    }
}

//---------------------------------------------------------
// selectable list of available locales
//---------------------------------------------------------
void UI_Editor::DrawLocalesControl() 
{
    if (ImGui::TreeNode("Locales"))
    {
        const char*  currLocale      = g_pFacadeEngine->GetCurrLocale();
        const char** availLocales    = g_pFacadeEngine->GetAvailLocales();
        const size_t numAvailLocales = g_pFacadeEngine->GetNumAvailLocales();
            
        for (size_t i = 0; i < numAvailLocales; ++i)
        {
            const char* locale = availLocales[i];
            bool     bSelected = (strcmp(locale, currLocale) == 0);

            if (ImGui::Selectable(locale, &bSelected))
            {
                // set only if differs
                if (strcmp(locale, currLocale) != 0)
                {
                    g_pFacadeEngine->SetLocale(availLocales[i]);
                }
            }
        }

        ImGui::TreePop();
        ImGui::Separator();
    }
}

//---------------------------------------------------------
// open a modal window for creation a new empty level
//---------------------------------------------------------
void UI_Editor::DrawModalWndCreateLevel()
{
    if (ImGui::Button("Create new level"))
        ImGui::OpenPopup("create_new_level");

    // always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    // open a MODAL window
    if (ImGui::BeginPopupModal("create_new_level", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char levelName[32]{ '\0' };
        
        ImGui::InputText("new level name", levelName, sizeof(levelName));

        if (ImGui::Button("Create"))
        {
            if (levelName[0] != '\0')
            {
                g_pFacadeEngine->SaveLevel();
                g_pFacadeEngine->CreateLevel(levelName);
                levelName[0] = '\0';
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            levelName[0] = '\0';
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

//---------------------------------------------------------
// open modal window for loading another game level
//---------------------------------------------------------
void UI_Editor::DrawModalWndLoadLevel()
{
    if (ImGui::Button("Load level"))
        ImGui::OpenPopup("load_level");

    // always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    // open a MODAL window
    if (ImGui::BeginPopupModal("load_level", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Select a level to load");

        static int itemSelectedIdx = -1;
        static bool bShowErrMsg = false;
        int itemIdx = 0;
        char dirName[64];
        char selectedLevel[64];
        dirName[0] = '\0';
        selectedLevel[0] = '\0';


        if (bShowErrMsg)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4({ 1.0f, 0.0f, 0.0f, 1.0f })));
            ImGui::Text("ERROR: You must select a level first!");
            ImGui::PopStyleColor();
        }

        //
        // render a list of levels to select
        //
        if (ImGui::BeginListBox("##game_levels_list"))
        {
            FileSys::OpenDir(PATH_LEVELS_DIR);

            while (FileSys::GetNextDirName(dirName))
            {
                if (dirName[0] == '\0' || dirName[0] == '.')
                    continue;

                // level's directory name is the same as level's name by itself
                const bool bSelected = (itemSelectedIdx == itemIdx);

                if (ImGui::Selectable(dirName, bSelected))
                    itemSelectedIdx = itemIdx;

                if (bSelected)
                {
                    ImGui::SetItemDefaultFocus();
                    strncpy(selectedLevel, dirName, sizeof(selectedLevel));
                }

                ++itemIdx;
            }

            FileSys::CloseDir();
            ImGui::EndListBox();
        }
        
        // load selected level
        if (ImGui::Button("Load"))
        {
            if (selectedLevel[0] != '\0')
            {
                itemSelectedIdx = -1;
                bShowErrMsg = false;
                g_pFacadeEngine->LoadLevel(selectedLevel);
                ImGui::CloseCurrentPopup();
            }
            else
            {
                bShowErrMsg = true;
            }
        }

        ImGui::SameLine();

        // cancel selection, closes modal window
        if (ImGui::Button("Cancel"))
        {
            itemSelectedIdx = -1;
            bShowErrMsg = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

//---------------------------------------------------------
// render lists of entities split by layers
//---------------------------------------------------------
void UI_Editor::DrawEnttsSceneHierarchy()
{
    if (ImGui::BeginChild("SceneHierarchy"))
    {
        for (int i = 0; i < NUM_LAYERS; ++i)
        {
            if (ImGui::CollapsingHeader(s_LayerNames[i]))
            {
                const cvector<Entity*>& entts = g_pFacadeEngine->GetEnttsByLayer(eLayerType(i));

                // tilemaps we setup separately
                if (i != LAYER_TILEMAP)
                {
                    DrawEnttsSelectableList(entts.data(), (uint)entts.size());
                }
                else
                {
                    // just text list
                    for (const Entity* pEntt : entts)
                        ImGui::Text(pEntt->GetName());
                }
            }
        }
    }
    ImGui::EndChild();
}

//---------------------------------------------------------
// draw entities selectable list;
// when you click on list element we focus the camera on related entity in the editor
//---------------------------------------------------------
void UI_Editor::DrawEnttsSelectableList(Entity** entts, const uint numEntts)
{
    if (numEntts == 0)
    {
        ImGui::Text("no entity on the scene belongs to this layer, yet");
        return;
    }

    for (index i = 0; i < numEntts; ++i)
    {
        bool bSelected = (entts[i]->GetId() == g_EditorState.selectedEnttId);

        if (ImGui::Selectable(entts[i]->GetName(), &bSelected))
        {
            g_pFacadeEngine->SelectEntt(entts[i]->GetId());
            g_pFacadeEngine->FocusCameraOnEntt(entts[i]->GetId());
         
            // always switch to noclip when select any entt in the list
            g_EditorState.bNoClipMode = true;
        }
    }
}
