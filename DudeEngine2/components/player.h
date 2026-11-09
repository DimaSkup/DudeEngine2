/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: player.h
    Desc:     component for a player
    Created:  24.06.2026 by DimaSkup
\**********************************************************************************/
#ifndef PLAYER_H
#define PLAYER_H

#include "../types.h"
#include "../vec.h"
#include "component.h"


//---------------------------------------------------------
// component:  Player
//---------------------------------------------------------
class Player : public Component
{
public:
    Player(const float speed, const WeaponID currWpnId);
    
    virtual const char*    GetComponentName() const override;
    virtual eComponentType GetComponentType() const override;

    virtual void           Update(const float dt) override {};
                           
    virtual void           Init()   override {};
    virtual void           Render() override {};
                           
    // player by itself    
    float                  GetMoveBasicSpeed() const;
    float                  GetMoveSpeed() const;
    Vec2                   GetVelocity() const;
    Vec2                   GetMoveDir() const;
    bool                   IsMoving() const;
                           
    void                   SetMoveBasicSpeed(const float speed);
    void                   SetMoveSpeed(const float speed);
    void                   SetMoveDir(const Vec2 dir);
                           
    void                   SetCurrWeapon(const WeaponID id);
    WeaponID               GetCurrWeaponId(void) const;


private:
    float       playerBasicSpeed_;
    float       playerCurrSpeed_;

    Vec2        dir_;
    WeaponID    currWpnId_;
};

//-----------------------------------------------------------------------------
// INLINE FUNCTIONS
//-----------------------------------------------------------------------------
inline void Player::SetCurrWeapon(const WeaponID id) 
{
    currWpnId_ = id;
}

inline WeaponID Player::GetCurrWeaponId(void) const
{
    return currWpnId_;
}

#endif
