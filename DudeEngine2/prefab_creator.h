/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   prefab_creator.h
    Desc:       manually create prefabs (which will be used 
                as origin entities for all the other entities which we create)
    Created:    08.06.2026 by DimaSkup
\**********************************************************************************/
#ifndef PREFAB_CREATOR_H
#define PREFAB_CREATOR_H

#include "types.h"

void CreateAllPrefabs();
void CreatePrefabTeleport(const EventID onEnter, const char* levelName);

#endif
