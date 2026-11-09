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
#ifndef UI_UTILS_H
#define UI_UTILS_H

bool DrawDropdown(const char* label,
                  const char* comboId,
                  const char** currParam,
                  const char** items,
                  const int numItems);

#endif
