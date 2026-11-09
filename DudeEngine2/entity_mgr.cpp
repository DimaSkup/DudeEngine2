/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: entity_mgr.cpp
    Created:  by DimaSkup
\**********************************************************************************/
#include "entity_mgr.h"
#include "mem_helpers.h"
#include "log.h"
#include "entity_save_load.h"
#include <time.h>

// components
#include "components/transform.h"
#include "components/sprite.h"
#include "components/tile.h"
#include "components/damage.h"
#include "components/health.h"
#include "components/enemy.h"
#include "components/collider.h"
#include "components/trigger.h"
#include "components/life_timer.h"
#include "components/hierarchy.h"
#include "components/movement.h"


//---------------------------------------------------------
// global instance
//---------------------------------------------------------
EntityMgr g_EnttMgr;

static EntityID s_EnttId = 0;

//---------------------------------------------------------
// internal helpers
//---------------------------------------------------------
inline bool IsEmptyStr(const char* str)
{
    return (!str || str[0] == '\0');
}

inline bool IsValidLayer(const eLayerType layer)
{
    return (layer >= 0 && layer < NUM_LAYERS);
}

//---------------------------------------------------------
// create default-dummy entity and prefab
// (we use it when ask for wrong data to prevent crash)
//---------------------------------------------------------
void EntityMgr::CreateDummyStuff()
{
    // entity
    Entity* pInvalid = AddEntt("invalid", LAYER_UI);
    const Vec2 pos = { 0,0 };
    const Vec2i sz = { 100, 100 };
    const int scale = 1;
    const bool bFixedOnScreen = false;

    pInvalid->AddComponent<Transform>(pos, sz, scale);
    Sprite& spr = pInvalid->AddComponent<Sprite>(INVALID_TEX_ID, bFixedOnScreen);
    spr.SetSrcRectSize(sz);
    spr.SetShow(false);

    // prefab
    AddPrefab("invalid_prefab", LAYER_UI);
}

//---------------------------------------------------------
// default constructor
//---------------------------------------------------------
EntityMgr::EntityMgr() :
    playerId_(INVALID_ENTT_ID)
{
    // render all the layers by default
    for (int i = 0; i < NUM_LAYERS; ++i)
        bShowLayers_[i] = true;
}

//---------------------------------------------------------
// Desc:  remove all the entities
//---------------------------------------------------------
void EntityMgr::ClearData()
{
    for (Entity* pEntt : entities_)
    {
        pEntt->Shutdown();
        SafeDelete(pEntt);
    }

    for (int i = 0; i < NUM_LAYERS; ++i)
        entitiesByLayer_[i].purge();

    for (Entity* pPrefab : prefabs_)
    {
        pPrefab->Shutdown();
        SafeDelete(pPrefab);
    }

    ids_.purge();
    entities_.purge();
    enttsToRemove_.clear();

    prefabs_.purge();
    prefabsNames_.purge();

    s_EnttId = 0;
    playerId_ = 0;
}

//---------------------------------------------------------
// Desc:  get a layer type by input name
//---------------------------------------------------------
eLayerType EntityMgr::GetLayerType(const char* layerName)
{
    if (IsEmptyStr(layerName))
        return NUM_LAYERS;

    for (int i = 0; i < NUM_LAYERS; ++i)
    {
        if (strcmp(layerName, s_LayerNames[i]) == 0)
            return eLayerType(i);
    }

    return NUM_LAYERS;
}

