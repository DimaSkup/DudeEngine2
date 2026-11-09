/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: map.cpp
    Created:  by DimaSkup
\**********************************************************************************/
#include "map.h"
#include "log.h"
#include "entity_mgr.h"
#include "asset_mgr.h"
#include "components/tile.h"
#include "file_sys.h"
#include "entity.h"
#include "math_helpers.h"
#include <assert.h>


//---------------------------------------------------------
// Desc:  constructor
// Args:  tileSize -- width and height of a single map cell in the world
//---------------------------------------------------------
Map::Map() :
    mapCellsX_(0),
    mapCellsY_(0),
    bWasModified_(false),
    texId_(INVALID_TEX_ID)
{
    mapName_[0] = '\0';
}

//---------------------------------------------------------
// destructor
//---------------------------------------------------------
Map::~Map()
{
    ReleaseMap();
}

//---------------------------------------------------------
// DEBUG: dump indices of the map tiles textures into the console
//        (print a mapCellsX * mapCellsY matrix of tiles indices)
//---------------------------------------------------------
void Map::DumpTiles()
{
    int x, y;

    printf("\nDUMP TILES TEXTURE INDICES:\n");
    for (y = 0; y < mapCellsY_; ++y)
    {
        for (x = 0; x < mapCellsX_; ++x)
        {
            const EntityID id = tiles_[y * mapCellsX_ + x];
            const Entity* pEntt = g_EnttMgr.GetEntt(id);
            printf("%d,", GetTileIdx(pEntt));
        }
        printf("\n");
    }

    printf("\nDUMP CELLS TOP-LEFT EDGES:\n");
    for (y = 0; y < mapCellsY_; ++y)
    {
        for (x = 0; x < mapCellsX_; ++x)
        {
            const EntityID id = tiles_[y * mapCellsX_ + x];
            const Entity* pEntt = g_EnttMgr.GetEntt(id);

            const Tile* pTile = pEntt->GetComponent<Tile>();
            assert(pTile);

            const Recti& r = pTile->GetDstRect();

            printf(" <%4d,%4d> |", r.x0, r.y0);
        }
        printf("\n");
    }

    printf("\n\n");
}

//---------------------------------------------------------
// write into a file map's metadata
//---------------------------------------------------------
void Map::SaveMapCommonInfo(FILE* fp, const char* mapName)
{
    assert(fp);
    assert(mapName && mapName[0] != '\0');

    fprintf(fp, "map:          %s\n", mapName);
    fprintf(fp, "map_size_x:   %d\n", mapCellsX_);
    fprintf(fp, "map_size_y:   %d\n", mapCellsY_);
    fprintf(fp, "tilemap_name: %s\n", g_AssetMgr.GetTexName(texId_));
    fprintf(fp, "tilemap_path: %s\n", g_AssetMgr.GetTexPath(texId_));
}

//---------------------------------------------------------
// write into a file indices of tiles on the tilemap
//---------------------------------------------------------
void Map::SaveMapTilesIndices(FILE* fp)
{
    assert(fp);

    int x;
    fprintf(fp, "\ntiles indices:\n");

    EntityID id = INVALID_ENTT_ID;
    Entity* pEntt = nullptr;

    for (int y = 0; y < mapCellsY_; ++y)
    {
        // mapCellsX-1 bacause after the last index we don't write a comma (",")
        for (x = 0; x < mapCellsX_ - 1; ++x)
        {
            id = tiles_[y * mapCellsX_ + x];
            pEntt = g_EnttMgr.GetEntt(id);
            fprintf(fp, "%d,", GetTileIdx(pEntt));
        }

        id = tiles_[y * mapCellsX_ + x];
        pEntt = g_EnttMgr.GetEntt(id);
        fprintf(fp, "%d\n", GetTileIdx(pEntt));
    }
}

//---------------------------------------------------------
// load map's metadata from file (name, sizes, texture info)
//---------------------------------------------------------
void Map::LoadMapCommonInfo(FILE* fp)
{
    assert(fp);

    int res = 0;
    char tilemapName[MAX_LEN_TEX_NAME];
    char tilemapPath[256];

    res = fscanf(fp, "map: %s\n", mapName_);
    assert(res == 1 && "failed read (map name)");

    res = fscanf(fp, "map_size_x: %d\n", &mapCellsX_);
    assert(res == 1 && "failed read (map size X)");

    res = fscanf(fp, "map_size_y: %d\n", &mapCellsY_);
    assert(res == 1 && "failed read (map size Y)");

    res = fscanf(fp, "tilemap_name: %s\n", tilemapName);
    assert(res == 1 && "failed read (tilemap name)");

    res = fscanf(fp, "tilemap_path: %s\n", tilemapPath);
    assert(res == 1 && "failed read (tilemap path)");

    //
    // setup a tilemap texture
    //
    if (g_AssetMgr.HasTexture(tilemapName))
    {
        texId_ = g_AssetMgr.GetTexId(tilemapName);
        return;
    }

    // such tilemap texture wasn't loaded yet, so load it
    texId_ = g_AssetMgr.LoadTexture(tilemapName, tilemapPath);
    if (texId_ == INVALID_TEX_ID)
    {
        LogErr(LOG, "can't load a texture for map (tex_name: %s, tex_path: %s)", tilemapName, tilemapPath);
    }
}

