/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: tile.h

    Created:  by DimaSkup
\**********************************************************************************/
#ifndef TILE_COMPONENT_H
#define TILE_COMPONENT_H

#include "../types.h"
#include "../vec.h"
#include "../r_list.h"
#include "../game_state.h"
#include "../rect.h"
#include "component.h"
#include <assert.h>

class Tile : public Component
{
public:
    Tile(const Tile* t)
    {
        assert(t);

        texId_   = t->texId_;
        srcRect_ = t->srcRect_;
        dstRect_ = t->dstRect_;
        color_   = t->color_;
        rect_    = rect_;
        pos_     = pos_;
        size_    = size_;
    }

    //-----------------------------------------------------

    Tile(
        const int srcX, 
        const int srcY, 
        const int dstX, 
        const int dstY,
        const int tileSz,   // size of a tile on the texture atlas
        const int cellSz,   // size of a target rectangle to draw a tile into
        const TexID texId)
    {
        assert(srcX >= 0);
        assert(srcY >= 0);
        assert(dstX >= 0);
        assert(dstY >= 0);
        assert(tileSz > 0);
        assert(cellSz > 0);

        texId_ = texId;

        srcRect_.x0 = srcX;
        srcRect_.x1 = srcX + tileSz;
        srcRect_.y0 = srcY;
        srcRect_.y1 = srcY + tileSz;

        dstRect_.x0 = dstX;
        dstRect_.x1 = dstX + cellSz;
        dstRect_.y0 = dstY;
        dstRect_.y1 = dstY + cellSz;

        SetDstRect(dstRect_);

        SetColor(255, 255, 255, 255);
    }

    //-----------------------------------------------------

    virtual const char* GetComponentName() const override 
    { 
        return s_ComponentNames[COMPONENT_TILE];
    }

    virtual eComponentType GetComponentType() const override
    {
        return COMPONENT_TILE;
    }

    //-----------------------------------------------------

    virtual void Init() override {};

    //-----------------------------------------------------

    virtual void Update(const float dt) override
    {
        // change position of the tile on screen according 
        // to current position of the main camera
        dstRect_.x0 = pos_.x - g_GameState.camPos.x;
        dstRect_.x1 = dstRect_.x0 + size_.w;
        dstRect_.y0 = pos_.y - g_GameState.camPos.y;
        dstRect_.y1 = dstRect_.y0 + size_.h;
    }

    //-----------------------------------------------------

    virtual void Render() override
    {
        constexpr eRendererFlip flip = R_FLIP_NONE;
        constexpr int8 zIndex = 0;
        PushItemToRender(LAYER_TILEMAP, texId_, srcRect_, dstRect_, color_, flip, zIndex);
    }

    //-----------------------------------------------------

    void         SetSrcRect(const Recti& r)       { srcRect_ = r; }
    const Recti& GetSrcRect(void)           const { return srcRect_; }

    //-----------------------------------------------------

    void SetDstRect(const Recti& r)
    {
        dstRect_ = r;
        rect_    = r;
        pos_     = { r.x0, r.y0 };
        size_    = { r.x1 - r.x0, r.y1 - r.y0 };
    }

    //-----------------------------------------------------
    
    const Recti& GetRectInWorld() const { return rect_; }
    
    //-----------------------------------------------------
    
    const Recti& GetDstRect() const { return dstRect_; }
    
    //-----------------------------------------------------

    void  SetTexture(TexID id)          { texId_ = id; }
    TexID GetTexture()            const { return texId_; }

    //-----------------------------------------------------

    Color GetColor()              const { return color_; }
    void  SetColor(const Color c)       { color_ = c; }

    void SetColor(uint8 r, uint g, uint b, uint a)
    {
        color_.r = r;
        color_.g = g;
        color_.b = b;
        color_.a = a;
    }


private:
    TexID texId_;
    Recti srcRect_;  // src rectangle on the texture atlas
    Recti dstRect_;  // where to render this tile
    Color color_;
    Recti rect_;
    Vec2i pos_;      // position of this tile in world
    Vec2i size_;     // size in the world
};

#endif