//---------------------------------------------------------
// Desc:  load entities (scene objects) from a file
//---------------------------------------------------------
void EntityMgr::LoadEntts(const char* filePath)
{
    assert(filePath && filePath[0] != '\0');

    EntitySaveLoad loader;
    FILE* fp = fopen(filePath, "r");
    if (!fp)
    {
        LogErr(LOG, "%s%s", ERR_OPEN_FILE, filePath);
        return;
    }

    char buf[128];
    char enttName[MAX_LEN_ENTT_NAME];
    char layerName[32];
    int count = 0;

    // skip comments section
    while (fgets(buf, sizeof(buf), fp))
    {
        if (buf[0] != ';')
            break;
    }

    // check if we read a valid file
    if (strncmp(buf, "entities", 8) != 0)
    {
        LogErr(LOG, "%s: %s", ERR_INV_FILE, filePath);
        fclose(fp);
        return;
    }

    
    // read in declarations of entities
    while (fgets(buf, sizeof(buf), fp))
    {
        // +1 to skip '\t'
        if (strncmp(buf+1, "entt", 4) != 0)
            continue;

        // get name for a new entity
        count = sscanf(buf+1, "entt %s", enttName);
        if (count != 1)
        {
            LogErr(LOG, "err read entt's name from str: %s", buf+1);
            continue;
        }

        // get a layer for a new entity
        fgets(buf, sizeof(buf), fp);
        count = sscanf(buf+2, "layer: %s", layerName);
        if (count != 1)
        {
            LogErr(LOG, "err read entt's layer name from str: %s", buf+2);
            continue;
        }

        const eLayerType layer = GetLayerType(layerName);
        if (layer == NUM_LAYERS)
        {
            LogErr(LOG, "err read layer's type by layer name: %s", layerName);
            continue;
        }

        Entity* pEntt = AddEntt(enttName, layer);
        loader.LoadEntt(fp, pEntt);
    }

    fclose(fp);
}

//---------------------------------------------------------
// Desc:  save entities (UI elements) into a file
//---------------------------------------------------------
void EntityMgr::SaveUiEntts(const char* filepath)
{
    assert(filepath && filepath[0] != '\0');

    EntitySaveLoad saver;
    FILE* fp = fopen(filepath, "w");
    if (!fp)
    {
        LogErr(LOG, "%s%s", ERR_OPEN_FILE, filepath);
        return;
    }

    fputs(";===============================================\n", fp);
    fputs("; declare entities (UI elements) here\n", fp);
    fputs(";===============================================\n", fp);
    fputs("entities {\n", fp);

    // save each UI element (begin from 1 because by 0 idx we have "invalid/default" 
    // entity which is created automatically, so don't save it)
    const cvector<Entity*>& entts = entitiesByLayer_[LAYER_UI];

    for (index i = 1; i < entts.size(); ++i)
        saver.SaveEntt(fp, entts[i]);

    fputc('}', fp);
    fclose(fp);
}

//---------------------------------------------------------
// Desc:  save entities (world objects) into a file
//---------------------------------------------------------
void EntityMgr::SaveEntts(const char* filePath)
{
    assert(filePath && filePath[0] != '\0');

    EntitySaveLoad saver;
    FILE* fp = fopen(filePath, "w");
    if (!fp)
    {
        LogErr(LOG, "%s%s", ERR_OPEN_FILE, filePath);
        return;
    }

    fputs(";===============================================\n", fp);
    fputs("; declare entities (scene objects) here\n", fp);
    fputs(";===============================================\n", fp);
    fputs("entities {\n", fp);

    // save each entity (begin from 1 because by 0 idx we have "invalid/default" 
    // entity which is created automatically, so don't save it)
    for (index i = 1; i < entities_.size(); ++i)
    {
        Entity* pEntt = entities_[i];
        assert(pEntt);

        const eLayerType layer = pEntt->GetLayer();
        
        // tiles of a map and UI elements we save separately
        if (layer == LAYER_TILEMAP || layer == LAYER_UI)
            continue;

        saver.SaveEntt(fp, pEntt);
    }

    fputs("}", fp);
    fclose(fp);
}

//---------------------------------------------------------
// Desc:  check if we have any entities
//---------------------------------------------------------
bool EntityMgr::HasEntts() const
{
    return entities_.size() != 0;
}

//---------------------------------------------------------
// Desc:  check if we have an entity by input name
// Ret:   ID (true) of entity by such a name, or 0 (false) if there is no such entity
//---------------------------------------------------------
EntityID EntityMgr::HasEntt(const char* enttName) const
{
    assert(!IsEmptyStr(enttName));

    for (Entity* pEntt : entities_)
    {
        if (strcmp(pEntt->GetName(), enttName) == 0)
            return pEntt->GetId();
    }

    return INVALID_ENTT_ID;
}

//---------------------------------------------------------
// Desc:  check if we have an entity by input ID
//---------------------------------------------------------
bool EntityMgr::HasEntt(const EntityID id) const
{
    return ids_.binary_search(id);
}

