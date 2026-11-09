/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: component.h
    Desc:     basic class for all the components (Entity-Component)

    Created:  by DimaSkup
\**********************************************************************************/
#ifndef COMPONENT_H
#define COMPONENT_H

//---------------------------------------------------------
// forward declarations (pointer use only)
//---------------------------------------------------------
class Entity;

//---------------------------------------------------------

enum eComponentType
{
    COMPONENT_COLLIDER = 0,
    COMPONENT_DAMAGE,
    COMPONENT_ENEMY,
    COMPONENT_HEALTH,
    COMPONENT_HIERARCHY,
    COMPONENT_KEYBOARD,
    COMPONENT_LIFE_TIMER,
    COMPONENT_MOVEMENT,
    COMPONENT_PLAYER,
    COMPONENT_SPRITE,
    COMPONENT_TILE,
    COMPONENT_TRANSFORM,
    COMPONENT_TRIGGER,
    COMPONENT_NAME,

    NUM_COMPONENTS,
};

static const char* s_ComponentNames[] = {
    "Collider",
    "Damage",
    "Enemy",
    "Health",
    "Hierarchy",
    "Keyboard",
    "LifeTimer",
    "Movement",
    "Player",
    "Sprite",
    "Tile",
    "Transform",
    "Trigger",
    "Name"
};

//---------------------------------------------------------
// base class for all the component (Entity-Component)
//---------------------------------------------------------
class Component
{
public:
	virtual ~Component() {}

	virtual void Init() {}
	virtual void Update(const float dt) {}
	virtual void Render() {}

	virtual const char*    GetComponentName() const = 0;
    virtual eComponentType GetComponentType() const = 0;

public:
	Entity* pOwner_ = nullptr;
};

#endif