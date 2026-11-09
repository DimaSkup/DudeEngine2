/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: entity_mgr.h
    Created:  by DimaSkup
\**********************************************************************************/
#ifndef ENTITY_MGR_H
#define ENTITY_MGR_H

#include "types.h"
#include "entity.h"
#include "components/component.h"
#include "cvector.h"

//---------------------------------------------------------
// an entitities manager for Entity-Component
//---------------------------------------------------------
class EntityMgr
{
public:
    EntityMgr();

    void CreateDummyStuff();
    void ClearData();


    // debug
    void        ListAllEntts() const;
    void        ListAllEnttsByLayer(const eLayerType layer) const;


    const char* GetLayerName(const eLayerType layer);
    eLayerType  GetLayerType(const char* layerName);

    // remove on the next Update()
    void        SetToRemove(const EntityID id);
    void        SetToRemove(Entity* pEntt);

    // immediate remove
    void        RemoveEntts(const EntityID* ids, const vsize count);
    void        RemoveEntts(Entity** entts, const vsize count);

    uint        GetNumEntts() const;
    EntityID    GetPlayerId() const;
    Entity*     GetPlayer();

    void        SaveEntts(const char* filePath);
    void        LoadEntts(const char* filePath);

    void        SaveUiEntts(const char* filepath);

    void        Update(const float dt);
    void        SetShowLayer(const eLayerType layer, const bool showHide);
    bool        IsShowLayer(const eLayerType layer) const;

    // entts
    void        AddComponentToEntt(const EntityID id, const eComponentType type);

    Entity*     AddEntt(const char* enttName, const eLayerType layer);
    Entity*     GetEntt(const EntityID id);
    Entity*     GetEntt(const char* enttName);

    const cvector<Entity*>& GetEntts() const;
    const cvector<Entity*>& GetEnttsByLayer(const eLayerType layer) const;

    bool        HasEntts() const;
    EntityID    HasEntt(const char* enttName) const;
    bool        HasEntt(const EntityID id) const;

    // prefabs
    Entity*     AddPrefab(const Entity* pEntt);
    Entity*     AddPrefab(const char* prefabName, const eLayerType layer);
    bool        HasPrefab(const EntityID prefabId) const;
    EntityID    HasPrefab(const char* prefabName) const;
    Entity*     GetPrefab(const char* prefabName);
    Entity*     GetPrefab(const EntityID id);
    Entity*     Instantiate(const Entity* prefab);
    const cvector<Entity*>& GetPrefabs() const;

    // queries
    template <typename T>
    void GetEnttsWithComponent(cvector<Entity*>& outEntts);

private:
    void CloneComponent  (Entity* pDst, const Entity* pSrc, const eComponentType compType);
    bool IsUniqueEnttName(const char* name) const;

    // helpers: remove entt
    void RemoveEnttFromMainLists(const EntityID id);
    void RemoveEnttFromLayer(const Entity* pEntt, const eLayerType layer);

private:
    EntityID            playerId_;

    cvector<EntityID>   ids_;                           // each entity has its own unique ID (can be different between game sessions)
    cvector<Entity*>    entities_;
    cvector<Entity*>    entitiesByLayer_[NUM_LAYERS];   // entities are grouped by its layers
    bool                bShowLayers_[NUM_LAYERS];       // enable/disable rendering of particular layer

    cvector<Entity*>    prefabs_;                       // prefabs are used as "templates" for entities instantiation
    cvector<PrefabName> prefabsNames_;

    cvector<Entity*>    enttsToRemove_;                 // array of entities which will be removed on the next Update()
};

//---------------------------------------------------------
// global instance
//---------------------------------------------------------
extern EntityMgr g_EnttMgr;

//---------------------------------------------------------
// get all entities which has a component (component type we get from the template parameter)
//---------------------------------------------------------
template <typename T>
inline void EntityMgr::GetEnttsWithComponent(cvector<Entity*>& outEntts)
{
    outEntts.reserve(32);

    for (Entity* pEntt : entitiesByLayer_[LAYER_ENTITY])
    {
        if (!pEntt->HasComponent<T>())
            continue;

        outEntts.push_back(pEntt);
    }
}

//---------------------------------------------------------
// return an ID of the main player
//---------------------------------------------------------
inline EntityID EntityMgr::GetPlayerId() const
{
    assert(playerId_ != INVALID_ENTT_ID);
    return playerId_;
}

//---------------------------------------------------------
// get a ptr to the main player
//---------------------------------------------------------
inline Entity* EntityMgr::GetPlayer(void)
{
    assert(playerId_ != INVALID_ENTT_ID);
    return GetEntt(playerId_);
}

#endif