//---------------------------------------------------------
// Desc:  return how many entities we have
//---------------------------------------------------------
uint EntityMgr::GetNumEntts() const
{
    return (uint)entities_.size();
}

//---------------------------------------------------------
// Desc:  update all the entities in the manager
// Args:  dt - delta time
//---------------------------------------------------------
void EntityMgr::Update(const float dt)
{
    for (Entity* pEntt : entities_)
    {
        pEntt->Update(dt);
    }

    if (!enttsToRemove_.empty())
    {
        RemoveEntts(enttsToRemove_.data(), enttsToRemove_.size());
        enttsToRemove_.clear();
    }

    // setup the main player's ID
    playerId_ = GetEntt("player")->GetId();
    assert(playerId_ != INVALID_ENTT_ID);
}

//---------------------------------------------------------
// immediate removing of all the entities by input IDs
//---------------------------------------------------------
void EntityMgr::RemoveEntts(const EntityID* ids, const vsize count)
{
    assert(ids);
    assert(count > 0);

    for (index i = 0; i < count; ++i)
    {
        Entity* pEntt = GetEntt(ids[i]);

        if (pEntt->GetId() != INVALID_ENTT_ID)
            RemoveEntts(&pEntt, 1);
    }
}

//---------------------------------------------------------
// immediate removing of all the input entities 
//---------------------------------------------------------
void EntityMgr::RemoveEntts(Entity** enttsToRemove, const vsize count)
{
    assert(enttsToRemove);
    assert(count > 0);

    // 1. remove entts from the main list 
    for (index i = 0; i < count; ++i)
    {
        Entity* pEntt = enttsToRemove[i];
        assert(pEntt);

        RemoveEnttFromMainLists(pEntt->GetId());
        
        // remove its children as well (if it has any)
        const Hierarchy* h = pEntt->GetComponent<Hierarchy>();
        if (h)
        {
            const EntityID* childrenIds = h->GetChildren();
            const size_t    numChildren = h->GetNumChildren();

            // this entt has only parent, no children
            if (numChildren == 0)
                continue;

            assert(childrenIds);

            for (size_t childIdx = 0; childIdx < numChildren; ++childIdx)
            {
                Entity* pChild = GetEntt(childrenIds[childIdx]);
                RemoveEntts(&pChild, 1);
            }
        }
    }

    // 2. remove entts from the layers list
    for (index i = 0; i < count; ++i)
    {
        Entity* pEntt = enttsToRemove[i];
        RemoveEnttFromLayer(pEntt, pEntt->GetLayer());
    }

    // 3. and release memory
    for (index i = 0; i < count; ++i)
    {
        Entity* pEntt = enttsToRemove[i];
        pEntt->Shutdown();
        SafeDelete(pEntt);
    }

    // check if we didn't fuck up
    assert(ids_.size() == entities_.size());
}

//---------------------------------------------------------
//---------------------------------------------------------
void EntityMgr::RemoveEnttFromMainLists(const EntityID id)
{
    const index i = ids_.get_idx(id);

    if (!ids_.is_valid_index(i) || ids_[i] != id)
    {
        LogErr(LOG, "no entt by id: %d", (int)id);
        return;
    }

    ids_.erase(i);
    entities_.erase(i);
}

//---------------------------------------------------------
//---------------------------------------------------------
void EntityMgr::RemoveEnttFromLayer(const Entity* pEntt, const eLayerType layer)
{
    assert(pEntt);
    assert(layer >= 0 && layer < NUM_LAYERS);

    cvector<Entity*>& enttsByLayer = entitiesByLayer_[layer];

    // TODO: linear search -- NEED TO OPTIMIZE IT: refactor for binary searching
    for (index i = 0; i < enttsByLayer.size(); ++i)
    {
        if (pEntt == enttsByLayer[i])
        {
            // swap & pop
            enttsByLayer[i] = enttsByLayer.back();
            enttsByLayer.pop_back();
            i = enttsByLayer.size();
        }
    }
}

