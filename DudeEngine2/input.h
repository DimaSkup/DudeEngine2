/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   input.h
    Desc:       wrapper for different input backends (SDL, LDL, etc.)
    Created:    07.06.2026 by DimaSkup
\**********************************************************************************/
#ifndef INPUT_MGR_H
#define INPUT_MGR_H

#include "input_events.h"

void               InputInit();
void               InputProcess();
bool               InputGetEvent(InputEvent* e);

const uint8*       GetKeyboardState();
bool               IsKeyPressed(eKeyCode k);
eKeyCode           GetKeycodeByName(const char* keyName);
const char*        GetKeyName(const eKeyCode c);

const Mouse&       GetMouseState(void);

#endif
