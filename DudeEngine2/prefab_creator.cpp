/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   prefab_creator.cpp
    Desc:       manually create prefabs 
    Created:    08.06.2026 by DimaSkup
\**********************************************************************************/
#include "prefab_creator.h"
#include "asset_mgr.h"
#include "entity_mgr.h"
#include "components/transform.h"
#include "components/sprite.h"
#include "components/damage.h"
#include "components/health.h"
#include "components/keyboard.h"
#include "components/enemy.h"
#include "components/collider.h"
#include "components/trigger.h"
#include "components/life_timer.h"
#include "components/hierarchy.h"

#include "entity_save_load.h"


//---------------------------------------------------------
// forward declaration of internal functions
//---------------------------------------------------------

// test
void CreateTestPrefabs();

// enemies
void CreatePrefabEnemySkeleton();
void CreatePrefabEnemySpider();
void CreatePrefabEnemyBat();
void CreatePrefabEnemyHuman();

// colliders
void CreatePrefabWallBoxCollider();

// props
void CreatePrefabsTrees();
void CreatePrefabsBranches();
void CreatePrefabsBushes();
void CreatePrefabsGrass();

void CreateBarrelPrefab();
void CreateScarecrowPrefab();
void CreateChestPrefabs();
void CreateStrawPrefab();
void CreateCastle01Prefab();
void CreatePrefabCastleBroken();
void CreatePrefabsGraves();

void CreatePrefabGravePit();
void CreatePrefabCampfire();

void CreatePrefabSunflower();

void CreatePrefabExplosion();
void CreatePrefabRainbow();
void CreatePrefabPointLight();
void CreatePrefabEmptyTrigger();

// ground decals
void CreatePentagram();
void CreateGroundDecals();

void SavePrefabs();
void LoadPrefabs();

//---------------------------------------------------------
// main method
//---------------------------------------------------------
void CreateAllPrefabs()
{
#if 1
    // test
    CreateTestPrefabs();

    // nature
    CreatePrefabsTrees();
    CreatePrefabsBranches();
    CreatePrefabsBushes();
    CreatePrefabsGrass();
    CreatePrefabSunflower();

    // enemies
    CreatePrefabEnemySkeleton();
    CreatePrefabEnemySpider();
    CreatePrefabEnemyBat();
    CreatePrefabEnemyHuman();

    // buildings
    CreateCastle01Prefab();
    CreatePrefabCastleBroken();

    // props
    CreatePrefabsGraves();
    CreatePrefabGravePit();
    CreatePrefabCampfire();
    CreateBarrelPrefab();
    CreateScarecrowPrefab();
    CreateStrawPrefab();
    CreateChestPrefabs();


    CreatePrefabWallBoxCollider();

    CreatePrefabExplosion();
    CreatePrefabRainbow();
    CreatePrefabPointLight();

    CreatePrefabEmptyTrigger();
    CreatePentagram();
    CreateGroundDecals();

    //SavePrefabs();
#else

    LoadPrefabs();

#endif
}

//---------------------------------------------------------
//---------------------------------------------------------
void SavePrefabs()
{
    const char* filepath = "assets/common/prefabs.entt";

    EntitySaveLoad saver;
    FILE* fp = fopen(filepath, "w");
    if (!fp)
        LogFatal(LOG, "%s%s", ERR_OPEN_FILE, filepath);

    fputs(";===============================================\n", fp);
    fputs("; declare prefabs here\n", fp);
    fputs(";===============================================\n", fp);
    fputs("prefabs {\n", fp);

    const cvector<Entity*>& prefabs = g_EnttMgr.GetPrefabs();

    // start from 1 to skip an 'invalid_prefab' by index == 0
    for (index i = 1; i < prefabs.size(); ++i)
        saver.SaveEntt(fp, prefabs[i]);

    fputc('}', fp);

    fclose(fp);
}

//---------------------------------------------------------
//---------------------------------------------------------
void LoadPrefabs()
{
    const char* filepath = "assets/common/prefabs.entt";

    EntitySaveLoad loader;
    FILE* fp = fopen(filepath, "r");
    if (!fp)
    {
        LogErr(LOG, "%s%s", ERR_OPEN_FILE, filepath);
        return;
    }

    char buf[128];
    char prefabName[MAX_LEN_ENTT_NAME];
    char layerName[32];
    int count = 0;

    // skip comments section
    while (fgets(buf, sizeof(buf), fp))
    {
        if (buf[0] != ';')
            break;
    }

    // check if we read a valid file
    if (strncmp(buf, "prefabs", 7) != 0)
    {
        LogErr(LOG, "%s: %s", ERR_INV_FILE, filepath);
        fclose(fp);
        return;
    }


    // read in declarations of entities
    while (fgets(buf, sizeof(buf), fp))
    {
        // +1 to skip '\t'
        if (strncmp(buf + 1, "entt", 4) != 0)
            continue;

        // get name for a new entity
        count = sscanf(buf + 1, "entt %s", prefabName);
        if (count != 1)
        {
            LogErr(LOG, "err read prefabs's name from str: %s", buf + 1);
            continue;
        }

        // get a layer for a new entity
        fgets(buf, sizeof(buf), fp);
        count = sscanf(buf + 2, "layer: %s", layerName);
        if (count != 1)
        {
            LogErr(LOG, "err read entt's layer name from str: %s", buf + 2);
            continue;
        }

        const eLayerType layer = g_EnttMgr.GetLayerType(layerName);
        if (layer == NUM_LAYERS)
        {
            LogErr(LOG, "err read layer's type by layer name: %s", layerName);
            continue;
        }

        Entity* pEntt = g_EnttMgr.AddPrefab(prefabName, layer);
        loader.LoadEntt(fp, pEntt);
    }

    fclose(fp);
}

