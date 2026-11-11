/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:  entity_save_load.cpp
    Desc:      functional for saving and loading entities (from/into a file)
    Created:   23.05.2026 by DimaSkup
\**********************************************************************************/
#include "entity_save_load.h"

#include "asset_mgr.h"
#include "entity_mgr.h"
#include "log.h"
#include "input.h"
#include "file.h"
#include "ui_facade_engine.h"

#include "components/tile.h"
#include "components/transform.h"
#include "components/sprite.h"
#include "components/keyboard.h"
#include "components/enemy.h"
#include "components/health.h"
#include "components/damage.h"
#include "components/collider.h"
#include "components/movement.h"
#include "components/player.h"
#include "components/trigger.h"
#include "components/hierarchy.h"
#include "components/life_timer.h"

#include <ctype.h>

// typedef for pointers to component's saving/loading functions
typedef void(*ComponentSaveFunc)(FILE* fp, const Entity* pEntt);
typedef void(*ComponentLoadFunc)(FILE* fp, Entity* pEntt);

// maps of [component_type_info => function_ptr]
static std::map<const std::type_info*, ComponentSaveFunc> s_SaversFunc;
static std::map<const std::type_info*, ComponentLoadFunc> s_LoadersFunc;



//=============================================================================
//=============================================================================
// PRIVATE SAVERS
//=============================================================================
//=============================================================================

//---------------------------------------------------------
// save Player component data of input entity
//---------------------------------------------------------
void SavePlayer(FILE* fp, const Entity* pEntt)
{
    assert(fp && pEntt);
    assert(pEntt->HasComponent<Player>());

    const Player*     player = pEntt->GetComponent<Player>();
    const WeaponID currWpnId = player->GetCurrWeaponId();

    PrintOffset(fp, 3);
    fprintf(fp, "current_wpn: %s\n", "wpn_sword");

    PrintOffset(fp, 3);
    fprintf(fp, "move_speed: %.2f\n", player->GetMoveBasicSpeed());
}

//---------------------------------------------------------
// save Transform component data of input entity
//---------------------------------------------------------
void SaveTransform(FILE* fp, const Entity* pEntt)
{
    assert(fp && pEntt);
    assert(pEntt->HasComponent<Transform>());

    const Transform* t = pEntt->GetComponent<Transform>();
    const Vec2       p = t->GetPos();
    const Vec2i     sz = t->GetLocalSize();

    PrintOffset(fp, 3);
    fprintf(fp, "pos: %d %d\n", (int)p.x, (int)p.y);

    PrintOffset(fp, 3);
    fprintf(fp, "size: %d %d\n", sz.w, sz.h);

    PrintOffset(fp, 3);
    fprintf(fp, "scale: %d\n", t->GetScale());
}

//---------------------------------------------------------
// save Movement component data of input entity
//---------------------------------------------------------
void SaveMovement(FILE* fp, const Entity* pEntt)
{
    assert(fp && pEntt);
    assert(pEntt->HasComponent<Movement>());

    const Vec2 v = pEntt->GetComponent<Movement>()->GetVelocity();

    PrintOffset(fp, 3);
    fprintf(fp, "velocity: %.2f %.2f\n", v.x, v.y);
}

//---------------------------------------------------------
// save Sprite component data of input entity
//---------------------------------------------------------
void SaveSprite(FILE* fp, const Entity* pEntt)
{
    assert(fp && pEntt);
    assert(pEntt->HasComponent<Sprite>());

    const Sprite*     pSpr = pEntt->GetComponent<Sprite>();
    const Recti&         r = pSpr->GetSrcRect();
    const Color      color = pSpr->GetColor();
    const eLayerType layer = pEntt->GetLayer();
    const bool   bAnimated = pSpr->IsAnimated();


    PrintOffset(fp, 3);
    fprintf(fp, "is_animated: %d\n", (int)bAnimated);

    PrintOffset(fp, 3);
    fprintf(fp, "is_fixed_on_screen: %d\n", (int)pSpr->IsFixedOnScreen());

    PrintOffset(fp, 3);
    fprintf(fp, "is_show: %d\n", (int)pSpr->IsShow());

    PrintOffset(fp, 3);
    fprintf(fp, "src_rect_pos: %d %d\n", r.x0, r.y0);

    PrintOffset(fp, 3);
    fprintf(fp, "src_rect_size: %d %d\n", r.x1 - r.x0, r.y1 - r.y0);
    assert((r.x1 - r.x0) != 0 && (r.y1 - r.y0) != 0 && "sprite's src tex width or height can't == 0");

    PrintOffset(fp, 3);
    fprintf(fp, "color: %d %d %d %d\n", color.r, color.g, color.b, color.a);

    if (layer == LAYER_GND_DECAL || layer == LAYER_UI)
    {
        PrintOffset(fp, 3);
        fprintf(fp, "z_index: %d\n", pSpr->GetZIndex());
    }

    // for animated sprites we need to store some additional data
    if (bAnimated)
    {
        const bool bManyAnims = pSpr->HasMultipleAnimTypes();

        // multiple animation types (idle, move, attack, etc.)
        if (bManyAnims)
        {
            for (int i = 0; i < NUM_ANIM_TYPES; ++i)
            {
                const char*  typeName = pSpr->GetAnimTypeName(eAnimType(i));
                const Animation& anim = pSpr->GetAnimByType(eAnimType(i));
                const char*  animName = g_AssetMgr.GetAnimName(anim.id);

                PrintOffset(fp, 3);
                fprintf(fp, "anim_%s: %s\n", typeName, animName);
            }
        }

        // has animations but only one animation type (idle)
        else
        {
            const char*  typeName = pSpr->GetAnimTypeName(ANIM_TYPE_IDLE);
            const Animation& anim = pSpr->GetAnimByType(ANIM_TYPE_IDLE);
            const char*  animName = g_AssetMgr.GetAnimName(anim.id);

            PrintOffset(fp, 3);
            fprintf(fp, "anim_%s: %s\n", typeName, animName);
        }
    }

    // non-animated sprite with only one texture
    else
    {
        PrintOffset(fp, 3);
        const TexID texId = pSpr->GetTexId(ANIM_TYPE_IDLE);
        fprintf(fp, "texture: %s\n", g_AssetMgr.GetTexName(texId));
    }
}

