/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   r_list.cpp
    Desc:       a rendering list
    Created:    by DimaSkup
\**********************************************************************************/
#include "r_list.h"

// globals
RenderItemsArr g_RenderLists[NUM_LAYERS];

//---------------------------------------------------------
// clear the render lists before the next frame
//---------------------------------------------------------
void ClearRenderLists()
{
    for (int i = 0; i < NUM_LAYERS; ++i)
        g_RenderLists[i].items.clear();
}

//---------------------------------------------------------
// push a new render item for rendering
//---------------------------------------------------------
void PushItemToRender(
    const eLayerType layer,
    const TexID texId,
    const Recti& srcRect,
    const Recti& dstRect,
    const Color color,
    const eRendererFlip flipMode,
    const int8 zIndex)
{
    assert(layer >= 0 && layer < NUM_LAYERS);
    g_RenderLists[layer].items.push_back(RenderItem(texId, 
                                                    srcRect, 
                                                    dstRect, 
                                                    color, 
                                                    flipMode, 
                                                    zIndex));
}

//---------------------------------------------------------
// (is used for LAYER_GND_DECAL and LAYER_UI)
// sort render items by z-index from min to max values,
// which means that items with lower value will be rendered first (most bottom)
//---------------------------------------------------------
static int CmpZindex(const void* a, const void* b)
{
    const RenderItem* arg1 = (RenderItem*)(a);
    const RenderItem* arg2 = (RenderItem*)(b);

    // smaller z-index -> higher z-index
    const int lowY1 = arg1->zIndex;
    const int lowY2 = arg2->zIndex;

    if (lowY1 < lowY2) return -1;
    if (lowY1 > lowY2) return 1;
    return 0;
}

void SortRenderListItemsByZindex(const eLayerType layer)
{
    assert(layer >= 0 && layer < NUM_LAYERS);

    RenderItem* items = g_RenderLists[layer].items.data();
    size_t   numItems = g_RenderLists[layer].items.size();

    std::qsort(items, numItems, sizeof(RenderItem), CmpZindex);
}

//---------------------------------------------------------
// sort render items by dst LOWER Y (!!!) value, from small to high values,
// so it helps us to make illusion of depth, since objects which "higher"
// on the screen will be rendered first (simply it is the Painter's Algorithm: 
// paint furthest objects first, and the closes objects last)
//---------------------------------------------------------
static int CmpLowY(const void* a, const void* b)
{
    const RenderItem* arg1 = (RenderItem*)(a);
    const RenderItem* arg2 = (RenderItem*)(b);

    // small_Y -> high_Y
    const int lowY1 = arg1->dstRect.y1;
    const int lowY2 = arg2->dstRect.y1;

    if (lowY1 < lowY2) return -1;
    if (lowY1 > lowY2) return 1;
    return 0;
}

void SortRenderListItemsByY(const eLayerType layer)
{
    assert(layer >= 0 && layer < NUM_LAYERS);

    RenderItem* items = g_RenderLists[layer].items.data();
    size_t   numItems = g_RenderLists[layer].items.size();

    std::qsort(items, numItems, sizeof(RenderItem), CmpLowY);
}