//---------------------------------------------------------
// create an entity which will be used as a prefab for skeletons enemies
//---------------------------------------------------------
void CreatePrefabEnemySkeleton()
{
    char prefabName[] = "skeleton_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    // load textures
    const TexID idleTexId   = g_AssetMgr.GetTexId("skeleton_idle");
    const TexID moveTexId   = g_AssetMgr.GetTexId("skeleton_move");
    const TexID attackTexId = g_AssetMgr.GetTexId("skeleton_attack");
    const TexID deathTexId  = g_AssetMgr.GetTexId("skeleton_death");
    const TexID damageTexId = g_AssetMgr.GetTexId("skeleton_damage");
    const TexID blockTexId  = g_AssetMgr.GetTexId("skeleton_block");

    // create animations
    const int idleAnimSpeedMs   = 300;
    const int idleNumFrames     = 3;
    const int moveAnimSpeedMs   = 100;
    const int moveNumFrames     = 4;
    const int attackAnimSpeedMs = 200;
    const int attackNumFrames   = 3;
    const int deathAnimSpeedMs  = 150;
    const int deathNumFrames    = 4;
    const int damageAnimSpeedMs = 100;
    const int damageNumFrames   = 3;
    const int blockAnimSpeedMs  = 100;
    const int blockNumFrames    = 3;
    const bool bLooped = true;

    if (!g_AssetMgr.HasAnimByName("skeleton_idle"))
        g_AssetMgr.AddAnim("skeleton_idle",   idleTexId,   idleAnimSpeedMs,    idleNumFrames,   bLooped);

    if (!g_AssetMgr.HasAnimByName("skeleton_move"))
        g_AssetMgr.AddAnim("skeleton_move",   moveTexId,   moveAnimSpeedMs,    moveNumFrames,   bLooped);

    if (!g_AssetMgr.HasAnimByName("skeleton_attack"))
        g_AssetMgr.AddAnim("skeleton_attack", attackTexId, attackAnimSpeedMs,  attackNumFrames, bLooped);

    if (!g_AssetMgr.HasAnimByName("skeleton_death"))
        g_AssetMgr.AddAnim("skeleton_death",  deathTexId,  deathAnimSpeedMs,   deathNumFrames,  !bLooped);

    if (!g_AssetMgr.HasAnimByName("skeleton_damage"))
        g_AssetMgr.AddAnim("skeleton_damage", damageTexId, damageAnimSpeedMs,  damageNumFrames, bLooped);

    if (!g_AssetMgr.HasAnimByName("skeleton_block"))
        g_AssetMgr.AddAnim("skeleton_block",  blockTexId,  blockAnimSpeedMs,   blockNumFrames,  bLooped);

    //
    // setup prefab's init params
    //

    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 32, 32 };
    const int scale = 4;

    // sprite
    const bool bFixedOnScreen = false;

    // health / damage
    const int currHP = 100;
    const int maxHP = 100;
    const int healPerSec = 0;
    const float attackCooldownSec = 1.0f;
    const float damage = 10.0f;

    // collider
    const Vec2i colliderCenter = { 62, 103 };
    const int colliderRadius = 15;


    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);

    // add components
    Transform& t = pPrefab->AddComponent<Transform>(p, sz, scale);

    Sprite& spr = pPrefab->AddComponent<Sprite>(
        g_AssetMgr.GetAnim("skeleton_idle"),
        g_AssetMgr.GetAnim("skeleton_move"),
        g_AssetMgr.GetAnim("skeleton_attack"),
        g_AssetMgr.GetAnim("skeleton_death"),
        g_AssetMgr.GetAnim("skeleton_damage"),
        g_AssetMgr.GetAnim("skeleton_block"),
        bFixedOnScreen);

    pPrefab->AddComponent<Enemy>(ENEMY_TYPE_SKELETON);
    pPrefab->AddComponent<Health>(currHP, maxHP, healPerSec);
    pPrefab->AddComponent<Damage>(attackCooldownSec, damage);
    pPrefab->AddComponent<Collider>(colliderCenter, colliderRadius);

    spr.SetSrcRectSize(sz);
    spr.PlayAnimation(ANIM_TYPE_MOVE);
}

//---------------------------------------------------------
// create an entity which will be used as a prefab for spiders enemies
//---------------------------------------------------------
void CreatePrefabEnemySpider()
{
    char prefabName[] = "spider_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    // load textures
    const TexID idleTexId   = g_AssetMgr.GetTexId("spider_idle");
    const TexID moveTexId   = g_AssetMgr.GetTexId("spider_move");
    const TexID attackTexId = g_AssetMgr.GetTexId("spider_attack");
    const TexID deathTexId  = g_AssetMgr.GetTexId("spider_death");
    const TexID damageTexId = g_AssetMgr.GetTexId("spider_damage");
    const TexID blockTexId  = g_AssetMgr.GetTexId("spider_block");

    // create animations
    const int idleAnimSpeedMs   = 100;
    const int idleNumFrames     = 2;
    const int moveAnimSpeedMs   = 100;
    const int moveNumFrames     = 12;
    const int attackAnimSpeedMs = 100;
    const int attackNumFrames   = 2;
    const int deathAnimSpeedMs  = 100;
    const int deathNumFrames    = 8;
    const int damageAnimSpeedMs = 100;
    const int damageNumFrames   = 2;
    const int blockAnimSpeedMs  = 100;
    const int blockNumFrames    = 2;
    const bool bLooped = true;

    if (!g_AssetMgr.HasAnimByName("spider_idle"))
        g_AssetMgr.AddAnim("spider_idle", idleTexId, idleAnimSpeedMs, idleNumFrames,   bLooped);

    if (!g_AssetMgr.HasAnimByName("spider_move"))
        g_AssetMgr.AddAnim("spider_move",  moveTexId,  moveAnimSpeedMs, moveNumFrames,   bLooped);

    if (!g_AssetMgr.HasAnimByName("spider_attack"))
        g_AssetMgr.AddAnim("spider_attack", attackTexId, attackAnimSpeedMs, attackNumFrames, bLooped);

    if (!g_AssetMgr.HasAnimByName("spider_death"))
        g_AssetMgr.AddAnim("spider_death",  deathTexId,  deathAnimSpeedMs,  deathNumFrames,  !bLooped);

    if (!g_AssetMgr.HasAnimByName("spider_damage"))
        g_AssetMgr.AddAnim("spider_damage", damageTexId, damageAnimSpeedMs, damageNumFrames, bLooped);

    if (!g_AssetMgr.HasAnimByName("spider_block"))
        g_AssetMgr.AddAnim("spider_block",  blockTexId,  blockAnimSpeedMs,  blockNumFrames,  bLooped); 

    //
    // setup prefab's init params
    //

    // transform (position, size, scale)
    const Vec2 p = { 0, 0 };
    const Vec2i sz = { 32, 32 };
    const int scale = 4;

    // sprite
    const bool bFixedOnScreen = false;

    // health / damage
    const int currHP = 100;
    const int maxHP = 100;
    const int healPerSec = 1;
    const float attackCooldownSec = 1.0f;
    const float damage = 10.0f;

    // collider
    const Vec2i colliderCenter = { 64, 64 };
    const int colliderRadius = 32;

    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);

    // add components
    Transform& t = pPrefab->AddComponent<Transform>(p, sz, scale);

    Sprite& spr = pPrefab->AddComponent<Sprite>(
        g_AssetMgr.GetAnim("spider_idle"),
        g_AssetMgr.GetAnim("spider_move"),
        g_AssetMgr.GetAnim("spider_attack"),
        g_AssetMgr.GetAnim("spider_death"),
        g_AssetMgr.GetAnim("spider_damage"),
        g_AssetMgr.GetAnim("spider_block"),
        bFixedOnScreen);

    spr.SetSrcRectSize(sz);
    spr.PlayAnimation(ANIM_TYPE_MOVE);

    pPrefab->AddComponent<Enemy>(ENEMY_TYPE_SPIDER);
    pPrefab->AddComponent<Health>(currHP, maxHP, healPerSec);
    pPrefab->AddComponent<Damage>(attackCooldownSec, damage);
    pPrefab->AddComponent<Collider>(colliderCenter, colliderRadius);

}