//---------------------------------------------------------
// create a grid of default tiles with "invalid" texture
//---------------------------------------------------------
void Map::CreateTilesGrid(const int numCellsX, const int numCellsY)
{
    assert(numCellsX > 0);
    assert(numCellsY > 0);

    int         tileIdx = 0;
    const Vec2i srcRectPos = { 0, 0 };                            // top-left corner of a texture tile (subrect) on the atlas

    tiles_.resize(numCellsX * numCellsY, INVALID_ENTT_ID);

    for (int y = 0; y < numCellsY; ++y)
    {
        for (int x = 0; x < numCellsX; ++x)
        {
            // setup name for tile
            char tileName[MAX_LEN_ENTT_NAME]{ '\0' };
            snprintf(tileName, sizeof(tileName), "tile_%d_%d", y, x);

            // create a tile entity and add components
            Entity* pTileEntt = g_EnttMgr.AddEntt(tileName, LAYER_TILEMAP);
            assert(pTileEntt->GetId() != INVALID_ENTT_ID);

            pTileEntt->AddComponent<Tile>(srcRectPos.x,       // left edge of a tile on texture atlas
                                          srcRectPos.y,       // top edge of a tile on texture atlas
                                          MAP_CELL_SIZE * x,  // left edge of a map's cell in world
                                          MAP_CELL_SIZE * y,  // top edge of a map's cell in world
                                          MAP_TILE_SIZE,      // width and height of a tile on texture atlas
                                          MAP_CELL_SIZE,      // width and height of a map's cell in world
                                          INVALID_TEX_ID);

            tiles_[tileIdx] = pTileEntt->GetId();
            ++tileIdx;
        }
    }
}

//---------------------------------------------------------
// load from a file indices of tiles on the tilemap
//---------------------------------------------------------
void Map::LoadMapTilesIndices(FILE* fp)
{
    assert(fp);
    assert(mapCellsX_);
    assert(mapCellsY_);

    // calc the number of tiles (rows and columns) on the map's texture atlas
    const Vec2i numTiles = GetNumTexTiles();

    int count = 0;
    char buf[128];
    fgets(buf, sizeof(buf), fp);

    for (int y = 0; y < mapCellsY_; ++y)
    {
        for (int x = 0; x < mapCellsX_; ++x)
        {
            int idx = -1;

            count = fscanf(fp, "%d", &idx);
            assert(count == 1);

            // skip a comma (,) or new line ('\n') symbol
            fgetc(fp);
            
            // we didn't specify tile index for this cell so skip it (
            if (idx == -1)
                continue;

            SetTileTex(idx % numTiles.x, idx / numTiles.x, x, y);
        }
    }
}

//---------------------------------------------------------
// Desc:  create a new EMPTY map of size mapSizeX x mapSizeY
// Args:  mapName  - a name for this new map
//        mapCells - number of cells by X and Y on the map
//---------------------------------------------------------
bool Map::CreateNewMap(const char* mapName, int mapCellsX, int mapCellsY)
{
    if (!mapName || mapName[0] == 0)
        return false;

    if (strlen(mapName) >= MAX_LEN_MAP_NAME)
        return false;

    if (mapCellsX <= 0 || mapCellsY <= 0)
        return false;


    // save the previous map if we have some modifications for it
    if (bWasModified_)
        SaveMap();

    ReleaseMap();
   
    strcpy(mapName_, mapName);
    mapCellsX_ = mapCellsX;
    mapCellsY_ = mapCellsY;
    bWasModified_ = false;

    // create a cells grid for this map
    CreateTilesGrid(mapCellsX, mapCellsY);

    LogMsg(LOG, "map is created (name: %s, size: %dx%d)", mapName, mapCellsX, mapCellsY);
    return true;
}

//---------------------------------------------------------
// save the current map into its file
//---------------------------------------------------------
void Map::SaveMap()
{
    char filePath[128]{ '\0' };
    strcat(filePath, pathToMapsDir);
    strcat(filePath, mapName_);
    strcat(filePath, ".map");

    SaveMapAs(filePath);
}