//---------------------------------------------------------
// Desc:  check if we have a prefab by ID
//---------------------------------------------------------
bool EntityMgr::HasPrefab(const EntityID id) const
{
    for (const Entity* pPrefab : prefabs_)
    {
        if (pPrefab->GetId() == id)
            return true;
    }

    return false;
}

//---------------------------------------------------------
// Desc:  check if we have a prefab by name
// Ret:   ID (true) of prefab by such a name, or 0 (false) if there is no such prefab
//---------------------------------------------------------
EntityID EntityMgr::HasPrefab(const char* prefabName) const
{
    assert(!IsEmptyStr(prefabName));

    for (index i = 0; i < prefabs_.size(); ++i)
    {
        if (strcmp(prefabsNames_[i].name, prefabName) == 0)
            return prefabs_[i]->GetId();
    }

    return INVALID_ENTT_ID;
}

//---------------------------------------------------------
// Desc:  get a prefab entity by input name
//---------------------------------------------------------
Entity* EntityMgr::GetPrefab(const char* prefabName)
{
    if (IsEmptyStr(prefabName))
    {
        LogErr(LOG, ERR_EMPTY_NAME);
        return prefabs_[0];
    }

    assert(prefabs_.size() == prefabsNames_.size());

    for (index i = 0; i < prefabs_.size(); ++i)
    {
        if (strcmp(prefabsNames_[i].name, prefabName) == 0)
            return prefabs_[i];
    }

    LogErr(LOG, "no prefab by name: %s", prefabName);
    return prefabs_[0];
}

//---------------------------------------------------------
// Desc:  get a prefab entity by input id
//---------------------------------------------------------
Entity* EntityMgr::GetPrefab(const EntityID id)
{
    for (Entity* pPrefab : prefabs_)
    {
        if (pPrefab->GetId() == id)
            return pPrefab;
    }

    LogErr(LOG, "no prefab by id: %d", id);
    return prefabs_[0];
}

//---------------------------------------------------------
// Desc:  create a new prefab based on input entity
// Ret:   a ptr to created prefab
//---------------------------------------------------------
Entity* EntityMgr::AddPrefab(const Entity* pEntt)
{
    assert(pEntt);

    char prefabName[MAX_LEN_ENTT_NAME];
    snprintf(prefabName, sizeof(prefabName), "prefab%d%ld", s_EnttId, (long)time(NULL));

    Entity* pPrefab = AddPrefab(prefabName, pEntt->GetLayer());

    for (const auto& it : pEntt->GetComponentsMap())
    {
        CloneComponent(pPrefab, pEntt, it.second->GetComponentType());
    }

    return pPrefab;
}

//---------------------------------------------------------
// Desc:  add a new prefab with name
//---------------------------------------------------------
Entity* EntityMgr::AddPrefab(const char* prefabName, const eLayerType layer)
{
    if (IsEmptyStr(prefabName))
    {
        LogErr(LOG, ERR_EMPTY_NAME);
        return prefabs_[0];
    }

    if (!IsValidLayer(layer))
    {
        LogErr(LOG, "invalid layer (layer code: %d)", (int)layer);
        return prefabs_[0];
    }

    // check if input name is unique
    if (HasPrefab(prefabName))
    {
        LogErr(LOG, "there is already a prefab by name: %s", prefabName);
        return prefabs_[0];
    }


    // create a new prefab entity
    Entity* pEntt = new Entity(s_EnttId, prefabName, layer);
    ++s_EnttId;

    prefabs_.push_back(pEntt);
    prefabsNames_.push_back(PrefabName());

    // store a name
    char* name = prefabsNames_.back().name;
    snprintf(name, MAX_LEN_ENTT_NAME, "%s", prefabName);

    return pEntt;
}