//---------------------------------------------------------
// create an entity which will be used as a prefab for bats enemies
//---------------------------------------------------------
void CreatePrefabEnemyBat()
{
    const char* prefabName = "bat_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    const TexID idleTexId   = g_AssetMgr.GetTexId("bat_idle");
    const TexID deathTexId  = g_AssetMgr.GetTexId("bat_death");
    const TexID shadowTexId = g_AssetMgr.GetTexId("bat_shadow");

    // create animations
    const int idleAnimSpeedMs = 100;
    const int idleNumFrames   = 8;
    const int deathAnimSpeedMs = 72;
    const int deathNumFrames   = 11;
    const bool bLooped = true;

    if (!g_AssetMgr.HasAnimByName("bat_idle"))
        g_AssetMgr.AddAnim("bat_idle", idleTexId, idleAnimSpeedMs, idleNumFrames, bLooped);

    if (!g_AssetMgr.HasAnimByName("bat_death"))
        g_AssetMgr.AddAnim("bat_death", deathTexId, deathAnimSpeedMs, deathNumFrames, !bLooped);

    //
    // setup prefab's init params
    //

    // transform (position, size, scale)
    const Vec2 p = { 0, 0 };
    const Vec2i sz = { 32, 32 };
    const int scale = 4;

    // sprite 
    const bool bFixedOnScreen = false;

    // health / damage
    const int currHP = 100;
    const int maxHP = 100;
    const int healPerSec = 1;
    const float attackCooldownSec = 1.0f;
    const float damage = 10.0f;

    // collider
    const Vec2i colliderCenter = { 56, 64 };
    const int colliderRadius = 19;

    const Animation& idleAnim = g_AssetMgr.GetAnim("bat_idle");
    const Animation& deathAnim = g_AssetMgr.GetAnim("bat_death");

    //
    // create prefab and add component
    //
    Entity* pBat = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);

                  pBat->AddComponent<Transform>(p, sz, scale);
                  pBat->AddComponent<Enemy>(ENEMY_TYPE_BAT);
                  pBat->AddComponent<Health>(currHP, maxHP, healPerSec);
                  pBat->AddComponent<Damage>(attackCooldownSec, damage);

    Collider& c = pBat->AddComponent<Collider>(colliderCenter, colliderRadius);
    Sprite& spr = pBat->AddComponent<Sprite>(idleAnim,
                                             idleAnim,           // move
                                             idleAnim,           // attack
                                             deathAnim,
                                             idleAnim,           // damage
                                             idleAnim,           // block
                                             bFixedOnScreen);

    Hierarchy& batHierarchy = pBat->AddComponent<Hierarchy>();

    c.SetSolid(false);
    spr.SetSrcRectSize(sz);
    spr.PlayAnimation(ANIM_TYPE_MOVE);


    // create a prefab for bat's shadow (will be a child entity for the bat)
    Entity* pShadow = g_EnttMgr.AddPrefab("bat_shadow", LAYER_GND_DECAL);
                               pShadow->AddComponent<Transform>(p, sz, scale);
    Hierarchy& shadHierarchy = pShadow->AddComponent<Hierarchy>();
    Sprite&        shadowSpr = pShadow->AddComponent<Sprite>(shadowTexId, bFixedOnScreen);
    
    shadowSpr.SetSrcRectSize(sz);
    batHierarchy.AddChild(pShadow);
    shadHierarchy.SetRelativePos(10, 70);
}

//---------------------------------------------------------
// create prefabs for human enemies instantiation
//---------------------------------------------------------
void CreatePrefabEnemyHuman()
{
    const char* prefabName = "human01_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    const TexID texId = g_AssetMgr.GetTexId("human01");

    // transform (position, size, scale)
    const Vec2 p = { 0, 0 };
    const Vec2i sz = { 96,96 };
    const int scale = 2;
    const bool bFixedOnScreen = false;

    // add a prefab, bind components to it
    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);
    
                  pPrefab->AddComponent<Transform>(p, sz, scale);
    Sprite& spr = pPrefab->AddComponent<Sprite>(texId, bFixedOnScreen);

    spr.SetSrcRectSize(sz);
}

//---------------------------------------------------------
// create an entity which will be used as a prefab 
// for wall colliders, or level edges colliders
//---------------------------------------------------------
void CreatePrefabWallBoxCollider()
{
    const char* prefabName = "wall_collider_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    const Vec2 pos = { 0,0 };
    const Vec2i sz = { 128, 128 };
    const int scale = 1;

    const Vec2i colliderPos = { 0,0 };
    const Vec2i colliderSz = { 128, 128 };

    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);
    pPrefab->AddComponent<Transform>(pos, sz, scale);
    pPrefab->AddComponent<Collider>(colliderPos, colliderSz);
}


//---------------------------------------------------------
// create an entity which will be used as a prefab for explosions
//---------------------------------------------------------
void CreatePrefabExplosion()
{
    const char prefabName[] = "explosion0_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;


    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 64, 64 };
    const int scale = 4;

    // sprite + animation
    const TexID      texId = g_AssetMgr.GetTexId("explosion0");
    const bool       bFixedOnScreen = false;
    const int        fullAnimTimeMs = 450;
    const int        numFrames = 10;
    const int        animSpeedMs = fullAnimTimeMs / numFrames;
    const float      lifeTimeSec = (float)(fullAnimTimeMs) * 0.001f;  // kill the enemy after....
    const char*      animName = "explosion_0";
    const bool       bLooped = true;

    if (!g_AssetMgr.HasAnimByName(animName))
        g_AssetMgr.AddAnim(animName, texId, animSpeedMs, numFrames, bLooped);

    const Animation& anim = g_AssetMgr.GetAnim(animName);


    // create a prefab and add components
    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);

                   pPrefab->AddComponent<Transform>(p, sz, scale);
                   pPrefab->AddComponent<LifeTimer>(lifeTimeSec);
    Sprite* spr = &pPrefab->AddComponent<Sprite>(anim, bFixedOnScreen);
    spr->SetSrcRectSize(sz);
}

//---------------------------------------------------------
// create an entity which will be used as a prefab for rainbows
//---------------------------------------------------------
void CreatePrefabRainbow()
{
    const char* prefabName = "rainbow0_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    const TexID explodeTexId = g_AssetMgr.GetTexId("rainbow0");

    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 32, 96 };
    const int scale = 4;
    const bool bFixedOnScreen = true;

    // create animation
    const int   fullAnimTimeMs = 450;
    const int   numFrames = 19;
    const int   animSpeedMs = fullAnimTimeMs / numFrames;
    const float lifeTimeSec = (float)(fullAnimTimeMs) * 0.001f;
    const bool  bLooped = true;

    if (!g_AssetMgr.HasAnimByName("rainbow_0"))
        g_AssetMgr.AddAnim("rainbow_0", explodeTexId, animSpeedMs, numFrames, bLooped);

    // create prefab and add components
    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);
    
                   pPrefab->AddComponent<Transform>(p, sz, scale);
                   pPrefab->AddComponent<LifeTimer>(lifeTimeSec);
    Sprite* spr = &pPrefab->AddComponent<Sprite>(g_AssetMgr.GetAnim("rainbow_0"), !bFixedOnScreen);
    spr->SetSrcRectSize(sz);
}

