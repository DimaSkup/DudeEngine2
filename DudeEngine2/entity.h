/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: entity.h
    Created:  by DimaSkup
\**********************************************************************************/
#ifndef ENTITY_H
#define ENTITY_H

#include "types.h"
#include "log.h"
#include "components/component.h"

#include "cvector.h"
#include <map>
#include <typeinfo>


//---------------------------------------------------------
// aliases
//---------------------------------------------------------
using ComponentsMap = std::map<const std::type_info*, Component*>;

//---------------------------------------------------------
// enumeration of entity states
//---------------------------------------------------------
enum eEnttState : uint8
{
    ENTT_IDLE,      // default state 
    ENTT_MOVE,      // walk, run, etc.
    ENTT_ATTACK,
    ENTT_DEATH,     // dies
    ENTT_DAMAGE,    // takes damage
    ENTT_BLOCK,     // block income attacks

    NUM_ENTT_STATES
};

//---------------------------------------------------------
// entity for Entity-Component
//---------------------------------------------------------
class Entity
{
public:
    Entity(const EntityID id);
    Entity(const EntityID id, const char* enttName, const eLayerType layer);
    ~Entity();

    void        Update(const float dt);
    void        Render();
    void        Shutdown();

    void        SetName(const char* name);

    bool        IsActive() const;
    EntityID    GetId() const;
    const char* GetName() const;
    eLayerType  GetLayer() const;

    eEnttState  GetState(void) const;
    void        SetState(eEnttState s);

    //---------------------------------------------------------
    // Desc:      add a new component to this entity 
    // Template:  T     - type of the component
    //            TArgs - arguments to init the component with
    //---------------------------------------------------------
    template <typename T, typename... TArgs>
    T& AddComponent(TArgs&&... args)
    {
        T* pComponent = new T(std::forward<TArgs>(args)...);

        pComponent->pOwner_ = this;
        components_.push_back(pComponent);
        componentTypeMap_[&typeid(*pComponent)] = pComponent;
        pComponent->Init();

        const eComponentType type = pComponent->GetComponentType();
        addedComponents_ |= (1 << type);

        return *pComponent;
    }

    //---------------------------------------------------------
    // Desc:         get a ptr to component by type 
    // Call example: GetComponent<Transform>()
    //---------------------------------------------------------
    template <typename T>
    T* GetComponent() 
    {
        auto it = componentTypeMap_.find(&typeid(T));

        if (it == componentTypeMap_.end())
            return nullptr;

        return (T*)it->second;
    }

    template <typename T>
    const T* GetComponent() const
    {
        auto it = componentTypeMap_.find(&typeid(T));
        
        if (it == componentTypeMap_.end())
            return nullptr;

        return (T*)it->second;
    }

    //---------------------------------------------------------
    // Desc:          remove a component from entity
    // NOTE:          if it hasn't such component we do nothing
    // Call example:  RemoveComponent<Transform>()
    //---------------------------------------------------------
    template <typename T>
    void RemoveComponent()
    {
        T* c = GetComponent<T>();

        // there is no such a component
        if (c == nullptr)
            return;

        c->pOwner_ = nullptr;
        componentTypeMap_.erase(&typeid(*c));

        for (index i = 0; i < components_.size(); ++i)
        {
            if (components_[i] == c)
            {
                // swap & pop
                components_[i] = components_.back();
                components_.pop_back();
                break;  // interrupt the for-loop
            }
        }

        const eComponentType type = c->GetComponentType();
        addedComponents_ &= ~(1 << type);

        delete c;
        c = nullptr;

        assert(!HasComponent<T>());
    }
    
    //---------------------------------------------------------
    // Desc:          check if this entity has a particular component
    // Call example:  HasComponent<Transform>()
    //---------------------------------------------------------
    template <typename T>
    bool HasComponent() const {
        return (componentTypeMap_.find(&typeid(T)) != componentTypeMap_.end());
    }

    bool HasComponent(const eComponentType comp) const
    {
        return addedComponents_ & (1 << comp);
    }
    
    //---------------------------------------------------------
    // debug
    //---------------------------------------------------------
    void                 ListAllComponents() const;
    const ComponentsMap& GetComponentsMap() const;

private:
    const EntityID      id_;
    cvector<Component*> components_;
    ComponentsMap       componentTypeMap_;

    char                name_[MAX_LEN_ENTT_NAME];
    eLayerType          layer_;
    eEnttState          state_;

    // bitflags where each flag represents related component, if set to 1 then 
    // this entt has a component (look at enumeration 'eComponentType')
    uint32              addedComponents_;

    bool                bActive_;
};

//=============================================================================
// INLINE FUNCTIONS
//=============================================================================
inline bool Entity::IsActive() const
{
    return bActive_;
}

inline EntityID Entity::GetId() const
{
    return id_;
}

inline const char* Entity::GetName() const
{
    return name_;
}

inline void Entity::SetName(const char* name)
{
    if (!name || name[0] == '\0')
        return;

    strncpy(name_, name, MAX_LEN_ENTT_NAME);
    name_[MAX_LEN_ENTT_NAME - 1] = '\0';
}

inline eLayerType Entity::GetLayer() const
{
    return layer_;
}

inline const ComponentsMap& Entity::GetComponentsMap() const
{
    return componentTypeMap_;
}

//---------------------------------------------------------
// get/set state of entity
//---------------------------------------------------------
inline eEnttState Entity::GetState() const 
{ 
    return state_; 
}

inline void Entity::SetState(eEnttState s) 
{ 
    assert(s >= 0 && s < NUM_ENTT_STATES);
    state_ = s; 
}

#endif
