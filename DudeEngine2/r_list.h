/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   r_list.h
    Desc:       a rendering list
    Created:    by DimaSkup
\**********************************************************************************/
#ifndef R_LIST_H
#define R_LIST_H

#include "types.h"
#include "cvector.h"
#include "rect.h"


//---------------------------------------------------------
// typedefs
//---------------------------------------------------------
struct RenderItem
{
    TexID         texId;
    Recti         srcRect;
    Recti         dstRect;
    Color         color;
    eRendererFlip flipMode;
    int8          zIndex;     // is used if related entt belongs to 
                              // LAYER_GND_DECAL or LAYER_UI,
                              // so we will sort render items from min => max 
                              // (bottom/farther => top/nearest)
                                
};

//---------------------------------------------------------

struct RenderItemsArr
{
    cvector<RenderItem> items;
};

//---------------------------------------------------------
// globals:
//---------------------------------------------------------

// each layer has its own render list
extern RenderItemsArr g_RenderLists[NUM_LAYERS];


//---------------------------------------------------------
// functions: 
//---------------------------------------------------------
void ClearRenderLists();
void SortRenderListItemsByY(const eLayerType layer);
void SortRenderListItemsByZindex(const eLayerType layer);

void PushItemToRender(
    const eLayerType layer,
    const TexID texId,
    const Recti& srcRect,
    const Recti& dstRect,
    const Color color,
    const eRendererFlip flipMode,
    const int8 zIndex);

#endif