//---------------------------------------------------------
// create an entity which will be used as a prefab for point lights
//---------------------------------------------------------
void CreatePrefabPointLight()
{
    const char* prefabName = "pointL_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    const TexID texId = g_AssetMgr.GetTexId("point_light");

    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 128, 128 };
    const int scale = 5;
    const bool bFixedOnScreen = false;

    // create prefab and add components
    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);

                  pPrefab->AddComponent<Transform>(p, sz, scale);
                  pPrefab->AddComponent<LifeTimer>(0.5f);
    Sprite& spr = pPrefab->AddComponent<Sprite>(texId, bFixedOnScreen);

    spr.SetColor(225, 255, 255, 255);
    spr.SetSrcRectSize(sz);
}

//---------------------------------------------------------
// create an entity which will be used as a prefab for sunflowers
//---------------------------------------------------------
void CreatePrefabSunflower()
{
    const char* prefabName = "sunflower_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    const TexID atlasTexId = g_AssetMgr.GetTexId("entts_atlas");

    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 32, 32 };
    const int scale = 4;
    const bool bFixedOnScreen = true;

    // collider
    const Vec2i colliderCenter = { 64, 81 };
    const int colliderRadius = 33;


    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);

    // add and setup components
                  pPrefab->AddComponent<Transform>(p, sz, scale);
                  pPrefab->AddComponent<Collider>(colliderCenter, colliderRadius);
    Sprite& spr = pPrefab->AddComponent<Sprite>(atlasTexId, !bFixedOnScreen);

    spr.SetSrcRectPos({ 0, 160 });
    spr.SetSrcRectSize(sz);
}

//---------------------------------------------------------
// create an entity which will be used as a prefab for barrels
//---------------------------------------------------------
void CreateBarrelPrefab()
{
    const char* prefabName = "barrel_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    const TexID atlasTexId = g_AssetMgr.GetTexId("entts_atlas");

    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 32, 32 };
    const int scale = 4;
    const bool bFixedOnScreen = true;

    // collider
    const Vec2i colliderCenter = { 64, 81 };
    const int colliderRadius = 33;

    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);

    // add and setup components
                  pPrefab->AddComponent<Transform>(p, sz, scale);
                  pPrefab->AddComponent<Collider>(colliderCenter, colliderRadius);
    Sprite& spr = pPrefab->AddComponent<Sprite>(atlasTexId, !bFixedOnScreen);

    spr.SetSrcRectPos({ 0, 0 });
    spr.SetSrcRectSize({ 32, 32 });
}

//---------------------------------------------------------
// create an entity which will be used as a prefab for scarecrows
//---------------------------------------------------------
void CreateScarecrowPrefab()
{
    const char* prefabName = "scarecrow_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    const TexID atlasTexId = g_AssetMgr.GetTexId("entts_atlas");

    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 32, 32 };
    const int scale = 4;
    const bool bFixedOnScreen = true;

    // collider
    const Vec2i colliderCenter = { 62, 103 };
    const int colliderRadius = { 15 };

    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);

    // add and setup components
                  pPrefab->AddComponent<Transform>(p, sz, scale);
                  pPrefab->AddComponent<Collider>(colliderCenter, colliderRadius);
    Sprite& spr = pPrefab->AddComponent<Sprite>(atlasTexId, !bFixedOnScreen);

    spr.SetSrcRectPos({ 32, 0 });
    spr.SetSrcRectSize(sz);
}

//---------------------------------------------------------
// create an entity which will be used as a prefab for chests
//---------------------------------------------------------
void CreateChestPrefabs() 
{
    const char* nameChestClosed = "chest_closed_";
    const char* nameChestOpened = "chest_opened_";

    if (g_EnttMgr.HasPrefab(nameChestClosed) && 
        g_EnttMgr.HasPrefab(nameChestClosed))
        return;

    const TexID atlasTexId = g_AssetMgr.GetTexId("entts_atlas");

    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 32, 32 };
    const int scale = 4;
    const bool bFixedOnScreen = true;

    // collider
    const Vec2i colliderPos = { 27, 65 };
    const Vec2i colliderSz = { 70, 30 };

    Entity* pChestClosed = g_EnttMgr.AddPrefab(nameChestClosed, LAYER_ENTITY);
    Entity* pChestOpened = g_EnttMgr.AddPrefab(nameChestOpened, LAYER_ENTITY);

    // add components
    pChestClosed->AddComponent<Transform>(p, sz, scale);
    pChestOpened->AddComponent<Transform>(p, sz, scale);

    pChestClosed->AddComponent<Collider>(colliderPos, colliderSz);
    pChestOpened->AddComponent<Collider>(colliderPos, colliderSz);

    Sprite& sprClosed = pChestClosed->AddComponent<Sprite>(atlasTexId, !bFixedOnScreen);
    Sprite& sprOpened = pChestOpened->AddComponent<Sprite>(atlasTexId, !bFixedOnScreen);

    sprClosed.SetSrcRectPos({ 0, 32 });
    sprClosed.SetSrcRectSize(sz);
    sprOpened.SetSrcRectPos({ 32, 32 });
    sprOpened.SetSrcRectSize(sz);
}

//---------------------------------------------------------
// create an entity which will be used as a prefab for straw
//---------------------------------------------------------
void CreateStrawPrefab() 
{
    const char* prefabName = "straw_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    const TexID atlasTexId = g_AssetMgr.GetTexId("entts_atlas");

    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 32, 32 };
    const int scale = 4;
    const bool bFixedOnScreen = true;

    // collider
    const Vec2i colliderCenter = { 62, 81 };
    const int colliderRadius = { 36 };

    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);

    // add components
                  pPrefab->AddComponent<Transform>(p, sz, scale);
                  pPrefab->AddComponent<Collider>(colliderCenter, colliderRadius);
    Sprite& spr = pPrefab->AddComponent<Sprite>(atlasTexId, !bFixedOnScreen);

    spr.SetSrcRectPos({ 64, 0 });
    spr.SetSrcRectSize(sz);
}

//---------------------------------------------------------
// create an entity which will be used as a prefab for castle 
//---------------------------------------------------------
void CreateCastle01Prefab()
{
    const char* prefabName = "castle_01_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    const TexID texId = g_AssetMgr.GetTexId("castle_01");

    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 768, 768 };
    const int scale = 1;
    const bool bFixedOnScreen = false;

    // collider
    const Vec2i colliderPos = { 38, 556 };
    const Vec2i colliderSz = { 703, 208 };

    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);

    // add components
                  pPrefab->AddComponent<Transform>(p, sz, scale);
                  pPrefab->AddComponent<Collider>(colliderPos, colliderSz);
    Sprite& spr = pPrefab->AddComponent<Sprite>(texId, bFixedOnScreen);

    spr.SetSrcRectPos({ 0, 0 });
    spr.SetSrcRectSize({ 128, 128 });
}

//---------------------------------------------------------
// create a prefab for castles instantiation
//---------------------------------------------------------
void CreatePrefabCastleBroken()
{
    const char* prefabName = "castleBroken01_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    const TexID texId = g_AssetMgr.GetTexId("castle_broken_01");

    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 1024, 512 };
    const int scale = 3;
    const bool bFixedOnScreen = false;


    // add a prefab, bind components to it
    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);

    pPrefab->AddComponent<Transform>(p, sz, scale);
    Sprite& spr = pPrefab->AddComponent<Sprite>(texId, bFixedOnScreen);

    spr.SetSrcRectSize(sz);
}

