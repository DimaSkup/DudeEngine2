/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   ui_ed_state.h
    Desc:       editor's states
    Created:    29.10.2026 by DimaSkup
\**********************************************************************************/
#pragma once

#include "types.h"

struct EditorState
{
    EntityID selectedEnttId;                    // if 0 then no entity is selected

    uint32 bShowEditorCursor        : 1;        // show/hide an "outliner" (red rectangle) which is used to highlight the selected entity
    uint32 bGrabbingEntt            : 1;        // are we currently moving the selected entity?
    uint32 bSceneViewportHovered    : 1;        // is the scene viewport currently hovered by mouse?
    uint32 bNoClipMode              : 1;        // camera free fly (not sticked to the player)
    uint32 bOpenedModalWnd          : 1;        // some editor's modal window is opened, so we want to block any actions outside this window (changing tilemaps, picking another entts, etc.)

    uint8 activeEditor;                         // which type of editor is currently active?
                                                // for details look at enumeration `eEditorType`
};

// global instance
extern EditorState g_EditorState;