//---------------------------------------------------------
// save the current map into a file by input path
//---------------------------------------------------------
void Map::SaveMapAs(const char* filepath)
{
    if (!filepath || filepath[0] == '\0')
    {
        LogErr(LOG, ERR_EMPTY_PATH);
        return;
    }

    FILE* fp = fopen(filepath, "w");
    if (!fp)
    {
        LogErr(LOG, "%s%s\n\tdid you forget to create a map? or you can save map as...", ERR_OPEN_FILE, filepath);
        return;
    }

    char filename[64];
    FileSys::GetFileStem(filepath, filename);

    SaveMapCommonInfo(fp, filename);
    SaveMapTilesIndices(fp);

    fclose(fp);
    LogMsg(LOG, "map is saved into file: %s", filepath);

    LoadMap(filepath);
}

//---------------------------------------------------------
// release all the data related to the current map
// (usually do it before loading of another map)
//---------------------------------------------------------
void Map::ReleaseMap()
{
    if (!tiles_.empty())
    {
        g_EnttMgr.RemoveEntts(tiles_.data(), tiles_.size());
        tiles_.purge();
    }

    mapName_[0] = '\0';
    mapCellsX_ = 0;
    mapCellsY_ = 0;
    bWasModified_ = false;
}

//---------------------------------------------------------
// set map's new size (number of cells by X and Y)
//---------------------------------------------------------
void Map::ResizeMap(Vec2i numCells)
{
    if (numCells.x <= 0 || numCells.y <= 0)
    {
        LogErr(LOG, "invalid number of cells (%dx%d):", numCells.x, numCells.y);
        return;
    }

    // no resize, just go out
    if (numCells.x == mapCellsX_ && numCells.y == mapCellsY_)
        return;


    // number of cells we will copy into the new tiles list
    const Vec2i copyGridSz = { Min(numCells.x, mapCellsX_), Min(numCells.y, mapCellsY_) };

    //
    // 0. save IDs of tiles so we will copy them into a new arr
    //
    cvector<EntityID> oldTiles = std::move(tiles_);

    //
    // 1. create a new set of cells with new size
    //
    tiles_.resize(numCells.x * numCells.y, INVALID_ENTT_ID);

    for (int y = 0; y < copyGridSz.y; ++y)
    {
        for (int x = 0; x < copyGridSz.x; ++x)
        {
            tiles_[y * numCells.x + x] = oldTiles[y * mapCellsX_ + x];

            oldTiles[y * mapCellsX_ + x] = INVALID_ENTT_ID;
        }
    }

    // setup a new number of cells
    mapCellsX_ = numCells.x;
    mapCellsY_ = numCells.y;

    const Vec2i srcRectPos = { 0, 0 };

    //
    // 2. create and setup those tiles which were added
    //
    for (int i = 0; i < (int)tiles_.size(); ++i)
    {
        if (tiles_[i] != INVALID_ENTT_ID)
            continue;

        // cell's coords by X and Y
        const int ix = i % numCells.x;
        const int iy = i / numCells.y;

        char tileName[MAX_LEN_ENTT_NAME];
        sprintf(tileName, "tile_%d_%d", iy, ix);

        Entity* pEntt = g_EnttMgr.AddEntt(tileName, LAYER_TILEMAP);
        assert(pEntt->GetId() != INVALID_ENTT_ID);

        pEntt->AddComponent<Tile>(srcRectPos.x,         // left edge of a tile on texture atlas
                                  srcRectPos.y,         // top edge of a tile on texture atlas
                                  MAP_CELL_SIZE * ix,     // position of left edge of a map cell in the world
                                  MAP_CELL_SIZE * iy,     // position of top edge of a map cell in the world
                                  MAP_TILE_SIZE,        // width and height of a tile on texture atlas
                                  MAP_CELL_SIZE,        // width and height of a cell in the world
                                  INVALID_TEX_ID);      // by default a new tile has "invalid" (checkerboard) texture

        tiles_[i] = pEntt->GetId();
    }

    //
    // 3. remove all the clipped tiles
    //
    for (const EntityID id : oldTiles)
    {
        if (id != INVALID_ENTT_ID)
            g_EnttMgr.SetToRemove(id);
    }
}