//---------------------------------------------------------
// Desc:  create a new instance of input prefab
//---------------------------------------------------------
Entity* EntityMgr::Instantiate(const Entity* prefab)
{
    const char* baseName = prefab->GetName();
    assert(strcmp(baseName, "player") != 0);

    //
    // generate a name for this instance
    //
    const char* prefixEnd = strchr(baseName, '_');
    assert(prefixEnd);

    const size_t prefixLen = abs(prefixEnd - baseName);
    assert(prefixLen > 0);

    char prefix[32];
    memset(prefix, 0, sizeof(prefix));

    assert(prefixLen < sizeof(prefix));
    strncpy(prefix, baseName, prefixLen);

    char name[MAX_LEN_ENTT_NAME];
    snprintf(name, MAX_LEN_ENTT_NAME-1, "%s_%d_%ld", prefix, s_EnttId, (long)time(NULL));

    if (!IsUniqueEnttName(name))
    {
        LogErr(LOG, "can't create entity instance of \"%s\"", prefab->GetName());
        return entities_[0];
    }

    //
    // instantiate
    //
    Entity* pEntt = AddEntt(name, prefab->GetLayer());

    for (const auto& it : prefab->GetComponentsMap())
    {
        CloneComponent(pEntt, prefab, it.second->GetComponentType());
    }

    return pEntt;
}

//---------------------------------------------------------
// clone a component of input type from pSrc entity into pDst entity
//---------------------------------------------------------
void EntityMgr::CloneComponent(Entity* pDst, const Entity* pSrc, const eComponentType type)
{
    assert(pDst);
    assert(pSrc);

    if (type < 0 && type >= NUM_COMPONENTS)
    {
        LogErr(LOG, "invalid component (%d)", type);
        return;
    }

    switch (type)
    {
        case COMPONENT_COLLIDER:     
        {
            const Collider* c = pSrc->GetComponent<Collider>();
            assert(c);
            assert(!pDst->HasComponent<Collider>());
            pDst->AddComponent<Collider>(c);
            break;
        }
        case COMPONENT_DAMAGE: 
        {
            const Damage* d = pSrc->GetComponent<Damage>();
            assert(d);
            assert(!pDst->HasComponent<Damage>());
            pDst->AddComponent<Damage>(d);
            break;
        }
        case COMPONENT_ENEMY:    
        {
            const Enemy* e = pSrc->GetComponent<Enemy>();
            assert(e);
            assert(!pDst->HasComponent<Enemy>());
            pDst->AddComponent<Enemy>(e);
            break;   
        }
        case COMPONENT_HEALTH:   
        {
            const Health* h = pSrc->GetComponent<Health>();
            assert(h);
            assert(!pDst->HasComponent<Health>());
            pDst->AddComponent<Health>(h);
            break;
        }
        case COMPONENT_HIERARCHY:    
        {
            const Hierarchy* hSrc = pSrc->GetComponent<Hierarchy>();
            assert(hSrc);

            assert(!pDst->HasComponent<Hierarchy>());
            Hierarchy& h = pDst->AddComponent<Hierarchy>(hSrc);

            // instantiate each child
            const EntityID* childrenIds = hSrc->GetChildren();
            const size_t    numChildren = hSrc->GetNumChildren();

            for (size_t i = 0; i < numChildren; ++i)
            {
                const EntityID childId = childrenIds[i];
                Entity* pChildSrc = nullptr;

                // we may clone a child of prefab OR entity, so handle it
                if (HasPrefab(childId))
                    pChildSrc = GetPrefab(childId);

                else if (HasEntt(childId))
                    pChildSrc = GetEntt(childId);


                Entity* pChild = Instantiate(pChildSrc);
                h.AddChild(pChild);
            }
            break;
        }
        case COMPONENT_KEYBOARD:     
        {
            break;
        }
        case COMPONENT_LIFE_TIMER:   
        {
            assert(!pDst->HasComponent<LifeTimer>());
            assert(pSrc->HasComponent<LifeTimer>());
            const LifeTimer* t = pSrc->GetComponent<LifeTimer>();
            pDst->AddComponent<LifeTimer>(t);
            break;
        }
        case COMPONENT_MOVEMENT:     
        {
            assert(0);
            break;
        }
        case COMPONENT_PLAYER:       
        {
            assert(0);
            break;
        }
        case COMPONENT_SPRITE:  
        {
            assert(!pDst->HasComponent<Sprite>());
            assert(pSrc->HasComponent<Sprite>());
            const Sprite* spr = pSrc->GetComponent<Sprite>();
            pDst->AddComponent<Sprite>(spr);
            break;
        }
        case COMPONENT_TILE:     
        {
            assert(!pDst->HasComponent<Tile>());
            assert(pSrc->HasComponent<Tile>());
            const Tile* t = pSrc->GetComponent<Tile>();
            pDst->AddComponent<Tile>(t);
            break;   
        }
        case COMPONENT_TRANSFORM:   
        {
            assert(!pDst->HasComponent<Transform>());
            assert(pSrc->HasComponent<Transform>());
            const Transform* t = pSrc->GetComponent<Transform>();
            Transform& dstTr = pDst->AddComponent<Transform>(t);
            break;       
        }
        case COMPONENT_TRIGGER:      
        {
            assert(!pDst->HasComponent<Trigger>());
            assert(pSrc->HasComponent<Trigger>());
            const Trigger* t = pSrc->GetComponent<Trigger>();
            pDst->AddComponent<Trigger>(t);
            break;
        }
        case COMPONENT_NAME:         
        {
            assert(0 && "TODO");
            break;
        }

        default:
            LogErr(LOG, "can't clone a component of type: %s (%d)", s_ComponentNames[type], type);
    }
}

