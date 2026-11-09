/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: player.cpp
\**********************************************************************************/
#include "player.h"
#include "../macros.h"
#include "../log.h"
#include <assert.h>
#include <memory.h>
#include <string.h>

//---------------------------------------------------------
// constructor
//---------------------------------------------------------
Player::Player(const float speed, const WeaponID currWpnId) :
    playerBasicSpeed_(speed),
    playerCurrSpeed_(speed),
    dir_{ 0,0 },
    currWpnId_(currWpnId)
{
    assert(speed > 0);
}

//---------------------------------------------------------
// return a name / type of this component
//---------------------------------------------------------
const char* Player::GetComponentName() const
{
    return s_ComponentNames[COMPONENT_PLAYER];
}

eComponentType Player::GetComponentType() const
{
    return COMPONENT_PLAYER;
}

//---------------------------------------------------------
// get basic speed of this player
//---------------------------------------------------------
float Player::GetMoveBasicSpeed() const 
{
    return playerBasicSpeed_;
}

//---------------------------------------------------------
// get player's current speed
//---------------------------------------------------------
float Player::GetMoveSpeed() const 
{
    return playerCurrSpeed_;
}

//---------------------------------------------------------
// get player's current velocity
//---------------------------------------------------------
Vec2 Player::GetVelocity() const
{
    return Vec2Mul(dir_, playerCurrSpeed_);
}

//---------------------------------------------------------
// get player's current movement direction (if <0,0> then player don't move)
//---------------------------------------------------------
Vec2 Player::GetMoveDir() const
{
    return dir_;
}

//---------------------------------------------------------
// is player currently moving?
//---------------------------------------------------------
bool Player::IsMoving() const
{
    return Vec2Dot(dir_) > 0;
}

//---------------------------------------------------------
// set player's basic speed (main speed without any affects)
//---------------------------------------------------------
void Player::SetMoveBasicSpeed(const float speed)
{
    if (speed <= 0.0f)
    {
        LogErr(LOG, "invalid speed: %.2f", speed);
        return;
    }

    playerBasicSpeed_ = speed;
}

//---------------------------------------------------------
// setup current speed of the player
//---------------------------------------------------------
void Player::SetMoveSpeed(const float speed)
{
    if (speed <= 0.0f)
    {
        LogErr(LOG, "invalid speed: %.2f", speed);
        return;
    }

    playerCurrSpeed_ = speed;
}

//---------------------------------------------------------
// setup current movement direction (or set <0,0> so player won't move)
//---------------------------------------------------------
void Player::SetMoveDir(const Vec2 dir)
{
    if (dir.x == 0 && dir.y == 0)
    {
        dir_ = { 0,0 };
        return;
    }

    Vec2Normalize(&dir, &dir_);
}