//---------------------------------------------------------
// create entities which will be used as prefabs for graveyard
//---------------------------------------------------------
void CreatePrefabsGraves()
{
    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 32, 32 };
    const int scale = 4;

    // collider
    const Vec2i colliderPos = { 37,70 };
    const Vec2i colliderSz = { 53,37 };

    // sprite
    const TexID gravesTexId   = g_AssetMgr.GetTexId("graves");
    const Vec2i texSz         = g_AssetMgr.GetTexSize(gravesTexId);
    const bool bFixedOnScreen = false;

    // generate prefabs according to its texture
    const int numRows = texSz.h / sz.w;
    const int numCols = texSz.w / sz.h;

    //
    // create prefabs and add components
    //
    for (int row = 0; row < numRows; ++row)
    {
        for (int col = 0; col < numCols; ++col)
        {
            const Vec2i srcRectPos = { col * sz.w, row * sz.h};
            const Vec2i srcRectSz = sz;

            char prefabName[MAX_LEN_ENTT_NAME];
            snprintf(prefabName, sizeof(prefabName), "grave%dx%d_", row, col);

            Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);

                          pPrefab->AddComponent<Transform>(p, sz, scale);
                          pPrefab->AddComponent<Collider>(colliderPos, colliderSz);
            Sprite& spr = pPrefab->AddComponent<Sprite>(gravesTexId, bFixedOnScreen);

            spr.SetSrcRectPos(srcRectPos);
            spr.SetSrcRectSize(srcRectSz);
        }
    }
}

//---------------------------------------------------------
// create entity which will serve as a prefab for switching game levels
//---------------------------------------------------------
void CreatePrefabTeleport(const EventID onEnter, const char* levelName)
{
    assert(levelName && levelName[0] != '\0');

    char prefabName[32];
    snprintf(prefabName, sizeof(prefabName), "teleport_to_%s", levelName);

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    // transform (position, size, scale)
    const Vec2 p = { 200,200 };
    const Vec2i sz = { 100, 100 };
    const int scale = 1;
    const bool bFixedOnScreen = true;

    // collider
    const Vec2i colliderPos = { 0, 0 };
    const Vec2i colliderSz = { 100, 100 };

    // trigger
    TriggerInitParams trigParams;
    memset(&trigParams, 0, sizeof(trigParams));
    trigParams.onEnter               = onEnter;
    trigParams.type                  = TRIGGER_ONCE;
    trigParams.bTriggersOnlyByPlayer = true;


    // create and setup a new prefab
    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);

                  pPrefab->AddComponent<Transform>(p, sz, scale);
                  pPrefab->AddComponent<Trigger>(trigParams);
    Collider& c = pPrefab->AddComponent<Collider>(colliderPos, colliderSz);

    // we can move through this collider
    c.SetSolid(false);
}

//---------------------------------------------------------
// create a prefab for instantiating empty triggers
//---------------------------------------------------------
void CreatePrefabEmptyTrigger()
{
    const char* prefabName = "empty_trigger_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 128, 128 };
    const int scale = 1;

    // collider
    const Vec2i colliderPos = { 0, 0 };
    const Vec2i colliderSz = { sz.w, sz.h };

    // trigger
    TriggerInitParams trigParams;
    memset(&trigParams, 0, sizeof(trigParams));
    trigParams.onEnter               = INVALID_EVENT_ID;               
    trigParams.type                  = TRIGGER_ONCE;
    trigParams.bTriggersOnlyByPlayer = true;

    // create and setup a new trigger
    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);

                  pPrefab->AddComponent<Transform>(p, sz, scale);
                  pPrefab->AddComponent<Trigger>(trigParams);
    Collider& c = pPrefab->AddComponent<Collider>(colliderPos, colliderSz);

    // we can move through this collider
    c.SetSolid(false);
}

//---------------------------------------------------------
// create entity which will serve as a prefab for graves pits
//---------------------------------------------------------
void CreatePrefabGravePit()
{
    const char* prefabName = "grave_pit_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 64, 48 };
    const int scale = 4;

    // sprite
    const TexID texId = g_AssetMgr.GetTexId("graves_pit");
    const bool bFixedOnScreen = false;

    // collider
    const Vec2i colliderPos = { 20, 26 };
    const Vec2i colliderSz = { 85,140 };

    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);

    Transform&  t = pPrefab->AddComponent<Transform>(p, sz, scale);
    Sprite&   spr = pPrefab->AddComponent<Sprite>(texId, bFixedOnScreen);
    Collider& col = pPrefab->AddComponent<Collider>(colliderPos, colliderSz);

    spr.SetSrcRectPos({ 0, 0 });
    spr.SetSrcRectSize(sz);
}

//---------------------------------------------------------
// create an entity which will serve as a prefab for campfire instances
//---------------------------------------------------------
void CreatePrefabCampfire()
{
    const char* prefabName = "campfire_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    // transform (position, size, scale)
    Vec2 p = { 0,0 };
    Vec2i sz = { 64, 64 };
    int scale = 4;

    // sprite + animation
    const TexID texId = g_AssetMgr.GetTexId("campfire");
    assert(texId != INVALID_TEX_ID);
    const int fullAnimTimeMs = 1200;
    const int numFrames = 12;
    const int animSpeedMs = fullAnimTimeMs / numFrames;
    const char* animName = "campfire";
    const bool bLooped = true;
    const bool bFixedOnScreen = false;

    if (!g_AssetMgr.HasAnimByName(animName))
        g_AssetMgr.AddAnim(animName, texId, animSpeedMs, numFrames, bLooped);

    const Animation& anim = g_AssetMgr.GetAnim(animName);

    // collider 
    const Vec2i colliderCenter = { 122, 183 };
    const int colliderRadius = 78;

    // hierarchy
    EntityID parentId = INVALID_ENTT_ID;    // no parent
    Vec2 relPos = { 0,0 };


    //
    // create a campfire's prefab
    //
    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_ENTITY);
                           pPrefab->AddComponent<Transform>(p, sz, scale);
                           pPrefab->AddComponent<Collider>(colliderCenter, colliderRadius);
    Hierarchy& hierarchy = pPrefab->AddComponent<Hierarchy>();
    Sprite&          spr = pPrefab->AddComponent<Sprite>(anim, bFixedOnScreen);
    spr.SetSrcRectSize(sz);
    spr.SetColor(255, 246, 223, 200);

    //
    // create a prefab for light decal on the ground
    //
    sz = { 128, 128 };
    scale = 5;
    const TexID pointLTexId = g_AssetMgr.GetTexId("point_light");

    // hierarchy
    parentId = pPrefab->GetId();
    relPos = { -198, -121 };

    Entity* pGndLight = g_EnttMgr.AddPrefab("campfireGndL_", LAYER_GND_DECAL);
                                pGndLight->AddComponent<Transform>(p, sz, scale);
    Hierarchy& lightHierarchy = pGndLight->AddComponent<Hierarchy>();
    Sprite&          lightSpr = pGndLight->AddComponent<Sprite>(pointLTexId, bFixedOnScreen);

    lightSpr.SetColor(255, 204, 63, 128);
    lightSpr.SetSrcRectSize(sz);

    hierarchy.AddChild(pGndLight);
    lightHierarchy.SetRelativePos(relPos.x, relPos.y);
}