//---------------------------------------------------------
// attach a component (in default state) to entity by ID
//---------------------------------------------------------
void EntityMgr::AddComponentToEntt(const EntityID id, const eComponentType type)
{
    assert(id != INVALID_ENTT_ID);
    Entity* pEntt = GetEntt(id);

    if (type < 0 && type >= NUM_COMPONENTS)
    {
        LogErr(LOG, "invalid component (%d)", type);
        return;
    }

    if (pEntt->HasComponent(type))
    {
        LogErr(LOG, "entity '%s' (id: %d) already has a component '%s' (%d)", pEntt->GetName(), id, s_ComponentNames[type], type);
        return;
    }

    switch (type)
    {
        case COMPONENT_COLLIDER:     pEntt->AddComponent<Collider>();   break;
            //case COMPONENT_DAMAGE:       pEntt->AddComponent<Damage>();     break; 
            //case COMPONENT_ENEMY:        pEntt->AddComponent<Enemy>();      break; 
            //case COMPONENT_HEALTH:       pEntt->AddComponent<Health>();     break; 
            //case COMPONENT_HIERARCHY:    pEntt->AddComponent<Hierarchy>();  break;     
            //case COMPONENT_KEYBOARD:     pEntt->AddComponent<Keyboard>();   break;     
            //case COMPONENT_LIFE_TIMER:   pEntt->AddComponent<LifeTimer>();  break;     
        case COMPONENT_MOVEMENT:     pEntt->AddComponent<Movement>();   break;     
            //case COMPONENT_PLAYER:       pEntt->AddComponent<Player>();     break; 
        case COMPONENT_SPRITE:       pEntt->AddComponent<Sprite>();     break; 
            //case COMPONENT_TRANSFORM:    pEntt->AddComponent<Transform>();  break;     
            //case COMPONENT_TRIGGER:      pEntt->AddComponent<Trigger>();    break;     

        default:
            LogErr(LOG, "unknown component (%d) to attach to entt '%s' (id: %d)", type, pEntt->GetName(), id);
    }
}

//---------------------------------------------------------
// Desc:  add a new entity with enttName
//---------------------------------------------------------
Entity* EntityMgr::AddEntt(const char* enttName, const eLayerType layer)
{
    if (IsEmptyStr(enttName))
    {
        LogErr(LOG, ERR_EMPTY_NAME);
        return entities_[0];
    }
    if (!IsValidLayer(layer))
    {
        LogErr(LOG, "invalid layer (layer code: %d)", (int)layer);
        return entities_[0];
    }
    if (HasEntt(enttName))
    {
        return GetEntt(enttName);
    }

    // create a new entity
    Entity* pEntt = new Entity(s_EnttId, enttName, layer);
    ++s_EnttId;
    ids_.push_back(pEntt->GetId());

    // push it into common arr
    entities_.push_back(pEntt);

    // attach it to particular rendering layer
    entitiesByLayer_[layer].push_back(pEntt);

    return pEntt;
}

//---------------------------------------------------------
// set input entt to remove (later during Update we will remove them all)
//---------------------------------------------------------
void EntityMgr::SetToRemove(const EntityID id)
{
    enttsToRemove_.push_back(GetEntt(id));
}

