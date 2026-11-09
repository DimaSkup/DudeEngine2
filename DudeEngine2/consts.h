/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: consts.h
    Desc:     a bunch of constants values used everywhere
    Created:  by DimaSkup
\**********************************************************************************/
#ifndef CONSTS_H
#define CONSTS_H

const int FPS = 60;
const int FRAME_TARGET_TIME = 1000 / FPS;

constexpr int MAX_LEN_MAP_NAME            = 32;
constexpr int MAX_LEN_ENTT_NAME           = 32;
constexpr int MAX_LEN_COMPONENT_NAME      = 16;
constexpr int MAX_LEN_TEX_NAME            = 32;
constexpr int MAX_LEN_SOUND_NAME          = 16;
constexpr int MAX_LEN_FILE_PATH           = 64;
constexpr int MAX_LEN_EVENT_NAME          = 32;
constexpr int MAX_LEN_EVENT_LISTENER_NAME = 32;
constexpr int MAX_LEN_WEAPON_NAME         = 32;
constexpr int MAX_CHILD_ENTTS             = 4;      // is used in the Hierarchy component 
constexpr int MAX_LEN_GAME_LVL_NAME       = 32;

constexpr int NUM_ALL_WEAPONS = 8;          // limit for number of all the weapons

constexpr int MAP_CELL_SIZE = 128;          // size of a single cell on the map
constexpr int MAP_TILE_SIZE = 64;           // size of a single tile on the texture atlas (for map)

constexpr int INVALID_TEX_ID = 0;
constexpr int INVALID_ENTT_ID = 0;
constexpr int INVALID_EVENT_ID = 0;
constexpr int INVALID_ANIM_ID = 0;
constexpr int INVALID_SOUND_ID = 0;

#define ENGINE_NAME "DudeEngine2\\"
#define DEFAULT_LOCALE "ua"

static const char* AVAIL_LOCALES[] = { "en", "ru", "ua" };

//
// paths
//
static const char PATH_LEVELS_DIR[]      = "assets/levels/";
static const char defaultLevelName[]     = "sandbox";
static const char pathToAssetsDir[]      = "assets/";
static const char pathToMapsDir[]        = "assets/maps/";
static const char pathToImgDir[]         = "assets/images/";
//static const char pathToLevelsDir[]      = "assets/levels/";
static const char pathToImgTilemapsDir[] = "assets/images/maps/";
static const char pathToScriptsDir[]     = "assets/scripts/";

//
// errors
//
static const char ERR_INV_FILE[]   = "you've opened a wrong file";
static const char ERR_EMPTY_NAME[] = "empty name";
static const char ERR_EMPTY_PATH[] = "empty path";
static const char ERR_OPEN_FILE[]  = "can't open a file: ";
static const char ERR_EMPTY_STR[]  = "empty str";
static const char ERR_EMPTY_KEY[]  = "empty key";

#endif
