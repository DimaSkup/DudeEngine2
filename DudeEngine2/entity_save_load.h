/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:  entity_save_load.h
    Desc:      functional for saving and loading entities (from/into a file)
    Created:   23.05.2026 by DimaSkup
\**********************************************************************************/
#ifndef ENTITY_SAVE_LOAD_H
#define ENTIYT_SAVE_LOAD_H

#include "entity.h"

class EntitySaveLoad
{
public:
    EntitySaveLoad();

    void SaveEntt(FILE* pFile, const Entity* pEntt);
    void LoadEntt(FILE* pFile, Entity* pEntt);
};

#endif