//---------------------------------------------------------
// save Keyboard component data of input entity
//---------------------------------------------------------
void SaveKeyboard(FILE* fp, const Entity* pEntt)
{
    assert(fp && pEntt);
    assert(pEntt->HasComponent<Keyboard>());

    const Keyboard* k = pEntt->GetComponent<Keyboard>();

    for (int i = 0; i < NUM_KEY_TYPES; ++i)
    {
        const eKeyCode keycode = k->GetKeyCodeByType(eKeyTypes(i));

        if (keycode == KEY_MAX)
        {
            LogErr(LOG, "no keycode is found for a key type: %s", s_KeyTypeNames[i]);
            continue;
        }

        PrintOffset(fp, 3);
        fprintf(fp, "%s: %s\n", s_KeyTypeNames[i], GetKeyName(keycode));
    }
}

//---------------------------------------------------------
// save the Enemy component data of input entity
//---------------------------------------------------------
void SaveEnemy(FILE* fp, const Entity* pEntt)
{
    assert(fp);
    assert(pEntt && pEntt->HasComponent<Enemy>());

    const Enemy* pEnemy = pEntt->GetComponent<Enemy>();

    PrintOffset(fp, 3);
    fprintf(fp, "type: %s\n", pEnemy->GetTypeAsStr());
}

//---------------------------------------------------------
// save Heath component data of input entity
//---------------------------------------------------------
void SaveHealth(FILE* fp, const Entity* pEntt)
{
    assert(fp && pEntt);
    assert(pEntt->HasComponent<Health>());

    const Health* h = pEntt->GetComponent<Health>();

    PrintOffset(fp, 3);
    fprintf(fp, "hp: %d\n", h->GetHP());

    PrintOffset(fp, 3);
    fprintf(fp, "max_hp: %d\n", h->GetMaxHP());

    PrintOffset(fp, 3);
    fprintf(fp, "heal_per_sec: %d\n", h->GetHealPerSec());
}

//---------------------------------------------------------
// save Damage component data of input entity
//---------------------------------------------------------
void SaveDamage(FILE* fp, const Entity* pEntt)
{
    assert(fp && pEntt);
    assert(pEntt->HasComponent<Damage>());

    const Damage* d = pEntt->GetComponent<Damage>();

    PrintOffset(fp, 3);
    fprintf(fp, "cooldown_max_sec: %f\n", d->GetCooldownMaxSec());

    PrintOffset(fp, 3);
    fprintf(fp, "damage: %f\n", d->GetDamage());
}

//---------------------------------------------------------
// save Collider component data of input entity
//---------------------------------------------------------
void SaveCollider(FILE* fp, const Entity* pEntt)
{
    assert(fp && pEntt);
    assert(pEntt->HasComponent<Collider>());

    const Collider* collider = pEntt->GetComponent<Collider>();
    const eColliderType colType = collider->GetType();

    PrintOffset(fp, 3);
    fprintf(fp, "type: %s\n", collider->GetColliderTypeName(colType));

    PrintOffset(fp, 3);
    fprintf(fp, "is_solid: %d\n", collider->IsSolid());

    if (colType == COLLIDER_BOX)
    {
        const Vec2i p  = collider->GetBoxPos();
        const Vec2i sz = collider->GetBoxSize();

        PrintOffset(fp, 3);
        fprintf(fp, "pos: %d %d\n", p.x, p.y);

        PrintOffset(fp, 3);
        fprintf(fp, "size: %d %d\n", sz.w, sz.h);
    }
    else if (colType == COLLIDER_CIRCLE)
    {
        const Vec2i center = collider->GetCenter();   // center of circle relatively to the top-left corner of entity

        PrintOffset(fp, 3);
        fprintf(fp, "center: %d %d\n", center.x, center.y);

        PrintOffset(fp, 3);
        fprintf(fp, "radius: %d\n", collider->GetRadius());
    }
    else
    {
        LogErr(LOG, "unknown type (%d) of collider for entity (%s)", (int)colType, pEntt->GetName());
    }
}

//---------------------------------------------------------
// save Trigger component data of input entity
//---------------------------------------------------------
void SaveTrigger(FILE* fp, const Entity* pEntt)
{
    assert(fp && pEntt);
    assert(pEntt->HasComponent<Trigger>());

    const Trigger*        t = pEntt->GetComponent<Trigger>();
    const eTriggerType type = t->GetType();


    PrintOffset(fp, 3);
    fprintf(fp, "is_trig_only_by_player: %d\n", t->IsTriggersOnlyByPlayer());

    PrintOffset(fp, 3);
    fprintf(fp, "type: %s\n", Trigger::GetNameByType(type));

    PrintOffset(fp, 3);
    fprintf(fp, "on_enter_event: %s\n", g_pFacadeEngine->GetEventNameById(t->GetOnEnter()));

    if (type == TRIGGER_MULTIPLE)
    {
        const char* onCollide = g_pFacadeEngine->GetEventNameById(t->GetOnCollide());
        const char* onLeave   = g_pFacadeEngine->GetEventNameById(t->GetOnLeave());

        PrintOffset(fp, 3);
        fprintf(fp, "on_collide_event: %s\n", onCollide);

        PrintOffset(fp, 3);
        fprintf(fp, "on_leave_event: %s\n", onLeave);
    }
}

