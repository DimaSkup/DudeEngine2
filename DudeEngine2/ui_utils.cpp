/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: ui_utils.h
    Desc:     helpers for the editor

    Created:  11.09.2026 by DimaSkup
\**********************************************************************************/
#include "log.h"
#include <imgui.h>

//---------------------------------------------------------
// Desc:  draw a dropdown menu (ImGui::Combo)
// Args:  - label:      label for the menu
//        - comboId:    unique identifier for the menu
//        - currParam:  current parameter (will be highlighted) and if we
//                      select any option it will be rewritten by this option value
//                      (we will just set a pointer to another value)
//        - items:      selectable options of menu
//        - numItems:   num of selectable options in menu
//
// Ret:   true if we selected any other option
//---------------------------------------------------------
bool DrawDropdown(
    const char* label,
    const char* comboId,
    const char** currParam,
    const char** items,
    const int numItems)
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
    if (!currParam || !(*currParam) || (*currParam)[0] == '\0')
    {
        LogErr(LOG, "empty current param");
        return false;
    }
    if (!items)
    {
        LogErr(LOG, "empty items arr");
        return false;
    }

    //---------------------------------

    bool bChanged = false;

    ImGui::TableNextColumn();
    ImGui::Text(label);

    ImGui::TableNextColumn();
    ImGui::PushItemWidth(-FLT_MIN);

    // dropdown menu
    if (ImGui::BeginCombo(comboId, *currParam))
    {
        for (int i = 0; i < numItems; ++i)
        {
            const bool bSelected = (strcmp(*currParam, items[i]) == 0);
            if (ImGui::Selectable(items[i], bSelected))
            {
                *currParam = items[i];
                bChanged = true;
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