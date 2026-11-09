/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: hierarchy.h
    Desc:     a component to hold entities hierarchy data:
              entity can have only one "parent" and have multiple "children"

    Created:  19.09.2026 by DimaSkup
\**********************************************************************************/
#ifndef HIERARCHY_COMPONENT_H
#define HIERARCHY_COMPONENT_H

#include "../types.h"
#include "../vec.h"
#include "component.h"


class Hierarchy : public Component
{
public:

    // for cloning the component
    Hierarchy(const Hierarchy* h);

    Hierarchy();

    //-----------------------------------------------------

    virtual void Init() override {};
    virtual void Render() override {};
    virtual void Update(const float dt) override;
    
    virtual const char* GetComponentName() const override
    {
        return s_ComponentNames[COMPONENT_HIERARCHY];
    }

    virtual eComponentType GetComponentType() const override
    {
        return COMPONENT_HIERARCHY;
    }

    //-----------------------------------------------------

    void            AddChild(const EntityID id);
    void            AddChild(Entity* pEntt);
    void            RemoveChild(const EntityID id);
    const EntityID* GetChildren(void) const;
    const size_t    GetNumChildren(void) const;

    EntityID        GetParent(void) const;
    void            SetParent(const EntityID id);

    void            SetRelativePos(const float px, const float py);
    void            SetRelativePos(const Vec2 p);
    Vec2            GetRelativePos(void) const;

private:
    size_t          GetChildIdx(const EntityID id) const;

private:
    EntityID parentId_;
    EntityID childrenIds_[MAX_CHILD_ENTTS];

    // position relatively to the parent
    Vec2 relativePos_;
};


#endif // HIERARCHY_COMPONENT_H