//---------------------------------------------------------
// create prefabs for making grass instances
//---------------------------------------------------------
void CreatePrefabsGrass()
{
    const char* prefabsNames[] = {
        "grass0_",
        "grass1_",
        "grass2_",
        "grass3_",
        "grass4_",
        "grass5_",
        "grass6_",
    };

    const size_t numPrefabs = ARRAY_SIZE(prefabsNames);

   // check if prefabs already created
   // (it may happen when we load/create another level in runtime)
    int numCreatedPrefabs = 0;

    for (int i = 0; i < numPrefabs; ++i)
        numCreatedPrefabs += g_EnttMgr.HasPrefab(prefabsNames[i]);

    if (numCreatedPrefabs == numPrefabs)
        return;


    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sizes[] = {
        {32,32},        // graveyard grass
        {48,48},
        {48,48},
        {48,48},
        {48,48},
        {48,48},
        {48,48},
    };
    const int scales[] = { 3, 2, 2, 2, 2, 2, 2 };
    const bool bFixedOnScreen = true;

    // sprite params
    const Vec2i srcRectPos[] = {      // offsets of the top-left corner of each grass on the atlas texture
        {0,0},          // graveyard grass
        {0,0},
        {48,0},
        {0,48},
        {48,48},
        {0,96},
        {48,96}
    };

    const Vec2i srcRectSize[] =
    {
        {32,32},        // graveyard grass
        {48,48},
        {48,48},
        {48,48},
        {48,48},
        {48,48},
        {48,48},
    };

    const TexID texId0 = g_AssetMgr.GetTexId("grass1");
    const TexID texId1 = g_AssetMgr.GetTexId("grass0");
    const TexID texIds[] = {
        texId0,                     // graveyard grass
        texId1, texId1, texId1,
        texId1, texId1 ,texId1 
    };

    // check stuff to prevent fuckup
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(sizes));
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(scales));
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(srcRectPos));
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(srcRectSize));
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(texIds));

    //
    // create prefabs
    //
    for (int i = 0; i < numPrefabs; ++i)
    {
        Entity* pPrefab = g_EnttMgr.AddPrefab(prefabsNames[i], LAYER_ENTITY);

        // add components
        pPrefab->AddComponent<Transform>(p, sizes[i], scales[i]);
        Sprite& spr = pPrefab->AddComponent<Sprite>(texIds[i], !bFixedOnScreen);

        spr.SetSrcRectPos(srcRectPos[i]);
        spr.SetSrcRectSize(srcRectSize[i]);
    }
}


//---------------------------------------------------------
// create entity which will serve as a prefab for pentagrams on the ground (gnd_decals)
//---------------------------------------------------------
void CreatePentagram()
{
    const char* prefabName = "pentagram_";

    if (g_EnttMgr.HasPrefab(prefabName))
        return;

    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 96,48 };
    const int scale = 4;

    // sprite
    const TexID texId = g_AssetMgr.GetTexId("pentagram_0");
    assert(texId != INVALID_TEX_ID);
    const bool bFixedOnScreen = false;

    // create prefab and add components
    Entity* pPrefab = g_EnttMgr.AddPrefab(prefabName, LAYER_GND_DECAL);
                  pPrefab->AddComponent<Transform>(p, sz, scale);
    Sprite& spr = pPrefab->AddComponent<Sprite>(texId, bFixedOnScreen);

    spr.SetSrcRectSize(sz);
}

//---------------------------------------------------------
// create entity which will serve as a prefab for decals over the ground
//---------------------------------------------------------
void CreateGroundDecals()
{
    const char* prefabNames[] =
    {
        "gndNoise0_",
        "gndNoise1_",
    };

    // transform (position, size, scale)
    const Vec2 p = { 0,0 };
    const Vec2i sz = { 128,128 };
    const int scale = 4;

    // sprite
    const char* textureNames[] =
    {
        "gnd_noise_decal0",
        "gnd_noise_decal1",
    };
    const bool bFixedOnScreen = false;

    STATIC_ASSERT(ARRAY_SIZE(prefabNames) == ARRAY_SIZE(textureNames));

    // create prefabs and add components
    for (int i = 0; i < ARRAY_SIZE(prefabNames); ++i)
    {
        if (g_EnttMgr.HasPrefab(prefabNames[i]))
            continue;

        const TexID texId = g_AssetMgr.GetTexId(textureNames[i]);
        assert(texId != INVALID_TEX_ID);

        Entity* pPrefab = g_EnttMgr.AddPrefab(prefabNames[i], LAYER_GND_DECAL);
                      
                      pPrefab->AddComponent<Transform>(p, sz, scale);
        Sprite& spr = pPrefab->AddComponent<Sprite>(texId, bFixedOnScreen);
        spr.SetSrcRectSize(sz);
    }
}

//---------------------------------------------------------
// create prefabs for bushes on the ground
//---------------------------------------------------------
void CreatePrefabsBushes()
{
    const char* prefabsNames[] = {
        "bush0_",
        "bush1_",
    };

    const size_t numPrefabs = ARRAY_SIZE(prefabsNames);

    // check if prefabs already created
    // (it may happen when we load/create another level in runtime)
    int numCreatedPrefabs = 0;

    for (int i = 0; i < numPrefabs; ++i)
        numCreatedPrefabs += g_EnttMgr.HasPrefab(prefabsNames[i]);

    if (numCreatedPrefabs == numPrefabs)
        return;

    //
    // prepare data for components
    //

    // transform (position, size, scale)
    const Vec2 pos = { 0,0 };
    const Vec2i sizes[] = {
        {96,96},
        {192,96},
    };
    const int scale = 1;

    // sprites 
    const TexID texId0 = g_AssetMgr.GetTexId("bushes0");
    const TexID texIds[] = { texId0, texId0 };
    const bool bFixedOnScreen = false;

    // top left position of a subrectangle on the texture
    const Vec2i srcRectPos[] = {
        {0,0},
        {48,0},
    };

    const Vec2i srcRectSize[] = {
        {48,48},
        {96,48},
    };

    // check stuff to prevent fuckup
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(sizes));
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(srcRectPos));
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(srcRectSize));
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(texIds));

    //
    // create prefabs and add components
    //
    for (int i = 0; i < numPrefabs; ++i)
    {
        Entity* p = g_EnttMgr.AddPrefab(prefabsNames[i], LAYER_ENTITY);

        p->AddComponent<Transform>(pos, sizes[i], scale);
        Sprite& spr = p->AddComponent<Sprite>(texIds[i], bFixedOnScreen);

        spr.SetSrcRectPos(srcRectPos[i]);
        spr.SetSrcRectSize(srcRectSize[i]);
    }
}

