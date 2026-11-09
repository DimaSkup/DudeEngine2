/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   map.h
    Desc:       represents a map (background) of the level
    Created:    by DimaSkup
\**********************************************************************************/
#ifndef MAP_H
#define MAP_H

#include "types.h"
#include "consts.h"
#include "rect.h"
#include "vec.h"
#include "cvector.h"

//---------------------------------------------------------
// forward declaration (pointer use only)
//---------------------------------------------------------
class Entity;

//---------------------------------------------------------
// class name:   Map
//---------------------------------------------------------
class Map
{
public:
    Map();
    ~Map();

    bool        CreateNewMap(const char* mapName,  int mapCellsX, int mapCellsY);
    void        LoadMap     (const char* filePath);
    void        SaveMap();
    void        SaveMapAs(const char* filepath);
    void        ReleaseMap();
    void        ResizeMap(Vec2i numCells);
                
    TexID       GetTexId() const;
    void        SetTexture(TexID texId);
    void        SetTileTex(int texTileIdxX, int texTileIdxY, int mapCellIdxX, int mapCellIdxY);
                
    Vec2i       GetMapSize() const;
    Vec2i       GetNumCells() const;

    const char* GetMapName() const;
    Vec2i       GetNumTexTiles() const;


private:
    int GetTileIdx(const Entity* pTile) const;

    Recti CalcSrcRect(const int tileIdxX, const int tileIdxY) const;
    Recti CalcDstRect(const int cellIdxX, const int cellIdxY) const;

    // helpers: saving map
    void SaveMapCommonInfo(FILE* pFile, const char* mapName);
    void SaveMapTilesIndices(FILE* pFile);

    // helpers: loading map
    void LoadMapCommonInfo(FILE* pFile);

    void CreateTilesGrid(const int numCellsX, const int numCellsY);

    void LoadMapTilesIndices(FILE* pFile);

    // DEBUG
    void DumpTiles();

private:
    char mapName_[MAX_LEN_MAP_NAME];
    TexID texId_;                       // tilemap texture
    cvector<EntityID> tiles_;       
    int mapCellsX_;                     // number of cells by X
    int mapCellsY_;                     // number of cells by Y
    bool bWasModified_;
};

//=========================================================
// INLINE FUNCTIONS
//=========================================================

// get ID of the current tilemap texture
inline TexID Map::GetTexId() const
{
    return texId_;
}

// get a name of the current map
inline const char* Map::GetMapName() const
{
    return mapName_;
}

// get size (in pixels) of the whole world (current scene)
inline Vec2i Map::GetMapSize() const
{
    return { mapCellsX_*MAP_CELL_SIZE, mapCellsY_* MAP_CELL_SIZE };
}

// get a number of cells on the tilemap (by X and Y)
inline Vec2i Map::GetNumCells() const
{
    return Vec2i{ mapCellsX_, mapCellsY_ };
}

#endif
