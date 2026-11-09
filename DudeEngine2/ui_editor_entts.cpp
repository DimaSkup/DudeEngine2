/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   ui_editor_entts.cpp
    Desc:       functional for creation and editing of the prefabs and entities
    Created:    13.05.2026 by DimaSkup
\**********************************************************************************/
#include "ui_editor_entts.h"
#include "ui_utils.h"
#include "ui_ed_state.h"
#include "log.h"
#include "math_helpers.h"
#include "entity_mgr.h"
#include "game_state.h"
#include "ui_facade_engine.h"
#include "texture.h"
#include "game.h"

#include "file_sys.h"
#include <tinyfiledialogs/tinyfiledialogs.h>  // for using OS filesystem dialog window

// components
#include "components/transform.h"
#include "components/sprite.h"
#include "components/enemy.h"
#include "components/health.h"
#include "components/damage.h"
#include "components/collider.h"
#include "components/player.h"
#include "components/movement.h"
#include "components/trigger.h"
#include "components/hierarchy.h"
#include "components/keyboard.h"
#include "components/life_timer.h"

// ImGui
#include <imgui.h>
#include <imgui_internal.h>

#include <time.h>


//---------------------------------------------------------
// default constructor
//---------------------------------------------------------
UI_EditorEntts::UI_EditorEntts() : bUpdateEdLayout_(true)
{
    STATIC_ASSERT(ARRAY_SIZE(s_ComponentNames) == NUM_COMPONENTS);
}