//---------------------------------------------------------
// create prefabs for branches on the ground
//---------------------------------------------------------
void CreatePrefabsBranches()
{
    const char* prefabsNames[] = {
        "branch0_",
        "branch1_",
        "branch2_",
        "branch3_",
        "branch4_",
        "branch5_",
    };

    const size_t numPrefabs = ARRAY_SIZE(prefabsNames);

    // check if prefabs already created
    // (it may happen when we load/create another level in runtime)
    int numCreatedPrefabs = 0;

    for (int i = 0; i < numPrefabs; ++i)
        numCreatedPrefabs += g_EnttMgr.HasPrefab(prefabsNames[i]);

    if (numCreatedPrefabs == numPrefabs)
        return;

    //
    // prepare data for components
    //

    // transform (position, size, scale)
    const Vec2 pos = { 0,0 };
    const Vec2i sizes[] = {
        {96,48},
        {48,48},
        {48,48},
        {96,48},
        {48,48},
        {48,48},
    };
    const int scale = 3;

    // sprites 
    const TexID texIds = g_AssetMgr.GetTexId("branches0");
    const bool bFixedOnScreen = false;

    // top left position of a subrectangle on the texture
    const Vec2i srcRectPos[] = {
        {0,0},
        {0,48},
        {48,48},
        {96,0},
        {96,48},
        {144,48}
    };

    // check stuff to prevent fuckup
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(sizes));
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(srcRectPos));

    //
    // create prefabs and add components
    //
    for (int i = 0; i < numPrefabs; ++i)
    {
        Entity* p = g_EnttMgr.AddPrefab(prefabsNames[i], LAYER_GND_DECAL);

                      p->AddComponent<Transform>(pos, sizes[i], scale);
        Sprite& spr = p->AddComponent<Sprite>(texIds, bFixedOnScreen);

        spr.SetSrcRectPos(srcRectPos[i]);
        spr.SetSrcRectSize(sizes[i]);
    }
}

//---------------------------------------------------------
// prefabs for test purposes
//---------------------------------------------------------
void CreateTestPrefabs()
{
    Entity* pPrefab = g_EnttMgr.AddPrefab("greenTriangle_", LAYER_ENTITY);

                  pPrefab->AddComponent<Transform>(Vec2(0, 0), Vec2i(64, 64), 1);
    Sprite& spr = pPrefab->AddComponent<Sprite>(g_AssetMgr.GetTexId("green_triangle"), false);

    spr.SetSrcRectSize(Vec2i(64, 64));
}

//---------------------------------------------------------
// initialization data for tree's colliders
//---------------------------------------------------------
struct TreeColliderParams
{
    // left circle
    Vec2i LCircleSizeW;
    Vec2  LCircleRelPos;
    Vec2i LCircleCenter;    
    int   LCircleRadius;    

    // right circle
    Vec2i RCircleSizeW;
    Vec2  RCircleRelPos;
    Vec2i RCircleCenter;    // collider's center
    int   RCircleRadius;    // collider's radius
};

