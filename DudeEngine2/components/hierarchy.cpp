/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: hierarchy.cpp
    Desc:     a component to hold entities hierarchy data:
              entity can have only one "parent" and have multiple "children"

    Created:  19.09.2026 by DimaSkup
\**********************************************************************************/
#include "hierarchy.h"

#include "../log.h"
#include "../macros.h"
#include "../entity.h"
#include "../entity_mgr.h"
#include "transform.h"


//---------------------------------------------------------
// a constructor for cloning the component
// (hierarchy relations we have to setup manually in the caller of this constructor)
//---------------------------------------------------------
Hierarchy::Hierarchy(const Hierarchy* h) :
    parentId_(0),
    relativePos_(0.0f, 0.0f)
{
    assert(h);

    relativePos_ = h->relativePos_;
}

//---------------------------------------------------------
// constructor
// NOTE: if parentId == INVALID_ENTT_ID (0) then this entity has no parent
//---------------------------------------------------------
Hierarchy::Hierarchy() :
    parentId_(0),
    relativePos_(0.0f, 0.0f)
{
    for (size_t i = 0; i < MAX_CHILD_ENTTS; ++i)
        childrenIds_[i] = INVALID_ENTT_ID;
}

//---------------------------------------------------------
// update position of this entity relatively to its parent
//---------------------------------------------------------
void Hierarchy::Update(const float dt)
{
    // this entity has no parent
    if (parentId_ == INVALID_ENTT_ID)
        return;

    const Entity*    pParent = g_EnttMgr.GetEntt(parentId_);
    const Transform* parentT = pParent->GetComponent<Transform>();
    const Vec2     parentPos = parentT->GetPos();

    Transform* t = pOwner_->GetComponent<Transform>();
    t->SetPos(parentPos.x + relativePos_.x, parentPos.y + relativePos_.y);
}

//---------------------------------------------------------
// add a new child (by input pointer) for this entity 
//---------------------------------------------------------
void Hierarchy::AddChild(Entity* pChild)
{
    assert(pChild);

    const EntityID id = pChild->GetId();

    if (GetChildIdx(id) != -1)
    {
        LogErr(LOG, "entity (%d) is already a child of entity by id: %d", id, pOwner_->GetId());
        return;
    }

    // find a free place for a new child
    for (size_t i = 0; i < MAX_CHILD_ENTTS; ++i)
    {
        if (childrenIds_[i] == INVALID_ENTT_ID)
        {
            childrenIds_[i] = id;

            // set a new parent for input child
            Hierarchy* h = pChild->GetComponent<Hierarchy>();
            assert(h && "child has no hierarchy component!");

            const EntityID parentId = pOwner_->GetId();
            h->SetParent(parentId);

            return;
        }
    }

    LogErr(LOG, "can't add one more child (%d) for entity (%d)", id, pOwner_->GetId());
}

//---------------------------------------------------------
// add a new child (by input ID) for this entity
//---------------------------------------------------------
void Hierarchy::AddChild(const EntityID id)
{
    assert(id != INVALID_ENTT_ID);

    if (GetChildIdx(id) != -1)
    {
        LogErr(LOG, "entity (%d) is already a child of entity '%s'", id, pOwner_->GetName());
        return;
    }

    // find a free place for a new child
    for (size_t i = 0; i < MAX_CHILD_ENTTS; ++i)
    {
        if (childrenIds_[i] == INVALID_ENTT_ID)
        {
            childrenIds_[i] = id;

            // set a new parent for input child
            Entity* pChild = g_EnttMgr.GetEntt(id);
            Hierarchy* h = pChild->GetComponent<Hierarchy>();
            assert(h && "child has no hierarchy component!");

            const EntityID parentId = pOwner_->GetId();
            h->SetParent(parentId);

            return;
        }
    }

    LogErr(LOG, "can't add one more child (%d) for entity (%d)", id, pOwner_->GetId());
}

//---------------------------------------------------------
// detach a child by ID from this entity
//---------------------------------------------------------
void Hierarchy::RemoveChild(const EntityID id)
{
    const size_t idx = GetChildIdx(id);

    if (idx == -1)
    {
        LogErr(LOG, "entity (%d) has no child with id: %d", pOwner_->GetId(), id);
        return;
    }

    // remove parent
    Entity* pChild = g_EnttMgr.GetEntt(id);
    Hierarchy* pChildHierarchy = pChild->GetComponent<Hierarchy>();

    assert(pChildHierarchy && "child has no hierarchy component");
    pChildHierarchy->SetParent(INVALID_ENTT_ID);

    // swap & pop this child
    const size_t last = GetNumChildren();
    childrenIds_[idx] = childrenIds_[last];
    childrenIds_[last] = INVALID_ENTT_ID;
}

//---------------------------------------------------------
// return a ptr to array of children
//---------------------------------------------------------
const EntityID* Hierarchy::GetChildren(void) const
{
    return childrenIds_;
}

//---------------------------------------------------------
// return a current number of children
//---------------------------------------------------------
const size_t Hierarchy::GetNumChildren(void) const
{
    size_t num = 0;

    for (size_t i = 0; i < MAX_CHILD_ENTTS; ++i)
    {
        if (childrenIds_[i] != INVALID_ENTT_ID)
            num++;
    }

    return num;
}

//---------------------------------------------------------
// return an ID of current parent of this entity;
// NOTE: if parentId == INVALID_ENTT_ID (0) it means that no parent is set;
//---------------------------------------------------------
EntityID Hierarchy::GetParent(void) const
{
    return parentId_;
}

//---------------------------------------------------------
// if input ID != INVALID_ENTT_ID (0) then we setup a new parent;
// in another case we detach any parent and set relative position to <0,0>
//---------------------------------------------------------
void Hierarchy::SetParent(const EntityID id)
{
    if (parentId_ == id)
        return;

    // detach from prev parent
    if (parentId_ != INVALID_ENTT_ID)
    {
        Entity* pPrevParent = g_EnttMgr.GetEntt(parentId_);
        Hierarchy* pHierarchy = pPrevParent->GetComponent<Hierarchy>();

        assert(pHierarchy && "previous parent hasn't a Hierarchy component (something is sus)");
        pHierarchy->RemoveChild(pOwner_->GetId());
    }

    Entity* pParent = nullptr;

    if (g_EnttMgr.HasEntt(id))
        pParent = g_EnttMgr.GetEntt(id);

    // maybe we tried to get a prefab entity
    else
        pParent = g_EnttMgr.GetPrefab(id);

    assert(pParent);
    assert(pParent->HasComponent<Hierarchy>() && "you have to add a Hierarchy component to entity before set it as a parent");
    parentId_ = id;

    // if no parent
    if (id == INVALID_ENTT_ID)
        relativePos_ = { 0,0 };
}

//---------------------------------------------------------
// setup a position of this entity relatively to its parent
//---------------------------------------------------------
void Hierarchy::SetRelativePos(const float px, const float py)
{
    relativePos_ = { px, py };
}

void Hierarchy::SetRelativePos(const Vec2 p)
{
    relativePos_ = { p.x, p.y };
}

//---------------------------------------------------------
// return a position of this entity relatively to its parent
//---------------------------------------------------------
Vec2 Hierarchy::GetRelativePos(void) const
{
    return relativePos_;
}

//---------------------------------------------------------
// return index of a child by input id;
// return -1 if there is no such child;
//---------------------------------------------------------
size_t Hierarchy::GetChildIdx(const EntityID id) const
{
    for (int i = 0; i < MAX_CHILD_ENTTS; ++i)
    {
        if (childrenIds_[i] == id)
            return i;
    }

    return -1;
}