//---------------------------------------------------------
// draw prefabs selector/editor
//---------------------------------------------------------
void UI_EditorEntts::RenderPrefabs()
{
    assert(g_pFacadeEngine);

    const ImGuiChildFlags childFlags = ImGuiChildFlags_None;
    const ImGuiWindowFlags  wndFlags = ImGuiWindowFlags_NoMove;

    const cvector<Entity*>& prefabs = g_EnttMgr.GetPrefabs();

    // if we have only the "dummy" prefab
    if (prefabs.size() <= 1)
    {
        ImGui::Text("No prefabs are created yet");
        return;
    }

    if (pSelectedPrefab_)
    {
        ImGui::Text("Selected prefab: %s", pSelectedPrefab_->GetName());
    }

    if (ImGui::BeginChild("Prefabs", ImVec2(0, 0), childFlags, wndFlags))
    {
        // trigger updating of the icons layout
        if (bUpdateEdLayout_)
        {
            numTilesCols_ = 1;
            numTilesRows_ = 1;
            numItems_ = (int)(prefabs.size() - 1);   // -1 to skip a "dummy" prefab
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

        // draw icons to choose a prefab for entity creation
        DrawWndSelectPrefab({ startPos.x, startPos.y });

        // resize icons
        ComputeIconsZooming({ startPos.x, startPos.y }, availWidth);
    }

    // Submit a 0x0 item to "claim" this space and extend the window boundaries (ImGui shit stuff)
    ImGui::Dummy(ImVec2(0, 0));

    ImGui::EndChild();
}

//---------------------------------------------------------
// setup currently selected entity
//---------------------------------------------------------
void UI_EditorEntts::RenderEnttData()
{
    //
    // switchers for choosing a selectable layer
    //
    const eLayerType options[] = {
        LAYER_GND_DECAL,
        LAYER_ENTITY,
        LAYER_UI,
    };

    assert(selectOnlyLayer_ >= 0 && selectOnlyLayer_ < NUM_LAYERS);

    ImGui::Text("Select only:");
    
    for (int i = 0; i < ARRAY_SIZE(options); ++i)
    {
        const eLayerType layer = options[i];
        bool bActive = (layer == selectOnlyLayer_);

        if (ImGui::RadioButton(s_LayerNames[layer], bActive))
            selectOnlyLayer_ = layer;
    }

    ImGui::Separator();

    //
    // draw properties of the selected entity
    //
    if (g_EditorState.selectedEnttId == INVALID_ENTT_ID)
    {
        ImGui::Text("No entity is selected yet");
        return;
    }

    if (ImGui::BeginChild("EntityEditor", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoMove))
    {
        PrintEnttInfo();
    }
    ImGui::EndChild();
}

//---------------------------------------------------------
// print common info about currently selected entity
//---------------------------------------------------------
void UI_EditorEntts::PrintEnttInfo()
{
    assert(g_EditorState.selectedEnttId != INVALID_ENTT_ID);

    Entity* pEntt = g_EnttMgr.GetEntt(g_EditorState.selectedEnttId);

    // make a prefab based on this entity
    if (ImGui::Button("Save as prefab"))
    {
        g_EnttMgr.AddPrefab(pEntt);
        bUpdateEdLayout_ = true;
    }

    ImGui::Text("Entity id: %d", g_EditorState.selectedEnttId);
    ImGui::Text("Entt name: %s", pEntt->GetName());

    DrawWndAddComponent();
    ImGui::Separator();

    DrawTransformProps(pEntt);
    DrawMovementProps(pEntt);
    DrawSpriteProps(pEntt);
    DrawPlayerProps(pEntt);
    DrawEnemyProps(pEntt);
    DrawHealthProps(pEntt);
    DrawDamageProps(pEntt);
    DrawColliderProps(pEntt);
    DrawTriggerProps(pEntt);
    DrawHierarchyProps(pEntt);
}

//---------------------------------------------------------
// draw a model window for adding a new component for the currently selected entity
//---------------------------------------------------------
void UI_EditorEntts::DrawWndAddComponent()
{
    assert(g_EditorState.selectedEnttId != INVALID_ENTT_ID);

    if (ImGui::Button("Add component"))
    {
        g_EditorState.bOpenedModalWnd = true;
        ImGui::OpenPopup("Add_component");
    }

    if (!g_EditorState.bOpenedModalWnd)
        return;


    // always center this window when appearing
    ImVec2 wndCenter = ImGui::GetMainViewport()->GetCenter();
    ImVec2 wndSz     = ImGui::GetWindowSize();
    ImGui::SetNextWindowPos(wndCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize({ wndSz.x * 0.5f, wndSz.y * 0.5f });

    static eComponentType selectedComponent = NUM_COMPONENTS;
    Entity* pEntt = g_EnttMgr.GetEntt(g_EditorState.selectedEnttId);

    // open a MODAL window
    if (ImGui::BeginPopupModal("Add_component", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // draw a list of components available to add
        for (int i = 0; i < ARRAY_SIZE(s_ComponentNames); ++i)
        {
            if (pEntt->HasComponent((eComponentType)i))
                continue;

            const bool            bSelected = (selectedComponent == (eComponentType)i);
            const ImGuiSelectableFlags flag = ImGuiSelectableFlags_DontClosePopups;

            if (ImGui::Selectable(s_ComponentNames[i], bSelected, flag))
                selectedComponent = (eComponentType)i;
        }

        // add selected component
        if (ImGui::Button("OK"))
        {
            g_EnttMgr.AddComponentToEntt(pEntt->GetId(), selectedComponent);
            g_EditorState.bOpenedModalWnd = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            g_EditorState.bOpenedModalWnd = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

//---------------------------------------------------------
// update layout of the tilemaps editor when we modify window size
//---------------------------------------------------------
void UI_EditorEntts::UpdateEdLayout(float availWidth)
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
// Desc:  draw a grid (a list) of available prefabs
// Args:  startPos - where to start drawing of icons
//---------------------------------------------------------
void UI_EditorEntts::DrawWndSelectPrefab(Vec2 startPos)
{
    ImGui::PushStyleColor(ImGuiCol_Header,        { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  { 1,1,1,1 });

    const int     numCols = numLayoutCols_;
    const ImVec2 iconSize = { iconSize_, iconSize_ };
    const ImVec2 itemSize = { layoutItemSizeX_, layoutItemSizeY_ };

    const cvector<Entity*>& prefabs = g_EnttMgr.GetPrefabs();
    assert(numItems_ == (prefabs.size() - 1));

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
                const ImVec2    iconPos = { startPos.x + itemOffsetX, startPos.y + itemOffsetY };

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
                snprintf(buf, sizeof(buf), "##prefab_%d", itemIdx);

                bool bSelected = (itemIdx == selectedItemIdx_);

                // we can select a prefab when click on some icon
                // NOTE: for selected prefabs we render a border around it icon
                if (ImGui::Selectable(buf, bSelected, ImGuiSelectableFlags_SelectOnClick, itemSize))
                {
                    selectedItemIdx_ = itemIdx;

                    // +1 to skip "invalid" prefab (by index == 0)
                    pSelectedPrefab_ = g_EnttMgr.GetPrefabs()[selectedItemIdx_ + 1];
                }

                // draw background (so we will have a border around the icon)
                const ImVec2 boxMin = { iconPos.x - 1, iconPos.y - 1 };
                const ImVec2 boxMax = { boxMin.x + itemSize.x + 2, boxMin.y + itemSize.y + 2 };
                const ImU32 bgColor = ImGui::GetColorU32({ 0,0,0,1 });

                ImGui::GetWindowDrawList()->AddRectFilled(boxMin, boxMax, bgColor);

                //
                // draw texture image
                //
                ImGui::SetCursorScreenPos(iconPos);

                Entity* prefab = prefabs[itemIdx + 1];

                // if this prefab is invisible (for instance: triggers)
                if (!prefab->HasComponent<Sprite>())
                {
                    ImGui::Image((ImTextureID)nullptr, iconSize, { 0,0 }, { 1,1 });
                }

                // this prefab is drawable so render its sprite
                else
                {
                    Sprite* spr = prefab->GetComponent<Sprite>();
                    spr->Update(g_GameState.deltaTime);  // update animation (if sprite is animated)

                    const TexID texId = spr->GetTex();
                    const Texture* tex = g_pFacadeEngine->GetTex(texId);

                    // calc uv for this icon
                    const Vec2 texSz = ToVec2(g_pFacadeEngine->GetTexSize(texId));
                    const Rect r = ToRect(spr->GetSrcRect());
                
                    const Vec2 invTexSz = { 1.0f / texSz.w, 1.0f / texSz.h };
                    Vec2 uv0      = { r.x0 * invTexSz.w, r.y0 * invTexSz.h };
                    Vec2 uv1      = { r.x1 * invTexSz.w, r.y1 * invTexSz.h };

                    // clamp texture coords (to prevent ImGui fucking up)
                    Vec2Clamp(uv0, { 0,0 }, { 1,1 });
                    Vec2Clamp(uv1, { 0,0 }, { 1,1 });

                    // draw prefab icon
                    ImGui::Image((ImTextureID)tex->pTex, iconSize, { uv0.x, uv0.y }, { uv1.x, uv1.y });
                }
               

                //
                // draw a label (just an index)
                //
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
void UI_EditorEntts::ComputeIconsZooming(Vec2 startPos, float availWidth)
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
    const float hoveredItemNX = (io.MousePos.x - startPos.x + layoutItemSpacing_ * 0.5f) / layoutItemStepX_;
    const float hoveredItemNY = (io.MousePos.y - startPos.y + layoutItemSpacing_ * 0.5f) / layoutItemStepY_;
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
// Desc:  draw a dropdown menu (ImGui::Combo)
// Args:  - label:       label for the menu
//        - comboId:     unique identifier for the menu
//        - currEventId: (in/out) identifier of the current event bounded to the trigger's stage
//
// Ret:   true if we selected any other option (NOTE: currEventId is modified in this case)
//---------------------------------------------------------
bool UI_EditorEntts::DrawDropdownSelectTriggerEvent(
    const char* label,
    const char* comboId,
    EventID& currEventId)
{
    // check input args
    if (!label)
    {
        LogErr(LOG, "empty label");
        return false;
    }
    if (!comboId)
    {
        LogErr(LOG, "empty combo id");
        return false;
    }

    //---------------------------------

    bool bChanged = false;

    const char*             currEventName = g_pFacadeEngine->GetEventNameById(currEventId);
    const cvector<EventName>& eventsNames = g_pFacadeEngine->GetAllEventsNames();
    const cvector<EventID>&     eventsIds = g_pFacadeEngine->GetAllEventsIds();


    ImGui::TableNextColumn();
    ImGui::Text(label);

    ImGui::TableNextColumn();
    ImGui::PushItemWidth(-FLT_MIN);

    // a dropdown menu
    if (ImGui::BeginCombo(comboId, currEventName))
    {
        for (int i = 0; i < eventsIds.size(); ++i)
        {
            const bool bSelected = (currEventId == eventsIds[i]);

            char label[64];
            snprintf(label, sizeof(label), "(id: %d, name: %s)", eventsIds[i], eventsNames[i].name);

            if (ImGui::Selectable(label, bSelected))
            {
                bChanged = true;
                currEventId = eventsIds[i];
            }

            // set the initial focus when opening the combo
            if (bSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    return bChanged;
}


//=============================================================================
//=============================================================================
// draw properties + components of selected entity
// and control fields for modifying these properties
//=============================================================================
//=============================================================================

//---------------------------------------------------------
// draw fields for modifying the Transform component of input entity
//---------------------------------------------------------
void UI_EditorEntts::DrawTransformProps(Entity* pEntt) 
{
    assert(pEntt);

    Transform* t  = pEntt->GetComponent<Transform>();
    assert(t && "all entities must have a Transform component!!!");

    if (ImGui::CollapsingHeader("Transform"))
    {
        Vec2i pos             = ToVec2i(t->GetPos());
        Vec2i szLocal         = t->GetLocalSize();
        Vec2i szWorld         = t->GetWorldSize();
        int   scale           = t->GetScale();
        bool  bUpdateEdCursor = false;

        if (ImGui::DragInt2("Position", &pos.x))
        {
            // clamp position to world's boundings
            pos = Vec2iClamp(pos, { 0,0 }, g_GameState.iWorldSize);
            t->SetPos(pos);
            bUpdateEdCursor = true;
        }
        
        // for UI elements we additionaly show its pos in percentage of the screen
        if (pEntt->GetLayer() == LAYER_UI)
        {
            Vec2 percentsPos = {
                t->GetPos().x / (float)g_GameState.screenSize.w * 100.0f,
                t->GetPos().y / (float)g_GameState.screenSize.h * 100.0f,
            };

            ImGui::BeginDisabled(true);
            ImGui::DragFloat2("Pos percents", &percentsPos.x);
            ImGui::EndDisabled();
        }

        ImGui::Text("Size in world: %dx%d", szWorld.w, szWorld.h);
            
        if (ImGui::DragInt2("Original size:", &szLocal.w))
        {
            bUpdateEdCursor = true;
            t->SetLocalSize(szLocal);
        }

        if (ImGui::DragInt("Scale:", &scale))
        {
            bUpdateEdCursor = true;
            t->SetScale(scale);
        }

        // for UI elements we additionaly show its size (with applied scale)
        // in percentage of the screen
        if (pEntt->GetLayer() == LAYER_UI)
        {
            const Vec2 sz = ToVec2(t->GetWorldSize());

            Vec2 percentsSize = {
                sz.w / (float)g_GameState.screenSize.w * 100.0f,
                sz.h / (float)g_GameState.screenSize.h * 100.0f,
            };

            ImGui::BeginDisabled(true);
            ImGui::DragFloat2("Size percents", &percentsSize.x);
            ImGui::EndDisabled();
        }

        if (bUpdateEdCursor == true)
        {
            g_pFacadeEngine->UpdateEditorCursorTransform(t->GetPos(), szLocal, scale);
        }

        ImGui::Separator();
    }
}

//---------------------------------------------------------
// draw fields for modifying the Movement component of input entity
//---------------------------------------------------------
void UI_EditorEntts::DrawMovementProps(Entity* pEntt) 
{
    assert(pEntt);

    Movement* pMov = pEntt->GetComponent<Movement>();
    if (!pMov)
        return;

    if (ImGui::CollapsingHeader("Movement"))
    {
        Vec2 vel = pMov->GetVelocity();
        if (ImGui::DragFloat2("velocity", &vel.x))
            pMov->SetVelocity(vel.x, vel.y);

        ImGui::Separator();
    }
}

//---------------------------------------------------------
// 1. open dialog window to select a texture
// 2. load this texture and create an asset
// 3. return ID of this texture asset
//---------------------------------------------------------
TexID LoadTexture()
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

        return g_pFacadeEngine->LoadTexture(texName, fullPath);
    }

    return INVALID_TEX_ID;
}

//---------------------------------------------------------
// draw info about texture related to animation by input type, visualize texture;
// and draw button to select another texture
//---------------------------------------------------------
void DrawTextureInfo(Sprite* pSpr, const eAnimType animType)
{
    TexID texId = pSpr->GetTex(animType);

    if (ImGui::Button("Load texture"))
    {
        texId = LoadTexture();
        pSpr->SetTex(animType, texId);
    }

    const Vec2i      iTexSz = g_pFacadeEngine->GetTexSize(texId);
    const Vec2        texSz = ToVec2(iTexSz);
    const float  availWidth = ImGui::GetContentRegionAvail().x;
    const float scaleFactor = availWidth / texSz.w;
    const ImVec2     iconSz = ImVec2(texSz.w * scaleFactor, texSz.h * scaleFactor);

    // common info
    ImGui::Text("ID: %d", texId);
    ImGui::Text("Name: %s", g_pFacadeEngine->GetTexName(texId));
    ImGui::Text("Size: %dx%d", iTexSz.w, iTexSz.h);

    // setup which part of the texture we will use (in cases when using texture atlas)
    const Recti& srcRect = pSpr->GetSrcRect();
    Vec2i topLeft = { srcRect.x0, srcRect.y0 };
    Vec2i size = { srcRect.x1 - srcRect.x0, srcRect.y1 - srcRect.y0 };

    ImGui::Text("Source texture rectangle:");
    if (ImGui::DragInt2("position:", &topLeft.x))
    {
        if (topLeft.x < 0) topLeft.x = 0;
        if (topLeft.y < 0) topLeft.y = 0;

        pSpr->SetSrcRectPos(topLeft);
    }

    if (ImGui::DragInt2("size:", &size.x))
    {
        if (size.x < 1) size.x = 1;
        if (size.y < 1) size.y = 1;

        pSpr->SetSrcRectSize(size);
    }

    // visualize the texture
    const Texture* tex = g_pFacadeEngine->GetTex(texId);
    ImGui::Image((ImTextureID)tex->pTex, iconSz);
}

//---------------------------------------------------------
// draw fields which are used to setup a sprite's animation of input type
//---------------------------------------------------------
void DrawAnimationControlFields(Sprite* pSpr, const eAnimType animType)
{
    assert(pSpr);
    assert(animType >= 0 && animType < NUM_ANIM_TYPES);

    Animation anim = pSpr->GetAnimByType(animType);

    //
    // TEXTURE
    //
    if (ImGui::CollapsingHeader("TEXTURE"))
        DrawTextureInfo(pSpr, animType);

    //
    // ANIMATIONS
    //
    if (ImGui::CollapsingHeader("ANIMATIONS"))
    {
        ImGui::Text("ID: %d", anim.id);
        ImGui::Text("Name: %s", pSpr->GetAnimTypeName(animType));

        // if we have multiple different types of animations
        if (pSpr->GetAnimations().size() > 1)
        {
            const eAnimDir currAnimDir = pSpr->GetCurrAnimDir();
            ImGui::Text("Curr direction: %s", pSpr->GetAnimDirName(currAnimDir));


            // select animation to modify
            const char** animationsNames = pSpr->GetArrAnimTypeNames();
            const char* currAnimName = pSpr->GetAnimTypeName(animType);

            if (DrawDropdown("Type: ", "##select_trig_type", &currAnimName, animationsNames, NUM_ANIM_TYPES))
            {
                const eAnimType newAnimType = pSpr->GetAnimType(currAnimName);
                pSpr->PlayAnimation(newAnimType);
            }
        }

        int frameTimeMs = anim.speed;
        if (ImGui::DragInt("frame time (ms)", &frameTimeMs))
        {
            if (frameTimeMs < 1)
                frameTimeMs = 1;

            anim.speed = frameTimeMs;
            pSpr->SetupAnimation(animType, anim);
        }

        int numFrames = anim.numFrames;
        if (ImGui::DragInt("num frames", &numFrames))
        {
            if (numFrames < 2)
                numFrames = 2;  // animation must have at least 2 frames

            anim.numFrames = numFrames;
            pSpr->SetupAnimation(animType, anim);
        }

        bool bLooped = anim.bLooped;
        if (ImGui::Checkbox("Looped", &bLooped))
        {
            anim.bLooped = bLooped;
            pSpr->SetupAnimation(animType, anim);
        }

        ImGui::Text("  - played_once: %d", anim.bPlayedOnce);
    }
}

//---------------------------------------------------------
// draw fields for modifying the Sprite component of input entity
//---------------------------------------------------------
void UI_EditorEntts::DrawSpriteProps(Entity* pEntt) 
{
    assert(pEntt);

    Sprite* pSpr = pEntt->GetComponent<Sprite>();
    if (!pSpr)
        return;

    if (ImGui::CollapsingHeader("Sprite"))
    {
        Color c = pSpr->GetColor();
       

        // show/hide
        bool bShow = pSpr->IsShow();
        if (ImGui::Checkbox("Show", &bShow))
            pSpr->SetShow(bShow);

        //
        // setup color
        //
        const ImVec4 color = ImGui::ColorConvertU32ToFloat4(c.rgba);
        if (ImGui::ColorEdit4("Color", (float*)&color))
        {
            c.rgba = ImGui::ColorConvertFloat4ToU32(color);
            pSpr->SetColor(c);
        }

        //
        // switch btw: NO_ANIMATIONS, SINGLE_ANIMATION, and MULTIPLE_ANIMATIONS
        //
        const AnimationsMap& anims = pSpr->GetAnimations();
        const bool bAnimated = pSpr->IsAnimated();

        int layoutType = 0;
        constexpr int NO_ANIMATIONS = 1;
        constexpr int SINGLE_ANIMATION = 2;
        constexpr int MULTIPLE_ANIMATIONS = 3;

        if (!bAnimated)
        {
            assert(anims.size() == 1);
            layoutType = NO_ANIMATIONS;
        }
        else
        {
            assert(anims.size() >= 1);

            if (anims.size() == 1)
                layoutType = SINGLE_ANIMATION;
            else 
                layoutType = MULTIPLE_ANIMATIONS;
        }

        //
        // switch between animations presets
        //
        if (ImGui::RadioButton("No animations", layoutType == NO_ANIMATIONS))
            pSpr->SetAnimPreset(ANIM_PRESET_NO_ANIM);

        if (ImGui::RadioButton("Single animation", layoutType == SINGLE_ANIMATION))
            pSpr->SetAnimPreset(ANIM_PRESET_SINGLE_ANIM);

        if (ImGui::RadioButton("Multiple animations", layoutType == MULTIPLE_ANIMATIONS))
            pSpr->SetAnimPreset(ANIM_PRESET_MULTIPLE_ANIM);


        ImGui::Separator();

        //
        // draw fields to control textures and animations
        //
        if (layoutType == NO_ANIMATIONS)
            DrawTextureInfo(pSpr, ANIM_TYPE_IDLE);

        else if (layoutType == SINGLE_ANIMATION)
            DrawAnimationControlFields(pSpr, ANIM_TYPE_IDLE);

        else if (layoutType == MULTIPLE_ANIMATIONS)
        {


            DrawAnimationControlFields(pSpr, pSpr->GetCurrAnimType());
        }
            

        ImGui::Separator();

        //
        // ground decals have a Z-index
        //
        if (pEntt->GetLayer() == LAYER_GND_DECAL)
        {
            const char* label = "Z-index [-127, 127]:";
            int zIndex = pSpr->GetZIndex();

            if (ImGui::DragInt(label, &zIndex, 1.0f, INT8_MIN, INT8_MAX))
                pSpr->SetZIndex(zIndex);
        }
     
        ImGui::Separator();
    }
}

//---------------------------------------------------------
// draw fields for modifying the Player component of input entity
//---------------------------------------------------------
void UI_EditorEntts::DrawPlayerProps(Entity* pEntt) 
{
    assert(pEntt);
    
    Player* pPlayer = pEntt->GetComponent<Player>();
    if (!pPlayer)
        return;

    if (ImGui::CollapsingHeader("Player"))
    {
        float basicSpeed = pPlayer->GetMoveBasicSpeed();

        if (ImGui::DragFloat("player speed", &basicSpeed, 1.0f, 1.0f))
            pPlayer->SetMoveBasicSpeed(basicSpeed);

        ImGui::Separator();
    }
}

//---------------------------------------------------------
// draw fields for modifying the Enemy component of input entity
//---------------------------------------------------------
void UI_EditorEntts::DrawEnemyProps(Entity* pEntt) 
{
    assert(pEntt);

    const Enemy* pEnemy = pEntt->GetComponent<Enemy>();
    if (!pEnemy)
        return;

    if (ImGui::CollapsingHeader("Enemy"))
    {
        const Vec2   moveTo = pEnemy->GetMoveTo();
        const Vec2   moveDir = pEnemy->GetDir();
        const Vec2   vel = pEnemy->GetVelocity();

        ImGui::Text("Type: %s", pEnemy->GetTypeAsStr());
        ImGui::Text("State: %s", pEnemy->GetCurrStateName());
        ImGui::Text("Move to: %.1f %.1f", moveTo.x, moveTo.y);
        ImGui::Text("Move dir: %.1f %.1f", moveDir.x, moveDir.y);
        ImGui::Text("Velocity: %.1f %.1f", vel.x, vel.y);
        ImGui::Text("Idling timer (sec): %.3f", pEnemy->GetIdleTime());

        ImGui::Separator();
    }
}

//---------------------------------------------------------
// draw fields for modifying the Health component of input entity
//---------------------------------------------------------
void UI_EditorEntts::DrawHealthProps(Entity* pEntt) 
{
    assert(pEntt);
    
    Health* h = pEntt->GetComponent<Health>();
    if (!h)
        return;

    if (ImGui::CollapsingHeader("Heath"))
    {
        ImGui::Text("HP: %d", h->GetHP());

        ImGui::Separator();
    }
}

//---------------------------------------------------------
// draw fields for modifying the Damage component of input entity
//---------------------------------------------------------
void UI_EditorEntts::DrawDamageProps(Entity* pEntt) 
{
    assert(pEntt);

    Damage* pDmg = pEntt->GetComponent<Damage>();
    if (!pDmg)
        return;

    if (ImGui::CollapsingHeader("Damage"))
    {
        ImGui::Text("Curr cooldown (sec): %f",   pDmg->GetCurrCooldownSec());
        ImGui::Text("Attack cooldown (sec): %f", pDmg->GetCooldownMaxSec());
        ImGui::Text("Damage: %f",                pDmg->GetDamage());

        ImGui::Separator();
    }
}

//---------------------------------------------------------
// draw fields for modifying the Collider component of input entity
//---------------------------------------------------------
void UI_EditorEntts::DrawColliderProps(Entity* pEntt) 
{
    assert(pEntt);

    Collider* pCollider = pEntt->GetComponent<Collider>();
    if (!pCollider)
        return;

    if (ImGui::CollapsingHeader("Collider"))
    {
        //
        // switch collider type
        //
        const eColliderType type = pCollider->GetType();
        ImGui::Text("Collider type:");
        ImGui::SameLine();

        if (ImGui::RadioButton("box", type == COLLIDER_BOX))
        {
            if (type != COLLIDER_BOX)
                pCollider->SetType(COLLIDER_BOX);
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("circle", type == COLLIDER_CIRCLE))
        {
            if (type != COLLIDER_CIRCLE)
                pCollider->SetType(COLLIDER_CIRCLE);
        }

        //
        // is_solid - we can't move through this collider
        //
        ImGui::Text("Is solid:");
        ImGui::SameLine();

        bool bSolid = pCollider->IsSolid();
        if (ImGui::Checkbox("##is_solid", &bSolid))
            pCollider->SetSolid(bSolid);

        //
        // setup collider according to its type
        //
        if (type == COLLIDER_BOX)
        {
            Vec2i p = pCollider->GetBoxPos();
            Vec2i s = pCollider->GetBoxSize();

            if (ImGui::DragInt2("Local pos", &p.x))
                pCollider->SetBoxPos(p.x, p.y);

            if (ImGui::DragInt2("World size", &s.w))
                pCollider->SetBoxSize(s.w, s.h);
        }

        else if (type == COLLIDER_CIRCLE)
        {
            Vec2i c = pCollider->GetCenter();
            int   r = pCollider->GetRadius();

            if (ImGui::DragInt2("Center pos", &c.x))
                pCollider->SetCenter(c.x, c.y);

            if (ImGui::DragInt("Radius", &r))
                pCollider->SetRadius(r);
        }

        ImGui::Separator();
    }
}

//---------------------------------------------------------
// draw fields for modifying the Trigger component of input entity
//---------------------------------------------------------
void UI_EditorEntts::DrawTriggerProps(Entity* pEntt) 
{
    assert(pEntt);

    Trigger* t = pEntt->GetComponent<Trigger>();
    if (!t)
        return;

    if (ImGui::CollapsingHeader("Trigger"))
    {
        Game* pGame = (Game*)g_GameState.pGame;
        EventMgr* pEventMgr = pGame->GetEventMgr();
        const eTriggerType trigType = t->GetType();
        const char* currTrigTypeName = s_TriggerTypesNames[trigType];

        if (DrawDropdown("Type: ", "##select_trig_type", &currTrigTypeName, s_TriggerTypesNames, NUM_TRIGGER_TYPES))
        {
            const eTriggerType newType = Trigger::GetTypeByName(currTrigTypeName);
            t->SetType(newType);
        }

        EventID onEnterId   = t->GetOnEnter();
        EventID onCollideId = t->GetOnCollide();
        EventID onLeaveId   = t->GetOnLeave();

        //
        // Draw a table of dropdown menus to select events for trigger's stages
        //
        const int             numColumns = 2;
        const ImGuiTableFlags tableFlags = 0;

        if (ImGui::BeginTable("setup_trigger_events", numColumns, tableFlags))
        {
            if (DrawDropdownSelectTriggerEvent(" on enter:", "##select_on_enter_event", onEnterId))
                t->SetOnEnter(onEnterId);

            if (trigType == TRIGGER_MULTIPLE)
            {
                if (DrawDropdownSelectTriggerEvent(" on collide:", "##select_on_collide_event", onCollideId))
                    t->SetOnCollide(onCollideId);

                if (DrawDropdownSelectTriggerEvent(" on leave:", "##select_on_leave_event", onLeaveId))
                    t->SetOnLeave(onLeaveId);
            }

            ImGui::EndTable();
        }

        ImGui::Separator();
    }
}

//---------------------------------------------------------
// draw fields for modifying the Hierarchy component of input entity
//---------------------------------------------------------
void UI_EditorEntts::DrawHierarchyProps(Entity* pEntt)
{
    assert(pEntt);

    Hierarchy* h = pEntt->GetComponent<Hierarchy>();
    if (!h)
        return;

    if (ImGui::CollapsingHeader("Hierarchy"))
    {
        //
        // info abount parent
        //
        const EntityID parentId = h->GetParent();

        if (parentId == INVALID_ENTT_ID)
        {
            ImGui::Text("Parent: no parent");
        }
        else
        {
            const Entity* pParent = g_EnttMgr.GetEntt(parentId);
            bool bSelected = false;

            char label[64];
            snprintf(label, sizeof(label), "Parent: %s (id: %d)", pParent->GetName(), parentId);

            // we can click on this field to pick this entity
            if (ImGui::Selectable(label, bSelected))
                g_pFacadeEngine->SelectEntt(parentId);
        }

        //
        // info about children
        //
        const size_t numChildren = h->GetNumChildren();

        if (numChildren == 0)
        {
            ImGui::Text("Children: no children");
        }
        else
        {
            ImGui::Text("Children:");
            const EntityID* childrenIds = h->GetChildren();

            for (size_t i = 0; i < numChildren; ++i)
            {
                const EntityID id = childrenIds[i];
                const Entity* pChild = g_EnttMgr.GetEntt(id);

                bool bSelected = false;
                char label[64];
                snprintf(label, sizeof(label), " - Child: %s (id: %d)", pChild->GetName(), id);

                // we can click on this field to pick this entity
                if (ImGui::Selectable(label, bSelected))
                    g_pFacadeEngine->SelectEntt(id);
            }
        }

        //
        // info about position relatively to the parent (if has any)
        //
        Vec2 relPos = h->GetRelativePos();

        ImGui::Text("Relative pos:");
        ImGui::SameLine();

        if (parentId == INVALID_ENTT_ID)
        {
            ImGui::BeginDisabled(true);
            ImGui::DragFloat2("##relative_pos", &relPos.x);
            ImGui::EndDisabled();
        }
        else
        {
            if (ImGui::DragFloat2("##relative_pos", &relPos.x))
                h->SetRelativePos(relPos.x, relPos.y);
        }

        ImGui::Separator();
    }
}