//---------------------------------------------------------
// prefabs for trees
//---------------------------------------------------------
void CreatePrefabsTrees()
{
    const char* prefabNames[] = {
        "treeDead1_",
        "treeDead2_",
        "tree0_",
        "tree1_",
        "tree2_",
        "tree3_",
        "treePine0_",
        "treeOak1_",
    };

    const size_t numPrefabs = ARRAY_SIZE(prefabNames);

    // check if prefabs already created
    // (it may happen when we load/create another level in runtime)
    int numCreatedPrefabs = 0;

    for (int i = 0; i < numPrefabs; ++i)
        numCreatedPrefabs += g_EnttMgr.HasPrefab(prefabNames[i]);

    if (numCreatedPrefabs == numPrefabs)
        return;

    //
    // prepare data for components:
    //
    const Vec2 pos = { 0,0 };
    const Vec2i sizes[] = 
    {
        { 112, 112 },   // tree dead 1
        { 80, 125 },   // tree dead 2

        { 96, 96 },
        { 96, 96 },
        { 96, 96 },

        { 172, 128 },
        { 108, 193 },   // tree pine
        { 225, 250 },   // tree oak1
    };
    const int scales[] = { 4,4,4,4,4,4, 3,2 };

    // sprites
    const bool bFixedOnScreen = false;

    const TexID texId0 = g_AssetMgr.GetTexId("tree_dead01");
    const TexID texId1 = g_AssetMgr.GetTexId("tree_dead02");
    const TexID texId2 = g_AssetMgr.GetTexId("trees_meadows");
    const TexID texId3 = g_AssetMgr.GetTexId("tree04");
    const TexID texId4 = g_AssetMgr.GetTexId("tree_pine");
    const TexID texId5 = g_AssetMgr.GetTexId("tree_oak1");

    const TexID texIds[] = {
        texId0,
        texId1,
        texId2,
        texId2,
        texId2,
        texId3,
        texId4,
        texId5,
    };

    // top left position of subrectangle on the texture
    const Vec2i srcRectPos[] = {
        {0, 0},         // tree dead 1
        {0, 0},         // tree dead 2

        {96 * 0, 0},
        {96 * 1, 0},
        {96 * 2, 0},

        {0, 0},
        {0, 0},         // tree pine
        {0, 0},         // tree oak1
    };

    const Vec2i srcRectSizes[] =
    {
        { 112, 112 },   // tree dead 1
        { 80, 125 },   // tree dead 2

        { 96, 96 },
        { 96, 96 },
        { 96, 96 },

        { 172, 128 },
        { 108, 193 },   // tree pine
        { 225, 250 },   // tree oak1
    };


    // colliders
    const Vec2i colliderRectPos[] = {
        { 200, 352 },       // tree dead 1
        { 180, 427 },       // tree dead 2
        { 0,0 },
        { 0,0 },
        { 0,0 },

        { 0,0 },
        { 104,520 },          // tree pine
        { 104,520 },          // tree oak1
    };

    const Vec2i colliderRectSz[] = {
        { 85,85 },          // tree dead 1
        { 158,62 },         // tree dead 2

        {98,98},
        {98,98},
        {98,98},

        {98,98},
        {128,56},            // tree pine
        {128,56},            // tree oak1
    };

    const TreeColliderParams collidersParams[] = {
        // tree_dead01
        {
            .LCircleSizeW  = {88,88},
            .LCircleRelPos = {143,350},
            .LCircleCenter = {44,44},
            .LCircleRadius = {45},

            .RCircleSizeW  = {88,88},
            .RCircleRelPos = {253,350},
            .RCircleCenter = {44,44},
            .RCircleRadius = {45},
        },

        // tree_dead02
        {
            .LCircleSizeW   = {70,70},
            .LCircleRelPos  = {146,423},
            .LCircleCenter  = {35,35},
            .LCircleRadius  = {35},

            .RCircleSizeW   = {70,70},
            .RCircleRelPos  = {300,423},
            .RCircleCenter  = {35,35},
            .RCircleRadius  = {35},
        },

        // tree type 0
        {
            .LCircleSizeW = {70,70},
            .LCircleRelPos = {146,423},
            .LCircleCenter = {35,35},
            .LCircleRadius = {35},

            .RCircleSizeW = {70,70},
            .RCircleRelPos = {300,423},
            .RCircleCenter = {35,35},
            .RCircleRadius = {35},
        },

        // tree type 1
        {
            .LCircleSizeW = {70,70},
            .LCircleRelPos = {146,423},
            .LCircleCenter = {35,35},
            .LCircleRadius = {35},

            .RCircleSizeW = {70,70},
            .RCircleRelPos = {300,423},
            .RCircleCenter = {35,35},
            .RCircleRadius = {35},
        },

        // tree type 2
        {
            .LCircleSizeW = {70,70},
            .LCircleRelPos = {146,423},
            .LCircleCenter = {35,35},
            .LCircleRadius = {35},

            .RCircleSizeW = {70,70},
            .RCircleRelPos = {300,423},
            .RCircleCenter = {35,35},
            .RCircleRadius = {35},
        },

        // tree type 3
        {
            .LCircleSizeW = {70,70},
            .LCircleRelPos = {146,423},
            .LCircleCenter = {35,35},
            .LCircleRadius = {35},

            .RCircleSizeW = {70,70},
            .RCircleRelPos = {300,423},
            .RCircleCenter = {35,35},
            .RCircleRadius = {35},
        },

        // tree pine
        {
            .LCircleSizeW = {58,58},
            .LCircleRelPos = {73,519},
            .LCircleCenter = {29,29},
            .LCircleRadius = {29},

            .RCircleSizeW = {58,58},
            .RCircleRelPos = {200,519},
            .RCircleCenter = {29,29},
            .RCircleRadius = {29},
        },

        // tree oak1
       {
           .LCircleSizeW = {58,58},
           .LCircleRelPos = {73,519},
           .LCircleCenter = {29,29},
           .LCircleRadius = {29},

           .RCircleSizeW = {58,58},
           .RCircleRelPos = {200,519},
           .RCircleCenter = {29,29},
           .RCircleRadius = {29},
       },
    };

    // check stuff to prevent fuckup
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(texIds));
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(srcRectPos));
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(srcRectSizes));
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(sizes));
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(scales));
    STATIC_ASSERT(numPrefabs == ARRAY_SIZE(collidersParams));

    //
    // create prefabs and add components
    //
    for (int i = 0; i < numPrefabs; ++i)
    {
        Entity* p = g_EnttMgr.AddPrefab(prefabNames[i], LAYER_ENTITY);

                        p->AddComponent<Transform>(pos, sizes[i], scales[i]);
        Hierarchy&  h = p->AddComponent<Hierarchy>();
        Collider& col = p->AddComponent<Collider>(colliderRectPos[i], colliderRectSz[i]);
        Sprite&   spr = p->AddComponent<Sprite>(texIds[i], bFixedOnScreen);
       
        spr.SetSrcRectPos(srcRectPos[i]);
        spr.SetSrcRectSize(srcRectSizes[i]);


        // create a special prefabs for additional tree's colliders (2 circles)
        char colliderNameC0[MAX_LEN_ENTT_NAME];
        char colliderNameC1[MAX_LEN_ENTT_NAME];

        snprintf(colliderNameC0, MAX_LEN_ENTT_NAME, "treeColliderC0%d_", i);
        snprintf(colliderNameC1, MAX_LEN_ENTT_NAME, "treeColliderC1%d_", i);

        Entity* pCol1 = g_EnttMgr.AddPrefab(colliderNameC0, LAYER_ENTITY);
        Entity* pCol2 = g_EnttMgr.AddPrefab(colliderNameC1, LAYER_ENTITY);

        const int colliderScale = 1;
        const TreeColliderParams& params = collidersParams[i];

        pCol1->AddComponent<Transform>(pos, params.LCircleSizeW, colliderScale);
        pCol2->AddComponent<Transform>(pos, params.RCircleSizeW, colliderScale);

        pCol1->AddComponent<Collider>(params.LCircleCenter, params.LCircleRadius);
        pCol2->AddComponent<Collider>(params.RCircleCenter, params.RCircleRadius);

        Hierarchy& hc1 = pCol1->AddComponent<Hierarchy>();
        Hierarchy& hc2 = pCol2->AddComponent<Hierarchy>();

        // attach these colliders to the tree
        h.AddChild(pCol1);
        h.AddChild(pCol2);

        hc1.SetRelativePos(params.LCircleRelPos);
        hc2.SetRelativePos(params.RCircleRelPos);
    }

    //-----------------------------------------------------
    // attach shadows of the trees in a separate way
    //-----------------------------------------------------
    Entity* pTree = nullptr;
    Entity* pShadow = nullptr;
    TexID texIdShadow = INVALID_TEX_ID;
    const Vec2 shadPos = { 0,0 };
    Vec2i shadSz = { 1, 1 };
    int shadScale = 1;

    Hierarchy* h = nullptr;
    Sprite* spr = nullptr;


    // shadow for dead tree 02
    pTree = g_EnttMgr.GetPrefab("treeDead2_");
    shadSz = { 108,30 };
    shadScale = pTree->GetComponent<Transform>()->GetScale();
    texIdShadow = g_AssetMgr.GetTexId("tree_dead02_shadow");

    pShadow = g_EnttMgr.AddPrefab("treeDead02Shad_", LAYER_ENTITY);

           pShadow->AddComponent<Transform>(shadPos, shadSz, shadScale);
    h   = &pShadow->AddComponent<Hierarchy>();
    spr = &pShadow->AddComponent<Sprite>(texIdShadow, bFixedOnScreen);
    
    spr->SetSrcRectSize(shadSz);
    spr->SetColor(255, 255, 255, 255);
    pTree->GetComponent<Hierarchy>()->AddChild(pShadow);
    h->SetRelativePos({ 74, 376 });

    
    // shadow for tree pine
    pTree = g_EnttMgr.GetPrefab("treePine0_");
    shadSz = { 206,55 };
    shadScale = pTree->GetComponent<Transform>()->GetScale();
    texIdShadow = g_AssetMgr.GetTexId("tree_pine_shadow");

    pShadow = g_EnttMgr.AddPrefab("treePineShad_", LAYER_ENTITY);

           pShadow->AddComponent<Transform>(shadPos, shadSz, shadScale);
    h   = &pShadow->AddComponent<Hierarchy>();
    spr = &pShadow->AddComponent<Sprite>(texIdShadow, bFixedOnScreen);

    spr->SetSrcRectSize(shadSz);
    spr->SetColor(0, 0, 0, 255);
    pTree->GetComponent<Hierarchy>()->AddChild(pShadow);
    h->SetRelativePos({ 81, 450 });

    // shadow for tree oak1
    pTree = g_EnttMgr.GetPrefab("treeOak1_");
    shadSz = { 275,150 };
    shadScale = pTree->GetComponent<Transform>()->GetScale();
    texIdShadow = g_AssetMgr.GetTexId("tree_oak1_shadow");

    pShadow = g_EnttMgr.AddPrefab("treeOak1Shad_", LAYER_ENTITY);

    pShadow->AddComponent<Transform>(shadPos, shadSz, shadScale);
    h = &pShadow->AddComponent<Hierarchy>();
    spr = &pShadow->AddComponent<Sprite>(texIdShadow, bFixedOnScreen);

    spr->SetSrcRectSize(shadSz);
    spr->SetColor(255, 255, 255, 188);
    pTree->GetComponent<Hierarchy>()->AddChild(pShadow);
    h->SetRelativePos({ 132, 302 });
}