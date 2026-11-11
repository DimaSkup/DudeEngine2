/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:  game.h
    Desc:      this is the heart of this dude
    Created:   by DimaSkup
\**********************************************************************************/
#ifndef GAME_H
#define GAME_H
#include "types.h"
#include "imgui_layer.h"
#include "event_mgr.h"
#include "vec.h"
#include "rect.h"
#include "circle.h"
#include "noise.h"
#include "game_state.h"
#include "r_debug.h"
#include "quest_sys.h"


//---------------------------------------------------------
// forward declaration (pointer use only)
//---------------------------------------------------------
class Map;
class Entity;
class UI_Editor;
class UI_Game;
struct lua_State;
struct Texture;

//---------------------------------------------------------
// container for currently visible entities
//---------------------------------------------------------
struct VisibleEntts
{
    cvector<Entity*> entts;
};

//---------------------------------------------------------
// class name:  Game
//---------------------------------------------------------
class Game
{
public:
    struct GameInit
    {
        char startGameLevel[MAX_LEN_GAME_LVL_NAME];
        char dbgFontName[MAX_LEN_TEX_NAME];
        char dbgFontDataPath[64];
        char gameFontName[MAX_LEN_TEX_NAME];
        char gameFontDataPath[64];
        bool bStartInGameMode;
    };

public:
    Game();
    ~Game();

    void          Init();
    void          ProcessInput();
    void          Update();
    void          Render();
    void          Shutdown();

    bool          IsRunning() const;
    bool          IsPaused() const;
                  
                  
    const char*   GetLevelDirPath() const;
    const char*   GetCurrLevelName() const;
                  
    void          CreateLevel(const char* levelName);
    void          ReadGameInitParams(const char* filepath, GameInit& params);
    void          LoadLevel(const char* levelName, const GameInit& gameInit);
    void          WantLoadLevel(const char* levelName);
    bool          IsLevelExist(const char* levelName) const;
    void          SaveLevel();
    void          LoadMap(const char* mapPath);
                  
    uint          GetNumRenderItems(const eLayerType layer) const;
                  
    EntityID      GetPlayerId();
    Map*          GetMap();
    UI_Editor*    GetEditor();
    EventMgr*     GetEventMgr();
    lua_State*    GetLuaState();
    
    Vec2          ScreenToWorld(const Vec2i screenPos) const;
    Vec2i         WorldToScreen(const Vec2 p) const;
    Vec2i         WorldToScreen(const Vec2i p) const;

    const cvector<EntityID>& GetVisEnemies(void)  const;
    const cvector<EntityID>& GetVisTriggers(void) const;

private:
    //
    // init / load / save / reload helpers
    //
    bool InitLua();

    void ReloadLevel();          // reload the current level
    void LoadLuaScripts();       // initial load / hot reload of scripts
    void CreatePlayer();
    void CreateFog();
    void ResetLevel();
    void CreatePrefabs();

    void RegEvents();
    void RegQuests();

    void LoadLevelSandbox(const GameInit& gameInit);
    void LoadLevelMetadata(const char* filepath, char(*)[128]);
    void SaveLevelMetadata(const char* filepath);

    //
    // input handling helpers
    //
    void HandleCameraMovement();
    void ProcessInputGame();
    void ProcessInputEditor();
    void ProcessInputConsole();

    EntityID PickEntt(const Vec2i worldPos, const eLayerType layer);
    void ChangeSelectedEnttPos(Vec2i center);

    void ToggleGameMode();
    void ToggleEditorMode();
    void ToggleConsole();

    //
    // update helpers
    //
    void FrustumCulling();
    void GatherVisibleColliders();
    void GatherVisibleTriggers();
    void TriggerVisibleTriggers();
    void UpdateCollision(const EntityID id);

    void UpdateWorldSize();
    void UpdateGame(const float dt);
    void UpdateEditor(const float dt);
    void UpdateFog();
    void UpdatePlayerEnttState(Entity* pEntt);
    void UpdateEnemyEnttState(Entity* pEntt);

    

    //
    // render helpers
    //
    void RenderScene();
    void RenderEditor();

    bool ShowClosingEngineMessageBox();
    void PushVisibleEnttsToRender();
    void RenderFogOverScene();

public:
    // all the quests
    cvector<Quest> quests_;

private:
    Map* pMap_;
    UI_Editor* pUiEditor_;
    UI_Game*   pUiGame_;
    DbgRender  dbgRender_;
    lua_State* L;

    Noise noise_;

    EventMgr eventMgr_;


    VisibleEntts visEntts_[NUM_LAYERS];  // arrays of currently visible entities split by layers

    ImGuiLayer imGuiLayer_;
    EntityID playerId_;
    EntityID fogEnttId_;

    Recti fogDstRects_[4];


    // colliders cached data
    cvector<EntityID>    circleCollidersIds_;
    cvector<CircleFixed> circleColliders_;
    cvector<EntityID>    boxCollidersIds_;
    cvector<Recti>       boxColliders_;

    // cached enemies (which are visible)
    cvector<EntityID>    visEnemies_;

    // cached triggers (which are visible)
    cvector<EntityID>    visTriggers_;
    //cvector<EntityID>    visTriggersByPlayer_;    // can be activated only by player
    //cvector<EntityID>    visTriggersByEntts_;     // can be activated by any entity (enemies, projectiles, etc.)

    // we can hide an entity when select it and press H;
    // to show back all the hidden entts we need to press Alt+H;
    EntityID hiddenEntts_[6];

    int ticksPrevFrame_;
    char levelDirPath_[64];
    char levelName_[MAX_LEN_GAME_LVL_NAME];
    char levelToLoad_[MAX_LEN_GAME_LVL_NAME];

    bool bWantLoadLevel_;       // before the next frame we will load another level
};

//=============================================================================
// INLINE FUNCTIONS
//=============================================================================
inline bool Game::IsRunning() const
{
    return g_GameState.bRunning;
}

inline bool Game::IsPaused() const
{
    return g_GameState.bGamePaused;
}

inline Map* Game::GetMap()
{
    return pMap_;
}

inline UI_Editor* Game::GetEditor()
{
    return pUiEditor_;
}

inline EventMgr* Game::GetEventMgr()
{
    return &eventMgr_;
}

inline lua_State* Game::GetLuaState()
{
    return L;
}

inline const char* Game::GetCurrLevelName() const
{
    return levelName_;
}

inline const char* Game::GetLevelDirPath() const
{
    return levelDirPath_;
}

// get currently visible enemies
inline const cvector<EntityID>& Game::GetVisEnemies(void) const
{ 
    return visEnemies_; 
}

// get currently visible triggers
inline const cvector<EntityID>& Game::GetVisTriggers(void) const
{ 
    return visTriggers_; 
}


#endif