//---------------------------------------------------------
// save Hierarchy component data of input entity
//---------------------------------------------------------
void SaveHierarchy(FILE* fp, const Entity* pEntt)
{
    assert(fp && pEntt);

    const Hierarchy* h = pEntt->GetComponent<Hierarchy>();
    assert(h && "no hierarchy component");

    const EntityID parentId = h->GetParent();

    // do we want to save an entity or prefab?
    bool isEntt = g_EnttMgr.HasEntt(pEntt->GetId());

    if (isEntt)
    {
        // save info about parent and relative position only if input entity has a parent
        if (parentId != INVALID_ENTT_ID)
        {
            PrintOffset(fp, 3);
            fprintf(fp, "parent: %s\n", g_EnttMgr.GetEntt(parentId)->GetName());

            const Vec2 relPos = h->GetRelativePos();
            PrintOffset(fp, 3);
            fprintf(fp, "rel_pos: %.2f %.2f\n", relPos.x, relPos.y);
        }

        // save children (if has any)
        const EntityID* childrenIds = h->GetChildren();

        for (size_t i = 0; i < h->GetNumChildren(); ++i)
        {
            const Entity* pChild = g_EnttMgr.GetEntt(childrenIds[i]);
            assert(pChild->GetId() != INVALID_ENTT_ID);

            PrintOffset(fp, 3);
            fprintf(fp, "child: %s\n", pChild->GetName());
        }
    }

    // save the prefab
    else
    {
        // save info about parent and relative position only if input entity has a parent
        if (parentId != INVALID_ENTT_ID)
        {
            PrintOffset(fp, 3);
            fprintf(fp, "parent: %s\n", g_EnttMgr.GetPrefab(parentId)->GetName());

            const Vec2 relPos = h->GetRelativePos();
            PrintOffset(fp, 3);
            fprintf(fp, "rel_pos: %.2f %.2f\n", relPos.x, relPos.y);
        }

        // save children (if has any)
        const EntityID* childrenIds = h->GetChildren();

        for (size_t i = 0; i < h->GetNumChildren(); ++i)
        {
            const Entity* pChild = g_EnttMgr.GetPrefab(childrenIds[i]);
            assert(pChild->GetId() != INVALID_ENTT_ID);

            PrintOffset(fp, 3);
            fprintf(fp, "child: %s\n", pChild->GetName());
        }
    }
}

//---------------------------------------------------------
// save LifeTimer component's data of input entity
//---------------------------------------------------------
void SaveLifeTimer(FILE* fp, const Entity* pEntt)
{
    assert(fp && pEntt);
    
    const LifeTimer* pTimer = pEntt->GetComponent<LifeTimer>();
    assert(pTimer);

    PrintOffset(fp, 3);
    fprintf(fp, "life_time: %f\n", pTimer->GetFullLifeTime());
}


//=============================================================================
//=============================================================================
// PRIVATE LOADERS
//=============================================================================
//=============================================================================

//---------------------------------------------------------
// load Player component's data from file and add this component to input entity
//---------------------------------------------------------
void LoadPlayer(FILE* fp, Entity* pEntt)
{
    assert(fp && pEntt);
    assert(!pEntt->HasComponent<Player>());

    int count = 0;
    float moveSpeed = 0;
    WeaponID currWpnId = 0;
    char buffer[128];
    char currWpnName[32];
    currWpnName[0] = '\0';

    while (fgets(buffer, sizeof(buffer), fp))
    {
        // we've done here
        if (buffer[2] == '}')
            break;

        // +3 to skip '\t' symbols
        const char* buf = buffer + 3;

        char key[32];
        count = sscanf(buf, "%s", key);
        assert(count == 1);

        // read in values according to field type
        if (strcmp(key, "current_wpn:") == 0)
        {
            count = sscanf(buf, "current_wpn: %s", currWpnName);
            assert(count == 1);
            currWpnId = g_AssetMgr.GetWpnIdByName(currWpnName);
        }
        else if (strcmp(key, "move_speed:") == 0)
        {
            count = sscanf(buf, "move_speed: %f", &moveSpeed);
            assert(count == 1);
        }
        else
        {
            LogErr(LOG, "unknown field key (%s) when read Player component", key);
        }
    }

    pEntt->AddComponent<Player>(moveSpeed, currWpnId);
}

//---------------------------------------------------------
// a helper for checking if input string is an integer number
//---------------------------------------------------------
bool IsNumber(const char* str)
{
    assert(str);

    char* end = NULL;

    if (*str == '\0')
        return false;

    strtol(str, &end, 10);

    // skip trailing whitespace
    while (isspace((unsigned char)*end))
        end++;

    return (*end == '\0');
}

//---------------------------------------------------------
// read Transform params for DECAL, ENTITY from a file
//---------------------------------------------------------
void ReadEnttTransformParams(FILE* fp, Vec2& outPos, Vec2i& outSize, int& outScale)
{
    assert(fp);

    outPos = Vec2(0, 0);
    outSize = Vec2i(0, 0);
    outScale = 0;

    int count = 0;
    char buffer[128];

    while (fgets(buffer, sizeof(buffer), fp))
    {
        // we've done here
        if (buffer[2] == '}')
            break;

        // +3 to skip '\t' symbols
        const char* buf = buffer + 3;

        char key[16];
        count = sscanf(buf, "%s", key);
        assert(count == 1);

        // read in values according to field type
        if (strcmp(key, "pos:") == 0)
        {
            count = sscanf(buf, "pos: %f %f", &outPos.x, &outPos.y);
            assert(count == 2);
        }
        else if (strcmp(key, "size:") == 0)
        {
            count = sscanf(buf, "size: %d %d", &outSize.w, &outSize.h);
            assert(count == 2);
        }
        else if (strcmp(key, "scale:") == 0)
        {
            count = sscanf(buf, "scale: %d", &outScale);
            assert(count == 1);
        }
        else
        {
            LogErr(LOG, "unknown field type (Transform): %s", key);
        }
    }
}

