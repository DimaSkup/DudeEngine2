/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: imgui_layer.cpp
    Created:  by DimaSkup
\**********************************************************************************/
#include "imgui_layer.h"
#include "r_main.h"
#include "log.h"
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <imgui_internal.h>
#include <SDL_Render.h>

//---------------------------------------------------------
// default constructor
//---------------------------------------------------------
ImGuiLayer::ImGuiLayer() : pImGuiCtx_{ nullptr }
{
}

//---------------------------------------------------------
// init ImGui stuff here (and its docking as well)
//---------------------------------------------------------
bool ImGuiLayer::Init()
{
    // init ImGui
    IMGUI_CHECKVERSION();
    pImGuiCtx_  = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    if (!ImGui_ImplSDL2_InitForSDLRenderer(g_Render.GetWindow(), g_Render.GetRenderer()))
    {
        LogErr(LOG, "can't init init ImGui for SDL renderer");
        Shutdown();
        return false;
    }
    ImGui_ImplSDLRenderer2_Init(g_Render.GetRenderer());

    // setup behavior
    io.WantCaptureMouse = true;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // setup fonts
    const char* pathFont = "assets/fonts/arial.ttf";
    const float fontSize = 16.0f;

    io.FontDefault = io.Fonts->AddFontFromFileTTF(pathFont, fontSize);
    if (!io.FontDefault)
    {
        LogErr(LOG, "can't load a font from file: %s", pathFont);
        return false;
    }

    // setup ImGui viewport
    ImGuiViewport* pViewport = ImGui::GetMainViewport();
    const Vec2 wndSz = ToVec2(g_Render.GetWndSize());

    pViewport->Pos      = { 0, 0 };
    pViewport->WorkPos  = { 0, 0 };

    pViewport->Size     = { wndSz.w, wndSz.h };
    pViewport->WorkSize = { wndSz.w, wndSz.h };

    
    // great success!
    return true;
}

//---------------------------------------------------------
// shutdown ImGui stuff
//---------------------------------------------------------
void ImGuiLayer::Shutdown()
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

//---------------------------------------------------------
// start the ImGui frame
//---------------------------------------------------------
void ImGuiLayer::Begin()
{
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // ImGui docking
    static bool                 bInitialized    = false;
    static bool                 bDockspaceOpen  = true;
    static bool                 bFullscreen     = true;
    static bool                 bPadding        = false;
    static ImGuiDockNodeFlags_  dockspaceFlags  = ImGuiDockNodeFlags_None;
    //static ImGuiDockNode*       pSceneNode      = nullptr;
    const  ImGuiViewport*       pViewport       = ImGui::GetMainViewport();

    // setup the dockspace wnd style
    ImGui::SetNextWindowPos(pViewport->WorkPos);
    ImGui::SetNextWindowSize(pViewport->WorkSize);
    ImGui::SetNextWindowViewport(pViewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGuiWindowFlags wndFlags = 0;
    wndFlags |= (ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);
    wndFlags |= (ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus);
    wndFlags |= (ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove);


    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
   // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
    {
        wndFlags |= ImGuiWindowFlags_NoTitleBar;
    }

    // push this style since we want to disable padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Root", &bDockspaceOpen, wndFlags);
    ImGui::PopStyleVar(3);

    // DockSpace 
    // NOTE: it is supposed that docking is always enalbed: (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) == true

    ImGuiID dockspaceId = ImGui::GetID("Root");
    ImGui::DockSpace(dockspaceId, pViewport->WorkSize, dockspaceFlags);

    // setup docked window positions and sizes
    if (!bInitialized)
    {
        bInitialized = true;

        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(dockspaceId, dockspaceFlags | ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceId, pViewport->Size);

        // sprit main dockspace into separate dock windows
        ImGuiID dockIdRight = dockspaceId;
        ImGuiID dockIdLeft  = ImGui::DockBuilderSplitNode(dockIdRight, ImGuiDir_Left,  0.75f, nullptr, &dockIdRight);

        // register dock windows and relate them to specific window names
        ImGui::DockBuilderDockWindow("Viewport", dockIdLeft);
        ImGui::DockBuilderDockWindow("Editor",   dockIdRight);

        //pSceneNode = ImGui::DockBuilderGetNode(dockIdScene);

        ImGui::DockBuilderFinish(dockspaceId);
    }
}

//---------------------------------------------------------
// present ImGui stuff onto the screen
//---------------------------------------------------------
void ImGuiLayer::End()
{
    ImGui::End();
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), g_Render.GetRenderer());
}