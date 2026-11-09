/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   types.h
    Desc:       typedefs which are used everywhere
    Created:    by DimaSkup
\**********************************************************************************/
#ifndef TYPES_H
#define TYPES_H

#include "consts.h"
#include <inttypes.h>

typedef int8_t          int8;
typedef uint8_t			uint8;
typedef uint16_t		uint16;
typedef uint32_t		uint32;
typedef unsigned int	uint;

typedef uint16_t        AnimID;
typedef uint16_t        TexID;
typedef uint32_t        EntityID;
typedef uint16_t        SoundID;
typedef uint16_t        EventID;
typedef uint8_t         WeaponID;

struct TextureName
{
    char name[MAX_LEN_TEX_NAME];
};

struct SoundName
{
    char name[MAX_LEN_SOUND_NAME];
};

struct EventName
{
    char name[MAX_LEN_EVENT_NAME];
};

struct FilePath
{
    char path[MAX_LEN_FILE_PATH];
};

struct PrefabName
{
    char name[MAX_LEN_ENTT_NAME];
};

struct Color
{
    union
    {
        uint32 rgba;
        struct { uint8 r, g, b, a; };
    };
};

// used for registration of C/C++ enumerations in the Lua as a bunch of globals
struct EnumValue
{
    const char* name;
    int         value;
};

struct Animation
{
    AnimID id;
    TexID  texId;            // texture is used for this animation
    uint16 speed;            // frame time ms
    uint8  numFrames;        // number of frames for this animation
    uint8  bLooped     : 1;  // can we repeat this animation during the time?
    uint8  bPlayedOnce : 1;
};

// layers of the scene: rendering is done in the same order as enumerated layers here
enum eLayerType : uint8
{
    LAYER_TILEMAP = 0,   // map tiles: walls, ground, floor
    LAYER_GND_DECAL,     // ground decals: grass, fallen leaves, blood, etc.
    LAYER_ENTITY,        // scene objects (we will sort them by Y to make illusion of depth): player, enemies, trees, etc.
    LAYER_UI,            // UI elements (draw over all the other stuff)
    NUM_LAYERS,
};

static const char* s_LayerNames[] = {
    "tilemap",
    "gnd_decal",
    "entity",
    "ui"
};

struct EventData
{
    EntityID enttId;        // event happened relatively to this entity (DAMAGE, DIE, HEAL, etc.)
    EntityID triggeredBy;   // is set when some entity triggers a trigger which run this event (so we store ID of this entity)

    union {
        struct { int   ix, iy, iz, iw; };
        struct { float fx, fy, fz, fw; };
        struct { char  text[16]; };
    };
};

enum eRendererFlip : uint8
{
    R_FLIP_NONE         = 0,    // no flip
    R_FLIP_HORIZONTAL   = 1,    // flip horizontally
    R_FLIP_VERTICAL     = 2,    // flip vertically
    NUM_RENDER_FLIP_MODES
};

//---------------------------------------------------------
// data of a single weapon
//---------------------------------------------------------
struct Weapon
{
    float       cooldownSec;
    uint16      attackRadius;
    WeaponID    id;
    uint8       damage;
};

struct WeaponName
{
    char        name[MAX_LEN_WEAPON_NAME];
};

#endif