//---------------------------------------------------------
// extract a percentage value from the input string
//---------------------------------------------------------
inline float GetPercentage(const char* str)
{
    assert(str && str[0] != '\0');

    float percentage = 100.0f;
    int count = sscanf(str, "%f%%", &percentage);
    assert(count == 1);
    assert(percentage <= 100.0f);

    return percentage;
}

//---------------------------------------------------------
// read Transform params for UI elements from a file
//---------------------------------------------------------
void ReadElementUiTransformParams(FILE* fp, Vec2& outPos, Vec2i& outSize, int& outScale)
{
    // we have to calc transformation relatively to the screen size (fullscreen)
    // not window size (which can be smaller for the editor mode)
    const Vec2 wndSz = ToVec2(g_GameState.screenSize);

    int count = 0;
    char buffer[128];

    // position/size might be a string (be represented as percentage or position manipulator)
    char posX[16];
    char posY[16];
    char width[16];
    char height[16];

    //
    // read params from a file
    //
    while (fgets(buffer, sizeof(buffer), fp))
    {
        // we've done here
        if (buffer[2] == '}')
            break;

        // +3 to skip '\t' symbols
        const char* buf = buffer + 3;

        char key[16];
        count = sscanf(buf, "%s", key);
        assert(count == 1);

        // read in values according to field type
        if (strcmp(key, "pos:") == 0)
        {
            count = sscanf(buf, "pos: %s %s", posX, posY);
            assert(count == 2);
        }
        else if (strcmp(key, "size:") == 0)
        {
            count = sscanf(buf, "size: %s %s", width, height);
            assert(count == 2);
        }
        else if (strcmp(key, "scale:") == 0)
        {
            count = sscanf(buf, "scale: %d", &outScale);
            assert(count == 1);
        }
        else
        {
            LogErr(LOG, "unknown field type (Transform): %s", key);
        }
    }

    //
    // according to read values (strings) we calculate final position and size
    //
    if (IsNumber(width))
    {
        outSize.w = atoi(width);
    }
    else
    {
        outSize.w = (int)(wndSz.w * 0.01f * GetPercentage(width));
    }

    if (IsNumber(height))
    {
        outSize.h = atoi(height);
    }
    else
    {
        outSize.h = (int)(wndSz.h * 0.01f * GetPercentage(height));
    }

    // we calculate position <x,y> after size since position may depend on the size
    if (IsNumber(posX))
    {
        outPos.x = (float)atof(posX);
    }
    else
    {
        // if we have a manipulator for position (for instance: "center")
        if (strcmp(posX, "center") == 0)
        {
            const float wndMidX = wndSz.w * 0.5f;
            const float halfW   = (float)(outSize.w / 2);
            outPos.x = wndMidX - halfW;
        }

        // horizontal position is based on percentage from the screen
        else
        {
            const float offset = wndSz.w * 0.01f * GetPercentage(posX);

            // offset from the left edge of the screen
            if (offset >= 0)
                outPos.x = offset;

            // offset from the right edge of the screen
            else
                outPos.x = wndSz.w + offset;
        }
    }

    if (IsNumber(posY))
    {
        outPos.y = (float)atof(posY);
    }
    else
    {
        // if we have a manipulator for position (for instance: center)
        if (strcmp(posY, "center") == 0)
        {
            const float wndMidH = wndSz.h * 0.5f;
            const float halfH   = (float)(outSize.h / 2);
            outPos.y = wndMidH - halfH;
        }

        // vertical position is based on percentage from the screen
        else
        {
            const float offset = wndSz.h * 0.01f * GetPercentage(posY);             

            // offset from the top edge of the screen
            if (offset >= 0)
                outPos.y = offset;

            // offset from the bottom edge of the screen
            else
                outPos.y = wndSz.h + offset;
        }
    }
}

//---------------------------------------------------------
// load Transform component's data from file and add this component to input entity
//---------------------------------------------------------
void LoadTransform(FILE* fp, Entity* pEntt)
{
    assert(fp && pEntt);

    Vec2 pos = { 0,0 };
    Vec2i sz = { 0,0 };
    int scale = 0;

    switch (pEntt->GetLayer())
    {
        case LAYER_ENTITY:
        case LAYER_GND_DECAL:
        {
            ReadEnttTransformParams(fp, pos, sz, scale);
            break;
        }
        case LAYER_UI:
        {
            ReadElementUiTransformParams(fp, pos, sz, scale);
            break;
        }
        default:
        {
            const eLayerType layer = pEntt->GetLayer();
            const char* layerName = g_EnttMgr.GetLayerName(layer);
            LogErr(LOG, "can't read Transform data for entity (%d) with layer: %s (%d)", pEntt->GetId(), layer, layerName);
        }
    }

    assert(sz.w > 0 && sz.h > 0);
    assert(scale > 0);
    pEntt->AddComponent<Transform>(pos.x, pos.y, sz.w, sz.h, scale);
}

