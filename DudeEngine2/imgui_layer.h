/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:  imgui_layer.h
    Desc:      a wrapper for ImGui initialization and running
    Created:   by DimaSkup
\**********************************************************************************/
#ifndef IMGUI_LAYER_H
#define IMGUI_LAYER_H

#include <imgui.h>

class ImGuiLayer
{
public:
    ImGuiLayer();

    bool Init();
    void Shutdown();

    void Begin();
    void End();

private:
    ImGuiContext* pImGuiCtx_;
};

#endif