//---------------------------------------------------------
// get a rectangle for tile texture by index (will serve as a source rectangle)
//---------------------------------------------------------
Recti Map::CalcSrcRect(const int tileIdxX, const int tileIdxY) const
{
    // check input args
    const Vec2i numTiles = GetNumTexTiles();
    assert(tileIdxX >= 0 && tileIdxX < numTiles.x);
    assert(tileIdxY >= 0 && tileIdxY < numTiles.y);

    Recti r;
    r.x0 = tileIdxX * MAP_TILE_SIZE;
    r.x1 = r.x0 + MAP_TILE_SIZE;
    r.y0 = tileIdxY * MAP_TILE_SIZE;
    r.y1 = r.y0 + MAP_TILE_SIZE;

    return r;
}

//---------------------------------------------------------
// get a rectangle for cell on the map (will serve as a destination rectangle)
//---------------------------------------------------------
Recti Map::CalcDstRect(const int cellIdxX, const int cellIdxY) const
{
    assert(cellIdxX >= 0 && cellIdxX < mapCellsX_);
    assert(cellIdxY >= 0 && cellIdxY < mapCellsY_);

    Recti r;
    r.x0 = cellIdxX * MAP_CELL_SIZE;
    r.x1 = r.x0 + MAP_CELL_SIZE;
    r.y0 = cellIdxY * MAP_CELL_SIZE;
    r.y1 = r.y0 + MAP_CELL_SIZE;

    return r;
}

//---------------------------------------------------------
// get a number of tiles (rows and columns) on the map's texture atlas
//---------------------------------------------------------
Vec2i Map::GetNumTexTiles() const
{
    const Vec2i texSz = g_AssetMgr.GetTexSize(texId_);
    return { texSz.w / MAP_TILE_SIZE, texSz.h / MAP_TILE_SIZE };
}

//---------------------------------------------------------
// get an index of input tile on the current tilemap
//---------------------------------------------------------
int Map::GetTileIdx(const Entity* pTileEntt) const
{
    assert(pTileEntt);

    const Tile* pTile = pTileEntt->GetComponent<Tile>();
    assert(pTile);

    // a texture isn't specified for this tile yet
    if (pTile->GetTexture() == INVALID_TEX_ID)
        return -1;

    const Recti& r   = pTile->GetSrcRect();
    const Vec2i  idx = { r.x0 / MAP_TILE_SIZE, r.y0 / MAP_TILE_SIZE };

    return (idx.y * GetNumTexTiles().x) + idx.x;
}

//---------------------------------------------------------
// setup a tile (texture) for a cell on the map
//---------------------------------------------------------
void Map::SetTileTex(int texTileIdxX, int texTileIdxY, 
                     int mapCellIdxX, int mapCellIdxY)
{
    if (texTileIdxX < 0 || texTileIdxY < 0)
    {
        LogErr(LOG, "invalid texture tile index (%d:%d)", texTileIdxX, texTileIdxY);
        return;
    }
    if (mapCellIdxX < 0 || mapCellIdxX >= mapCellsX_ ||
        mapCellIdxY < 0 || mapCellIdxY >= mapCellsY_)
    {
        LogErr(LOG, "invalid map cell index (%d:%d)", mapCellIdxX, mapCellIdxY);
        return;
    }

    // get a tile to setup
    const EntityID id = tiles_[(mapCellIdxY * mapCellsX_) + mapCellIdxX];
    Entity* pTile = g_EnttMgr.GetEntt(id);
    assert(pTile);

    Tile* pTileComp = pTile->GetComponent<Tile>();
    assert(pTileComp);

    pTileComp->SetSrcRect(CalcSrcRect(texTileIdxX, texTileIdxY));
    pTileComp->SetDstRect(CalcDstRect(mapCellIdxX, mapCellIdxY));
    pTileComp->SetTexture(texId_);
}

//---------------------------------------------------------
// setup a texture for the whole map
//---------------------------------------------------------
void Map::SetTexture(TexID texId)
{
    if (texId != INVALID_TEX_ID)
        texId_ = texId;
}

//---------------------------------------------------------
// read the map definitions from the .map file
//---------------------------------------------------------
void Map::LoadMap(const char* filePath)
{
    assert(filePath && filePath[0] != '\0');

    FILE* fp = fopen(filePath, "r");
    if (!fp)
    {
        LogErr(LOG, "%s%s", ERR_OPEN_FILE, filePath);
        return;
    }

    // check if the file isn't empty
    fseek(fp, 0, SEEK_END);
    long fileSz = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (fileSz == 0)
    {
        LogFatal(LOG, "your map file is empty! (%s)", filePath);
    }

    // save the previous map if we have some modifications for it
    if (bWasModified_)
        SaveMap();

    ReleaseMap();
    LoadMapCommonInfo(fp);

    CreateTilesGrid(mapCellsX_, mapCellsY_);
    LoadMapTilesIndices(fp);

    fclose(fp);
}