//---------------------------------------------------------
// load Movement component's data from file and add this component to input entity
//---------------------------------------------------------
void LoadMovement(FILE* fp, Entity* pEntt)
{
    assert(fp && pEntt);
    assert(!pEntt->HasComponent<Movement>());

    const Vec2 v = FileReadFlt2(fp, "velocity:");
    pEntt->AddComponent<Movement>(v.x, v.y);
}

//---------------------------------------------------------
// load Sprites component's data from file and add this component to input entity
//---------------------------------------------------------
void LoadSprite(FILE* fp, Entity* pEntt)
{
    assert(fp && pEntt);
    assert(!pEntt->HasComponent<Sprite>());

    char buffer[128];
    char fieldName[32];
    char animName[64];
    int count = 0;
    TexID texId = INVALID_TEX_ID;
    Vec2i srcRectPos = { 0,0 };
    Vec2i srcRectSz = { 0,0 };
    Color color = { 0 };
    Animation animations[NUM_ANIM_TYPES];
    int bAnimated = 0;
    int bShow = 0;
    int numFrames = 0;
    int animSpeed = 0;
    int bFixedOnScreen = 0;
    int zIndex = 0;
    bool bHasManyAnimTypes = false;

    while (fgets(buffer, sizeof(buffer), fp))
    {
        // we've done
        if (buffer[2] == '}')
            break;

        // +3 to skip '\t' symbols
        const char* buf = buffer + 3;

        //
        // get field name (propery type)
        //
        count = sscanf(buf, "%s", fieldName);
        if (count != 1)
        {
            LogErr(LOG, "can't get a Sprite component property from str: %s", buf);
            continue;
        }

        // skip ':' at the end of the fieldName
        fieldName[strlen(fieldName)-1] = '\0';

        //
        // load params according to field type
        //
        if (strcmp(fieldName, "texture") == 0)
        {
            char texName[MAX_LEN_TEX_NAME];
            count = sscanf(buf, "texture: %s", texName);
            assert(count == 1);
            
            texId = g_AssetMgr.GetTexId(texName);
        }

        else if (strcmp(fieldName, "is_animated") == 0)
        {
            count = sscanf(buf, "is_animated: %d", &bAnimated);
            assert(count == 1);
        }

        else if (strcmp(fieldName, "is_fixed_on_screen") == 0)
        {
            count = sscanf(buf, "is_fixed_on_screen: %d", &bFixedOnScreen);
            assert(count == 1);
        }

        else if (strcmp(fieldName, "is_show") == 0)
        {
            count = sscanf(buf, "is_show: %d", &bShow);
            assert(count == 1);
        }

        else if (strcmp(fieldName, "src_rect_pos") == 0)
        {
            count = sscanf(buf, "src_rect_pos: %d %d", &srcRectPos.x, &srcRectPos.y);
            assert(count == 2);
        }

        else if (strcmp(fieldName, "src_rect_size") == 0)
        {
            count = sscanf(buf, "src_rect_size: %d %d", &srcRectSz.w, &srcRectSz.h);
            assert(count == 2);
        }

        else if (strcmp(fieldName, "color") == 0)
        {
            int r, g, b, a;
            count = sscanf(buf, "color: %d %d %d %d", &r, &g, &b, &a);
            assert(count == 4);

            color.rgba = RGBATOU32(r,g,b,a);
        }

        else if (strcmp(fieldName, "z_index") == 0)
        {
            count = sscanf(buf, "z_index: %d", &zIndex);
            assert(count == 1);
            assert(zIndex >= INT8_MIN && zIndex <= INT8_MAX);
        }

        else if (strcmp(fieldName, "texture") == 0)
        {
            assert(!bAnimated);

            char texName[MAX_LEN_TEX_NAME];
            texName[0] = '\0';

            count = sscanf(buf, "texture: %s", texName);
            assert(count == 1);

            texId = g_AssetMgr.GetTexId(texName);
        }

        //
        // read in animations
        //
        else if (strncmp(fieldName, "anim_", 5) == 0)
        {
            assert(bAnimated);
            
            const char* animTypeStr = fieldName + 5;

            eAnimType animType = Sprite::GetAnimType(animTypeStr);
            if (animType == NUM_ANIM_TYPES)
            {
                LogErr(LOG, "can't define animation type by str: %s", animTypeStr);
                continue;
            }

            // read animation name
            count = sscanf(buf, "%s %s", fieldName, animName);
            assert(count == 2);

            // setup animation by type
            animations[animType] = g_AssetMgr.GetAnim(animName);

            if (animType != ANIM_TYPE_IDLE)
                bHasManyAnimTypes = true;
        }

        else
            LogErr(LOG, "unknown type of sprite's prop: %s", fieldName);
    }

    // add a component and setup it
    Sprite* pSpr = nullptr;

    // no animation (one frame), no directions, one texture
    if (!bAnimated)
    {
        pSpr = &pEntt->AddComponent<Sprite>(texId, bFixedOnScreen);
    }
        
    // animated, but has only one animation type
    else if (bAnimated && !bHasManyAnimTypes)
    {
        pSpr = &pEntt->AddComponent<Sprite>(animations[ANIM_TYPE_IDLE], bFixedOnScreen);
    }

    // has multiple animations
    else if (bAnimated && bHasManyAnimTypes)
    {
        pSpr = &pEntt->AddComponent<Sprite>(
            animations[ANIM_TYPE_IDLE],
            animations[ANIM_TYPE_MOVE],
            animations[ANIM_TYPE_ATTACK],
            animations[ANIM_TYPE_DEATH],
            animations[ANIM_TYPE_DAMAGE],
            animations[ANIM_TYPE_BLOCK],
            bFixedOnScreen);
    }

    assert(pSpr);
    pSpr->SetShow(bShow);
    pSpr->SetSrcRectPos(srcRectPos);
    pSpr->SetSrcRectSize(srcRectSz);
    pSpr->SetColor(color);
    pSpr->SetZIndex(zIndex);
}