void EntityMgr::SetToRemove(Entity* pEntt)
{
    assert(pEntt);
    enttsToRemove_.push_back(pEntt);
}

//---------------------------------------------------------
// Desc:  return an array of all the currently created prefabs
//---------------------------------------------------------
const cvector<Entity*>& EntityMgr::GetPrefabs() const
{
    return prefabs_;
}

//---------------------------------------------------------
// Desc:  return an arr of all the entities
//---------------------------------------------------------
const cvector<Entity*>& EntityMgr::GetEntts() const
{
    return entities_;
}

//---------------------------------------------------------
// Desc:  get all the entities which belong to input layer
//---------------------------------------------------------
const cvector<Entity*>& EntityMgr::GetEnttsByLayer(const eLayerType layer) const
{
    assert(IsValidLayer(layer));
    return entitiesByLayer_[layer];
}

//---------------------------------------------------------
// debug: dump all the entities and its components into a console
//---------------------------------------------------------
void EntityMgr::ListAllEntts() const
{
    printf("Print a list of all entities:\n");

    for (uint i = 0; Entity* pEntt : entities_)
    {
        printf("\t[%u]: (id: %d,   name: %s)\n", i++, (int)pEntt->GetId(), pEntt->GetName());
        pEntt->ListAllComponents();
    }
    printf("\n");
}

//---------------------------------------------------------
// debug:  dump the entities related to input layer
//---------------------------------------------------------
void EntityMgr::ListAllEnttsByLayer(const eLayerType layer) const
{
    assert(layer >= 0 && layer < NUM_LAYERS);

    printf("DUMP a list of entities related to layer '%s':\n", s_LayerNames[layer]);

    for (uint i = 0; const Entity* pEntt : entitiesByLayer_[layer])
    {
        printf("\t[%u]: (id: %d,   name: %s)\n", i++, (int)pEntt->GetId(), pEntt->GetName());
    }
    printf("\n");
}

//---------------------------------------------------------
// get a name of a layer by input layer code
//---------------------------------------------------------
const char* EntityMgr::GetLayerName(const eLayerType layer)
{
    if (layer >= 0 && layer < NUM_LAYERS)
        return s_LayerNames[layer];
    else
        return "invalid_layer";
}

//---------------------------------------------------------
// get a ptr to entity by input id
//---------------------------------------------------------
Entity* EntityMgr::GetEntt(const EntityID id)
{
    const index idx = ids_.get_idx(id);

    // invalid index
    if (idx <= 0 && idx >= ids_.size())
    {
        LogErr(LOG, "no entt by id: %d", (int)id);
        return entities_[0];
    }

    // by valid index placed wrong entity 
    if (entities_[idx]->GetId() != id)
    {
        LogErr(LOG, "no entt by id: %d", (int)id);
        return entities_[0];
    }

    return entities_[idx];
}

//---------------------------------------------------------
// get a ptr to entity by input name
//---------------------------------------------------------
Entity* EntityMgr::GetEntt(const char* enttName)
{
    assert(!IsEmptyStr(enttName));

    for (Entity* pEntt : entities_)
    {
        if (strcmp(pEntt->GetName(), enttName) == 0)
            return pEntt;
    }

    // return a ptr to the "invalid/dummy" entity
    LogErr(LOG, "no entt by name: %s", enttName);
    return entities_[0];
}

//---------------------------------------------------------
// check if input entity name is unique (so we can use it for creation of a new entity)
//---------------------------------------------------------
bool EntityMgr::IsUniqueEnttName(const char* name) const
{
    for (const Entity* pEntt : entities_)
    {
        if (strcmp(pEntt->GetName(), name) == 0)
            return false;
    }
    return true;
}

//---------------------------------------------------------
// is layer enabled for rendering?
//---------------------------------------------------------
bool EntityMgr::IsShowLayer(const eLayerType layer) const
{
    return bShowLayers_[layer];
}

//---------------------------------------------------------
// enable/disable rendering of the layer
//---------------------------------------------------------
void EntityMgr::SetShowLayer(const eLayerType layer, const bool showHide)
{
    assert(layer >= 0 && layer < NUM_LAYERS);
    bShowLayers_[layer] = showHide;
}