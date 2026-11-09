/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: name.h
    Desc:     a component to assign some specific name for entity
    NOTE:     a name is not necessarily unique
              (for instance: when we name an enemy using its type)
    Created:  17.09.2026 by DimaSkup
\**********************************************************************************/
#ifndef NAME_COMPONENT_H
#define NAME_COMPONENT_H

#include "../consts.h"
#include "component.h"

#include <assert.h>
#include <string.h>


class Name : public Component
{
private:
    char keyToName_[MAX_LEN_ENTT_NAME];   // actually we store a key to localized name of this entity

public:
    Name(const char* keyToName)
    {
        SetName(keyToName);
    }

    virtual void Init() {}
    virtual void Update(const float dt) {}
    virtual void Render() {}

    virtual const char* GetComponentName() const 
    { 
        return s_ComponentNames[COMPONENT_NAME]; 
    }

    virtual eComponentType GetComponentType() const
    {
        return COMPONENT_NAME;
    }


    void SetName(const char* keyToName)
    {
        assert(keyToName && keyToName[0] != '\0');
        assert(strlen(keyToName) < MAX_LEN_ENTT_NAME);
        memset(keyToName_, 0, sizeof(keyToName_));
        strcpy(keyToName_, keyToName);
    }

    const char* GetName(void) const 
    { 
        return keyToName_; 
    }
};

#endif // NAME_COMPONENT_H