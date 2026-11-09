/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: entity.cpp
    Created:  by DimaSkup
\**********************************************************************************/
#include "entity.h"
#include "mem_helpers.h"
#include <assert.h>


//---------------------------------------------------------
// constructors
//---------------------------------------------------------
Entity::Entity(EntityID id) :
    id_(id),
    layer_(eLayerType(0)),
    state_(eEnttState(0)),
    bActive_(true),
    addedComponents_(0)
{
    strcpy(name_, "invalid");
}

//---------------------------------------------------------

Entity::Entity(EntityID id, const char* enttName, eLayerType layer) :
    id_(id),
    layer_(layer),
    state_(eEnttState(0)),
    bActive_(true),
    addedComponents_(0)
{
    assert(enttName && enttName[0] != '\0');
    assert(strlen(enttName) < (MAX_LEN_ENTT_NAME - 1) && "entity name is too long");

    strcpy(name_, enttName);
}

//---------------------------------------------------------
// destructor
//---------------------------------------------------------
Entity::~Entity()
{
    Shutdown();
}

//---------------------------------------------------------
// Desc:  update this entity and its components
// Args:  dt - delta time
//---------------------------------------------------------
void Entity::Update(const float dt)
{
    for (Component* pComp : components_)
        pComp->Update(dt);
}

//---------------------------------------------------------
// Desc:  go through each component of this entity and call its 
//        methods which are responsible for rendering
//---------------------------------------------------------
void Entity::Render()
{
    for (Component* pComp : components_)
        pComp->Render();
}

//---------------------------------------------------------
// Desc:  destroy this entity
//---------------------------------------------------------
void Entity::Shutdown()
{
    // release components
    for (Component* pComp : components_)
        SafeDelete(pComp);

    components_.clear();
    componentTypeMap_.clear();

    name_[0] = '\0';
    layer_ = NUM_LAYERS;
    bActive_ = false;
    addedComponents_ = 0;
}

//---------------------------------------------------------
// print into console a list of components of this entity
//---------------------------------------------------------
void Entity::ListAllComponents() const
{
    for (auto& elem : componentTypeMap_)
    {
        printf("\t  Component<%s>\n", elem.first->name());
    }
}