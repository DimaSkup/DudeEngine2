#pragma once

#include "types.h"
#include "cvector.h"


// forward declaration (pointer use only)
class Entity;

class DbgRender
{
public:
    void Render(
        const cvector<Entity*> sceneObjects,
        const cvector<EntityID>& visibleTriggers,
        const cvector<EntityID>& visibleEnemies);

private:
    void RenderDbgInfo();
    void RenderScreenGrid();
    void RenderTriggers(const cvector<EntityID>& enttsIds);
    void RenderColliders(const cvector<Entity*>& entts);
    void RenderAttackRanges(const cvector<EntityID>& enemies);
};