//---------------------------------------------------------
// load Keyboard component's data from file and add this component to input entity
//---------------------------------------------------------
void LoadKeyboard(FILE* fp, Entity* pEntt)
{
    assert(fp && pEntt);

    int count = 0;
    char buffer[128];
    char keyBind[NUM_KEY_TYPES][32];
    memset(keyBind, 0, sizeof(keyBind));

    while (fgets(buffer, sizeof(buffer), fp))
    {
        // we've done
        if (buffer[2] == '}')
            break;

        // +3 to skip '\t' symbols
        const char* buf = buffer + 3;

        char keyType[32];
        char keyName[32];

        count = sscanf(buf, "%s %s", keyType, keyName);
        assert(count == 2);

        // skip ':' symbol at the end
        keyType[strlen(keyType)-1] = '\0';

        // find binding
        for (int i = 0; i < NUM_KEY_TYPES; ++i)
        {
            if (strcmp(keyType, s_KeyTypeNames[i]) != 0)
                continue;

            strcpy(keyBind[i], keyName);
        }
    }

    // all bindings are valid?
    for (int i = 0; i < NUM_KEY_TYPES; ++i)
    {
        assert(keyBind[i][0] != '\0');
    }

    
    pEntt->AddComponent<Keyboard>(keyBind[KEY_PLAYER_UP],
                                  keyBind[KEY_PLAYER_LEFT],
                                  keyBind[KEY_PLAYER_DOWN],
                                  keyBind[KEY_PLAYER_RIGHT]);
}

//---------------------------------------------------------
// load EnemyAI component's data from file and add this component to input entity
//---------------------------------------------------------
void LoadEnemy(FILE* fp, Entity* pEntt)
{
    assert(fp && pEntt);
    assert(!pEntt->HasComponent<Enemy>());

    char buf[128];
    char key[32];
    char value[32];
    int count = 0;
    eEnemyType enemyType = eEnemyType(-1);

    while (fgets(buf, sizeof(buf), fp))
    {
        // we've done here
        if (buf[2] == '}')
            break;

        // read a pair [key => value]
        count = sscanf(buf, "%s %s", key, value);
        assert(count == 2);

        if (strcmp(key, "type:") == 0)
        {
            enemyType = Enemy::GetTypeByTypeName(value);
        }
    }

    pEntt->AddComponent<Enemy>(enemyType);
}

//---------------------------------------------------------
// load Health component's data from file and add this component to input entity
//---------------------------------------------------------
void LoadHealth(FILE* fp, Entity* pEntt)
{
    assert(fp && pEntt);
    assert(!pEntt->HasComponent<Health>());

    const int currHP     = FileReadInt(fp, "hp:");
    const int maxHP      = FileReadInt(fp, "max_hp:");
    const int healPerSec = FileReadInt(fp, "heal_per_sec:");

    pEntt->AddComponent<Health>(currHP, maxHP, healPerSec);
}

//---------------------------------------------------------
// load Damage component's data from file and add this component to input entity
//---------------------------------------------------------
void LoadDamage(FILE* fp, Entity* pEntt)
{
    assert(fp && pEntt);
    assert(!pEntt->HasComponent<Damage>());

    const float cooldownSec = FileReadFlt(fp, "cooldown_max_sec:");
    const float      damage = FileReadFlt(fp, "damage:");

    pEntt->AddComponent<Damage>(cooldownSec, damage);
}

//---------------------------------------------------------
// load Collider component's data from file and add this component to input entity
//---------------------------------------------------------
void LoadCollider(FILE* fp, Entity* pEntt)
{
    assert(fp && pEntt);
    assert(!pEntt->HasComponent<Collider>());

    int count = 0;
    int bSolid = 0;
    char colliderTypeName[64];
    eColliderType colType = NUM_COLLIDER_TYPES;

    // read in a type of collider
    FileReadStr(fp, "type:", colliderTypeName);
    colType = Collider::GetTypeByName(colliderTypeName);

    // if true - we can't move through this collider
    bSolid = FileReadInt(fp, "is_solid:");

    // according to collider's type we read in its params
    if (colType == COLLIDER_BOX)
    {
        const Vec2i p  = FileReadInt2(fp, "pos:");
        const Vec2i sz = FileReadInt2(fp, "size:");

        Collider& c = pEntt->AddComponent<Collider>(p, sz);
        c.SetSolid(bSolid);
    }
    else if (colType == COLLIDER_CIRCLE)
    { 
        Vec2i center = FileReadInt2(fp, "center:");
        int   radius = FileReadInt(fp, "radius:");

        Collider& c = pEntt->AddComponent<Collider>(center, radius);
        c.SetSolid(bSolid);
    }
    else
    {
        LogErr(LOG, "unknown type (%d) of collider for entt (%s)", (int)colType, pEntt->GetName());
        return;
    }
}

