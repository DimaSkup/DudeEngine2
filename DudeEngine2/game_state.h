/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:  game_state.h
    Desc:      global container (struct) for the state of the game and engine
    Created:   by DimaSkup
\**********************************************************************************/
#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "vec.h"

// struct for common game data
struct GameState
{
    void* pRT;      // TEMP!

    float    deltaTime;
    float    gameTime;

    Vec2i    camPos;            // camera top-left position in world
    Vec2i    camSize;           // camera size in world
    Vec2i    camPadding;        // padding of the camera's rectangle from world edges
    Vec2i    wndSize;           // current size of the game/editor window
    Vec2i    screenSize;        // size in pixels of your monitor (screen)
    Vec2     viewportTopLeft;   // top left viewport's corner position on the screen
    Vec2     viewportSize;      // working area (rectangle where render the scene)

    // camera's shaking params
    float    camShake;
    float    camAngle;
    float    camShakeInc;
    float    camShakeMaxAngle;
    float    camShakeMaxOffset;

    // camera's movement params
    float    camAsymptoticAvgWeight;   // 0.01 = nice and slow (at 60 fps), 0.1 = reasonably fast, 0.5 = incredibly fast

    Vec2i    mousePos;          // mouse cursor position on the screen
    Vec2i    mousePosW;         // mouse cursor position in the game world
    //Vec2i    mouseLeftClickPos;
    //Vec2i    mouseWheelClickPos;

    Vec2i    iWorldSize;
    Vec2     fWorldSize;
    Vec2i    iWorldPadding;
    Vec2     fWorldPadding;
    Vec2i    iPlayerPos;
    Vec2     fPlayerPos;

    void*    pGame;             // a ptr to the Game instance :)
    bool     bRunning;          // once set to false then we will exit from the engine/game

    bool     bUseSmoothCamMove;
    bool     bUseCamShaking;

    bool     bShowFog;                  // enable/disable fog rendering
    bool     bShowColliders;            // visualize colliders of entities
    bool     bShowTriggers;             // visualize triggers
    bool     bShowScreenDbgGrid;        
    bool     bShowAttackRange;          // visualize attack range of the player and enemies
    bool     bShowDebugInfo;            
    bool     bGameMode;                 // to switch between editor/game mode
    bool     bUseConsole;               // turn on/off the console
    bool     bStickEnttToMouse;         // a selected entity is stick to the mouse cursor

    bool     bPlayerInvisibleToEnemy;
    bool     bPlayerImmortal;
    bool     bGamePaused;
    bool     bReloadLevel;

    bool     bDebug;
};

// global instance
extern GameState g_GameState;

#endif