//---------------------------------------------------------
// load Trigger component's data from file and add this component to input entity
//---------------------------------------------------------
void LoadTrigger(FILE* fp, Entity* pEntt)
{
    assert(fp && pEntt);
    assert(!pEntt->HasComponent<Trigger>());

    TriggerInitParams initParams;
    memset(&initParams, 0, sizeof(initParams));

    int bTrigOnlyByPlayer = 0;
    char triggerTypeName[32];
    char eventName[MAX_LEN_EVENT_NAME];

    triggerTypeName[0] = '\0';
    eventName[0] = '\0';

    //
    // read data
    //
    initParams.bTriggersOnlyByPlayer = (bool)FileReadInt(fp, "is_trig_only_by_player:");

    FileReadStr(fp, "type:", triggerTypeName);
    initParams.type = Trigger::GetTypeByName(triggerTypeName);
    assert(initParams.type != NUM_TRIGGER_TYPES);

    FileReadStr(fp, "on_enter_event:", eventName);
    initParams.onEnter = g_pFacadeEngine->GetEventIdByName(eventName);

    // read in params specific for "TRIGGER_MULTIPLE"
    if (initParams.type == TRIGGER_MULTIPLE)
    {
        FileReadStr(fp, "on_collide_event:", eventName);
        initParams.onCollide = g_pFacadeEngine->GetEventIdByName(eventName);

        FileReadStr(fp, "on_leave_event:", eventName);
        initParams.onLeave = g_pFacadeEngine->GetEventIdByName(eventName);
    }

    pEntt->AddComponent<Trigger>(initParams);
}

//---------------------------------------------------------
// load Hierarchy component's data from file and add this component to input entity
//---------------------------------------------------------
void LoadHierarchy(FILE* fp, Entity* pEntt)
{
    assert(fp && pEntt);
    assert(!pEntt->HasComponent<Hierarchy>());

    int count = 0;
    char buf[128];

    Entity* children[MAX_CHILD_ENTTS]{ nullptr };
    Entity* pParent = nullptr;              // optional: entt/prefab may have or have not a parent
    int numChildren = 0;
    Vec2 relativePos = { 0,0 };
    bool bHasChildExist = false;            // do we have any children to attach to this entity?

    // do we want to load a hierarchy for entity or prefab?
    bool isEntt = g_EnttMgr.HasEntt(pEntt->GetId());


    while (fgets(buf, sizeof(buf), fp))
    {
        // we've done here
        if (buf[2] == '}')
            break;

        char key[32];
        count = sscanf(buf, "%s", key);
        assert(count == 1);

        // read data according to the key (field) type
        if (strcmp(key, "parent:") == 0)
        {
            char parentName[MAX_LEN_ENTT_NAME];
            count = sscanf(buf+3, "parent: %s", &parentName);
            assert(count == 1);

            // if entity/prefab by this name actually has a parent
            if (parentName[0] != '0')
            {
                if (isEntt)
                    pParent = g_EnttMgr.GetEntt(parentName);
                else
                    pParent = g_EnttMgr.GetPrefab(parentName);
            }
                
        }

        else if (strcmp(key, "rel_pos:") == 0)
        {
            count = sscanf(buf+3, "rel_pos: %f %f", &relativePos.x, &relativePos.y);
            assert(count == 2);
        }

        else if (strcmp(key, "child:") == 0)
        {
            char childName[MAX_LEN_ENTT_NAME];
            count = sscanf(buf+3, "child: %s", childName);
            assert(count == 1);

            Entity* pChild = nullptr;

            if (isEntt)
            {
                const EntityID childId = g_EnttMgr.HasEntt(childName);

                // if child hasn't been created yet we skip it 
                // (later when child is created we set a parent which is this entt)
                if (childId == INVALID_ENTT_ID)
                    continue;

                pChild = g_EnttMgr.GetEntt(childId);
            }
            else
            {
                const EntityID childId = g_EnttMgr.HasPrefab(childName);

                // if child hasn't been created yet we skip it 
                // (later when child is created we set a parent which is this entt)
                if (childId == INVALID_ENTT_ID)
                    continue;

                pChild = g_EnttMgr.GetPrefab(childId);
            }

            children[numChildren] = pChild;
            numChildren++;
            bHasChildExist = true;
        }
    }

    Hierarchy& h = pEntt->AddComponent<Hierarchy>();
    h.SetRelativePos(relativePos.x, relativePos.y);

    // if has a parent and parent already exists
    if (pParent)
    {
        Hierarchy* pParentHierarchy = pParent->GetComponent<Hierarchy>();

        // if parent doesn't have the hierarchy yet we add it
        if (!pParentHierarchy)
            pParentHierarchy = &pParent->AddComponent<Hierarchy>();

        h.SetParent(pParent->GetId());
        pParentHierarchy->AddChild(pEntt);
    }

    // if has any child and this child already exists
    if (bHasChildExist)
    {
        for (int i = 0; i < numChildren; ++i)
            h.AddChild(children[i]);
    }
}

//---------------------------------------------------------
// load LifeTimer component's data from file and add this component to input entity
//---------------------------------------------------------
void LoadLifeTimer(FILE* fp, Entity* pEntt)
{
    assert(fp && pEntt);
    assert(!pEntt->HasComponent<LifeTimer>());

    const float t = FileReadFlt(fp, "life_time:");
    assert(t > 0);

    pEntt->AddComponent<LifeTimer>(t);
}


//=============================================================================
//=============================================================================
// PUBLIC FUNCTIONAL
//=============================================================================
//=============================================================================

//---------------------------------------------------------
// default constructor: 
// bind handlers for components saving/loading
//---------------------------------------------------------
EntitySaveLoad::EntitySaveLoad()
{
    s_SaversFunc.insert({ &typeid(Transform),   SaveTransform });
    s_SaversFunc.insert({ &typeid(Sprite),      SaveSprite });
    s_SaversFunc.insert({ &typeid(Keyboard),    SaveKeyboard });
    s_SaversFunc.insert({ &typeid(Enemy),       SaveEnemy });
    s_SaversFunc.insert({ &typeid(Health),      SaveHealth });
    s_SaversFunc.insert({ &typeid(Damage),      SaveDamage });
    s_SaversFunc.insert({ &typeid(Collider),    SaveCollider });
    s_SaversFunc.insert({ &typeid(Movement),    SaveMovement });
    s_SaversFunc.insert({ &typeid(Player),      SavePlayer });
    s_SaversFunc.insert({ &typeid(Trigger),     SaveTrigger });
    s_SaversFunc.insert({ &typeid(Hierarchy),   SaveHierarchy });
    s_SaversFunc.insert({ &typeid(LifeTimer),   SaveLifeTimer });

    s_LoadersFunc.insert({ &typeid(Transform),  LoadTransform });
    s_LoadersFunc.insert({ &typeid(Sprite),     LoadSprite });
    s_LoadersFunc.insert({ &typeid(Keyboard),   LoadKeyboard });
    s_LoadersFunc.insert({ &typeid(Enemy),      LoadEnemy });
    s_LoadersFunc.insert({ &typeid(Health),     LoadHealth });
    s_LoadersFunc.insert({ &typeid(Damage),     LoadDamage });
    s_LoadersFunc.insert({ &typeid(Collider),   LoadCollider });
    s_LoadersFunc.insert({ &typeid(Movement),   LoadMovement });
    s_LoadersFunc.insert({ &typeid(Player),     LoadPlayer });
    s_LoadersFunc.insert({ &typeid(Trigger),    LoadTrigger });
    s_LoadersFunc.insert({ &typeid(Hierarchy),  LoadHierarchy });
    s_LoadersFunc.insert({ &typeid(LifeTimer),  LoadLifeTimer });
}

//---------------------------------------------------------
// Args:  name - name of component to get its type info
//---------------------------------------------------------
const type_info* GetTypeInfoByComponentName(const char* name)
{
    assert(name && name[0] != '\0');

    if      (strcmp(name, "Transform") == 0)    return (&typeid(Transform));
    else if (strcmp(name, "Sprite") == 0)       return (&typeid(Sprite));
    else if (strcmp(name, "Keyboard") == 0)     return (&typeid(Keyboard));
    else if (strcmp(name, "Enemy") == 0)        return (&typeid(Enemy));
    else if (strcmp(name, "Health") == 0)       return (&typeid(Health));
    else if (strcmp(name, "Damage") == 0)       return (&typeid(Damage));
    else if (strcmp(name, "Collider") == 0)     return (&typeid(Collider));
    else if (strcmp(name, "Movement") == 0)     return (&typeid(Movement));
    else if (strcmp(name, "Player") == 0)       return (&typeid(Player));
    else if (strcmp(name, "Trigger") == 0)      return (&typeid(Trigger));
    else if (strcmp(name, "Hierarchy") == 0)    return (&typeid(Hierarchy));
    else if (strcmp(name, "LifeTimer") == 0)    return (&typeid(LifeTimer));

    LogErr(LOG, "can't get a type info by component name: %s", name);
    return nullptr;
}

//---------------------------------------------------------
// save entity and its components data into a file
//---------------------------------------------------------
void EntitySaveLoad::SaveEntt(FILE* fp, const Entity* pEntt)
{
    assert(fp);
    assert(pEntt);
    assert(!pEntt->HasComponent<Tile>());   // TILE entities we save separately (when saving a map)

    PrintOffset(fp, 1);
    fprintf(fp, "entt %s {\n", pEntt->GetName());
    PrintOffset(fp, 2);
    fprintf(fp, "layer: %s\n", s_LayerNames[pEntt->GetLayer()]);

    // save Transform component manually at the very beginning 
    // since many other components are dependent on it
    const auto item = s_SaversFunc.find(&typeid(Transform));
    if (item != s_SaversFunc.end())
    {
        PrintOffset(fp, 2);
        fprintf(fp, "component Transform {\n");
        item->second(fp, pEntt);
        fputs("\t\t}\n", fp);
    }
    else
    {
        LogErr(LOG, "no handlers for saving (%s) entity component (Transform) was found", pEntt->GetName());
        return;
    }

    // save components data of this entity
    for (const auto& elem : pEntt->GetComponentsMap())
    {
        // skip the Transform component since it is already saved
        if (elem.first == (&typeid(Transform)))
            continue;

        const auto it = s_SaversFunc.find(elem.first);
        if (it == s_SaversFunc.end())
        {
            LogErr(LOG, "no handlers for saving entity component (%s) was found", elem.second->GetComponentName());
            continue;
        }

        // print beginning of the declaration
        PrintOffset(fp, 2);
        fprintf(fp, "component %s {\n", elem.second->GetComponentName());

        // write component's data
        s_SaversFunc[elem.first](fp, pEntt);

        fputs("\t\t}\n", fp);
    }

    fputs("\t}\n", fp);
}

//---------------------------------------------------------
// load entity and its components data from file
//---------------------------------------------------------
void EntitySaveLoad::LoadEntt(FILE* fp, Entity* pEntt)
{
    assert(fp);
    assert(pEntt);

    char buf[128];
    char componentName[32];
    int count = 0;

    while (fgets(buf, sizeof(buf), fp))
    {
        // we've done for this entt 
        if (buf[1] == '}')
            return;

        // +2 to skip '\t' symbols
        if (strncmp(buf+2, "component", 9) != 0)
            continue;

        // get component name
        count = sscanf(buf+2, "component %s", componentName);
        if (count != 1)
        {
            LogErr(LOG, "can't get a component name from str: %s", buf+2);
            continue;
        }

        const type_info* pTypeInfo = GetTypeInfoByComponentName(componentName);

        // load component data
        if (pTypeInfo != nullptr)
        {
            s_LoadersFunc[pTypeInfo](fp, pEntt);
        }
        else
        {
            LogFatal(LOG, "unknown component to read: %s", componentName);
        }
    }
}