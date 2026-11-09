/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: game.cpp
    Created:  by DimaSkup
\**********************************************************************************/
#include "game.h"
#include "map.h"
#include "entity.h"
#include "log.h"
#include "consts.h"
#include "vec.h"
#include "rect.h"
#include "collision.h"

#include "game_state.h"
#include "math_helpers.h"
#include "mem_helpers.h"
#include "macros.h"
#include "input.h"
#include "prefab_creator.h"
#include "timer.h"
#include "file_sys.h"
#include "file.h"
#include "console.h"

// for using OS filesystem dialog window
#include <tinyfiledialogs/tinyfiledialogs.h>  

// render stuff
#include "r_main.h"
#include "r_list.h"     // render list

// UI stuff
#include "ui_game.h"
#include "ui_editor.h"
#include "ui_facade_engine.h"
#include "ui_editor_tilemap.h"
#include "ui_text_mgr.h"

// managers
#include "entity_mgr.h"
#include "asset_mgr.h"

// component (of Entity-Component)
#include "components/transform.h"
#include "components/sprite.h"
#include "components/keyboard.h"
#include "components/enemy.h"
#include "components/health.h"
#include "components/damage.h"
#include "components/tile.h"
#include "components/collider.h"
#include "components/player.h"
#include "components/movement.h"
#include "components/trigger.h"
#include "components/life_timer.h"
#include "components/hierarchy.h"

// LUA stuff
#include "lua_stuff.h"
#include "lua_lib_sys.h"
#include "lua_lib_game.h"
#include "lua_lib_input.h"
#include "lua_lib_entity.h"
#include "lua_lib_sound.h"
#include "lua_lib_enemy.h"
#include "lua_lib_player.h"
#include "lua_lib_text.h"


#include <imgui.h>

#pragma warning (disable : 4996)

//---------------------------------------------------------
// internal constants
//---------------------------------------------------------
#define NUM_FOG_RECTS 4         // number of fog rectangles around the player

//---------------------------------------------------------
// constructor / destructor
//---------------------------------------------------------
Game::Game() :
    pMap_(nullptr),
    playerId_(INVALID_ENTT_ID),
    fogEnttId_(INVALID_ENTT_ID),
    pUiEditor_(nullptr),
    pUiGame_(nullptr),
    ticksPrevFrame_(0)
{
    STATIC_ASSERT(NUM_LAYERS == ARRAY_SIZE(s_LayerNames));

    pUiEditor_      = new UI_Editor();
    pUiGame_        = new UI_Game();
    g_pFacadeEngine = new UI_FacadeEngine(this);

    snprintf(levelName_,    sizeof(levelName_), defaultLevelName);
    snprintf(levelDirPath_, sizeof(levelDirPath_), "%s%s", PATH_LEVELS_DIR, levelName_);

    bWantLoadLevel_ = false;

    memset(levelToLoad_, 0, sizeof(levelToLoad_));
    memset(hiddenEntts_, 0, sizeof(hiddenEntts_));

    g_GameState.bRunning = false;

    LogMsg(LOG, "game is created");
}

//---------------------------------------------------------

Game::~Game()
{
    lua_close(L);
    Shutdown();

    LogMsg(LOG, "game is shutted down");
    CloseLogger();
}

//---------------------------------------------------------
// initialize all the LUA related stuff
//---------------------------------------------------------
bool Game::InitLua()
{
    L = luaL_newstate();        // opens Lua

    // bind a pointer to the game instance
    lua_newtable(L);
    lua_pushlightuserdata(L, this);
    lua_setfield(L, -2, "pGame");
    lua_setglobal(L, "game_ctx");

    luaL_openlibs(L);           // opens the std libs
    LuaLoadSysLib(L);           // opens custom libs
    LuaLoadGameLib(L);          
    LuaLoadInputLib(L);         
    LuaLoadEntityLib(L);
    LuaLoadSoundLib(L);
    LuaLoadEnemyLib(L);
    LuaLoadPlayerLib(L);
    LuaLoadTextLib(L);


    //LuaRunInterpreter(L);
    
    //
    // before loading of scripts we register bunch of global enumeration values
    //

    // the same as eEnttState
    const EnumValue enttStates[] = 
    {
        { "ENTT_IDLE",   ENTT_IDLE },
        { "ENTT_MOVE",   ENTT_MOVE },
        { "ENTT_ATTACK", ENTT_ATTACK },
        { "ENTT_DEATH",  ENTT_DEATH },
        { "ENTT_DAMAGE", ENTT_DAMAGE },
        { "ENTT_BLOCK",  ENTT_BLOCK },
    };

    // the same as eAnimationType
    const EnumValue spriteAnimTypes[] = 
    {
        { "ANIM_TYPE_IDLE",   ANIM_TYPE_IDLE },     
        { "ANIM_TYPE_MOVE",   ANIM_TYPE_MOVE },     
        { "ANIM_TYPE_ATTACK", ANIM_TYPE_ATTACK },   
        { "ANIM_TYPE_DEATH",  ANIM_TYPE_DEATH },    
        { "ANIM_TYPE_DAMAGE", ANIM_TYPE_DAMAGE },   
        { "ANIM_TYPE_BLOCK",  ANIM_TYPE_BLOCK },    
    };

    // the same as eAIState
    const EnumValue aiStates[] =
    {
        { "AI_IDLE",    AI_IDLE },
        { "AI_MOVE",    AI_MOVE },
        { "AI_ATTACK",  AI_ATTACK },
        { "AI_DEATH",   AI_DEATH },
        { "AI_DAMAGE",  AI_DAMAGE },
        { "AI_BLOCK",   AI_BLOCK },
        { "AI_CHASE",   AI_CHASE },
    };

    // the same as eTextPosFlags
    const EnumValue textFlags[] =
    {
        { "TEXT_CENTER_X",  TEXT_CENTER_X },
        { "TEXT_CENTER_Y",  TEXT_CENTER_Y },
        { "TEXT_SHADOWED",  TEXT_SHADOWED },
        { "TEXT_FADED",     TEXT_FADED },
    };

    // the same as eEnemyType
    const EnumValue enemyTypes[] =
    {
        { "ENEMY_TYPE_BAT",      ENEMY_TYPE_BAT },
        { "ENEMY_TYPE_SKELETON", ENEMY_TYPE_SKELETON },
        { "ENEMY_TYPE_SPIDER",   ENEMY_TYPE_SPIDER },
    };

    // the same as eColliderType
    const EnumValue colliderTypes[] =
    {
        { "COLLIDER_BOX",       COLLIDER_BOX },
        { "COLLIDER_CIRCLE",    COLLIDER_CIRCLE },
    };

    // the same as eFontType
    const EnumValue fontTypes[] =
    {
        { "FONT_TYPE_GAME",     FONT_TYPE_GAME },
        { "FONT_TYPE_DEBUG",    FONT_TYPE_DEBUG },
        { "FONT_TYPE_DIALOG",   FONT_TYPE_DIALOG },
    };

    STATIC_ASSERT(ARRAY_SIZE(enttStates)      == NUM_ENTT_STATES);
    STATIC_ASSERT(ARRAY_SIZE(spriteAnimTypes) == NUM_ANIM_TYPES);
    STATIC_ASSERT(ARRAY_SIZE(aiStates)        == NUM_AI_STATES);
    STATIC_ASSERT(ARRAY_SIZE(aiStates)        == NUM_AI_STATES);
    STATIC_ASSERT(ARRAY_SIZE(textFlags)       == NUM_TEXT_FLAGS);
    STATIC_ASSERT(ARRAY_SIZE(enemyTypes)      == NUM_ENEMY_TYPES);
    STATIC_ASSERT(ARRAY_SIZE(colliderTypes)   == NUM_COLLIDER_TYPES);
    STATIC_ASSERT(ARRAY_SIZE(fontTypes)       == NUM_FONT_TYPES);    

    LuaRegEnumeration(L, enttStates,      NUM_ENTT_STATES);
    LuaRegEnumeration(L, spriteAnimTypes, NUM_ANIM_TYPES);
    LuaRegEnumeration(L, aiStates,        NUM_AI_STATES);    
    LuaRegEnumeration(L, textFlags,       NUM_TEXT_FLAGS);
    LuaRegEnumeration(L, enemyTypes,      NUM_ENEMY_TYPES);
    LuaRegEnumeration(L, colliderTypes,   NUM_COLLIDER_TYPES);
    LuaRegEnumeration(L, fontTypes,       NUM_FONT_TYPES);

    // load scripts by itself
    LoadLuaScripts();

    return true;
}

//---------------------------------------------------------
// initial load / hot reload of scripts
//---------------------------------------------------------
void Game::LoadLuaScripts()
{
    LogMsg(LOG, "load/reload LUA script: start");

    int count = 0;
    char buf[128];
    char scriptFilename[64];
    const char* filename = "scripts.cfg";

    char path[64];
    snprintf(path, sizeof(path), "%s%s", pathToScriptsDir, filename);

    FILE* fp = fopen(path, "r");
    if (!fp)
        LogFatal(LOG, "%s%s", ERR_OPEN_FILE, filename);

    // skip comments section
    while (fgets(buf, sizeof(buf), fp))
    {
        if (buf[0] != ';')
            break;
    }

    // make sure that we're reading the proper file
    if (strncmp(buf, "scripts", 7) != 0)
        LogFatal(LOG, "read some wrong file");

    while (fgets(buf, sizeof(buf), fp))
    {
        // we've done here
        if (buf[0] == '}')
            break;

        count = sscanf(buf, "%s", scriptFilename);
        assert(count == 1 && "can't parse a string with script's filename");

        LuaLoadScript(L, scriptFilename);
    }

    fclose(fp);
    LogMsg(LOG, "load/reload LUA script: success");
}

//---------------------------------------------------------
// Desc:  initialize the game
//---------------------------------------------------------
void Game::Init()
{
    if (!InitLua())
        LogFatal(LOG, "can't init Lua");

    if (!g_Render.Init())
        LogFatal(LOG, "can't init the render");

    if (!imGuiLayer_.Init())
        LogFatal(LOG, "can't init ImGui");

    if (!g_Text.SetLocale(DEFAULT_LOCALE))
        LogFatal(LOG, "can't load locale (%s)");

    // generate/load some default-invalid assets
    if (!g_AssetMgr.Init())
        LogFatal(LOG, "can't init the ASSETS mgr");

    // setup game global state
    memset(&g_GameState, 0, sizeof(g_GameState));
    g_GameState.pGame = this;

    GameInit gameInit;
    ReadGameInitParams("init.cfg", gameInit);  

    if (gameInit.bStartInGameMode)
        ToggleGameMode();

    InputInit();

    RegEvents();
    RegQuests();

    LoadLevel(gameInit.startGameLevel, gameInit);
    //LoadLevelSandbox(gameInit);

    g_GameState.bRunning = true;
}

//---------------------------------------------------------
// reload the current level
//---------------------------------------------------------
void Game::ReloadLevel()
{
    Game::GameInit gameInit;
    ReadGameInitParams("init.cfg", gameInit);
    LoadLevel(levelName_, gameInit);
}

//---------------------------------------------------------
// register game events here and bind handlers for them
//---------------------------------------------------------
void Game::RegEvents()
{
    eventMgr_.Subscribe("entt_keyboard_move",   "EventHandlerKeyboard");
    eventMgr_.Subscribe("entt_nomove",          "EventHandlerEnttNoMove");
    eventMgr_.Subscribe("DAMAGE_ENTT",          "EventHandlerDamageEntt");
    eventMgr_.Subscribe("teleport_dungeon",     "EventLoadLevelDungeon");
    eventMgr_.Subscribe("teleport_meadows",     "EventLoadLevelMeadows");
    eventMgr_.Subscribe("teleport_graveyard",   "EventLoadLevelGraveyard");
    eventMgr_.Subscribe("ENTITY_DIED",          "EventHandlerEnttDied");
    eventMgr_.Subscribe("NEAR_DEAD_ENEMY",      "EventHandlerNearDeadEnemy");

    eventMgr_.Subscribe("ON_ENTER_TEST",        "LuaTestOnEnter");
    eventMgr_.Subscribe("ON_COLLIDE_TEST",      "LuaHealZone");
    eventMgr_.Subscribe("ON_LEAVE_TEST",        "LuaTestOnLeave");
}

//---------------------------------------------------------
//---------------------------------------------------------
void Game::RegQuests()
{
    // manually add some quests
    quests_.push_back(Quest());
    Quest& quest = quests_.back();

    InitQuestKillBats(&quest);
}

//---------------------------------------------------------
// Desc:  handle all the input from the player's devices
//---------------------------------------------------------
void Game::ProcessInput()
{
    InputProcess();

    if (g_GameState.bGameMode)
        ProcessInputGame();
    else
        ProcessInputEditor();
}

//---------------------------------------------------------
// convert screen position into world position
//---------------------------------------------------------
Vec2 Game::ScreenToWorld(const Vec2i s) const
{
    const Vec2i viewportPos = ToVec2i(g_GameState.viewportTopLeft);
    const Vec2i offset      = { viewportPos.x - g_GameState.camPos.x,
                                viewportPos.y - g_GameState.camPos.y };

    return ToVec2(Vec2iSub(s, offset));
}

//---------------------------------------------------------
// convert world position into screen coords
//---------------------------------------------------------
Vec2i Game::WorldToScreen(const Vec2 p) const 
{
    const Vec2i viewportPos = ToVec2i(g_GameState.viewportTopLeft);
    const Vec2i offset      = { viewportPos.x - g_GameState.camPos.x, 
                                viewportPos.y - g_GameState.camPos.y };   

    return Vec2i{ (int)p.x + offset.x, (int)p.y + offset.y };
}

Vec2i Game::WorldToScreen(const Vec2i p) const 
{
    const Vec2i viewportPos = ToVec2i(g_GameState.viewportTopLeft);
    const Vec2i offset      = { viewportPos.x - g_GameState.camPos.x, 
                                viewportPos.y - g_GameState.camPos.y }; 
    
    return Vec2i{ p.x + offset.x, p.y + offset.y };
}

//---------------------------------------------------------
// check if two input colliders intersect each other
//---------------------------------------------------------
bool IsCollide(Collider* c1, Collider* c2)
{
    assert(c1 && c2);

    const eColliderType type1 = c1->GetType();
    const eColliderType type2 = c2->GetType();

    if (type1 == COLLIDER_BOX)
    {
        if (type2 == COLLIDER_BOX)
            return CollisionRectiRecti(c1->GetBoxInWorld(), c2->GetBoxInWorld());

        else if (type2 == COLLIDER_CIRCLE)
            return CollisionBoxCircle(c1->GetBoxInWorld(), c2->GetCircleInWorld());
    }

    else if (type1 == COLLIDER_CIRCLE)
    {
        if (type2 == COLLIDER_CIRCLE)
            return CollisionCircleCircle(c1->GetCircleInWorld(), c2->GetCircleInWorld());

        else if (type2 == COLLIDER_BOX)
            return CollisionBoxCircle(c2->GetBoxInWorld(), c1->GetCircleInWorld());
    }

    LogErr(LOG, "unknown type of colliders (collider1: %d, collider2: %d)", (int)type1, (int)type2);
    return false;
}

//---------------------------------------------------------
// Desc:  updating all the parts of the game
//---------------------------------------------------------
void Game::Update()
{
    if (bWantLoadLevel_)
    {
        GameInit gameInit;
        ReadGameInitParams("init.cfg", gameInit);
        LoadLevel(levelToLoad_, gameInit);
    }

    if (g_GameState.bReloadLevel)
    {
        ReloadLevel();
        g_GameState.bReloadLevel = false;
    }

    const uint32 ticks = Sys_GetTicks();

    // sleep the execution until we reach the target frame time in milliseconds
    int timeToWait = FRAME_TARGET_TIME - (ticks - ticksPrevFrame_);

    // only call decaly if we are too fast to process this frame
    if (timeToWait > 0 && timeToWait <= FRAME_TARGET_TIME)
        Sys_Delay(timeToWait);

    // delta time
    float dt = (ticks - ticksPrevFrame_) * 0.001f;

    // clamp delta time to a maximum value
    dt = (dt > 0.05f) ? 0.05f : dt;

    g_GameState.deltaTime = dt;
    g_GameState.gameTime += dt;

    // set the new ticks for the current frame to be used in the next pass
    ticksPrevFrame_ = ticks;


    if (IsPaused())
        return;

    if (g_GameState.bGameMode)
        UpdateGame(dt);
    else
        UpdateEditor(0.0f);

    FrustumCulling();
    GatherVisibleColliders();



    if (g_GameState.bGameMode)
    {
        const Event* pEvent = nullptr;

        // handle events from the events queue
        while (pEvent = eventMgr_.GetNextEvent())
        {
            if (pEvent->id != INVALID_EVENT_ID)
            {
                eventMgr_.TriggerEvent(pEvent->id, this, &pEvent->data);
            }
            else
            {
                const EventID id = eventMgr_.GetEventIdByName(pEvent->name);
                eventMgr_.TriggerEvent(id, this, &pEvent->data);
            }
        }

        //g_EventsQueue.clear();

        // gather and update all the visible enemies
        visEnemies_.clear();

        for (Entity* pEntt : visEntts_[LAYER_ENTITY].entts)
        {
            if (pEntt->HasComponent<Enemy>())
                visEnemies_.push_back(pEntt->GetId());
        }

        // update player 
        UpdateCollision(playerId_);
        UpdatePlayerEnttState(g_EnttMgr.GetEntt(playerId_));

        // update enemies
        for (const EntityID id : visEnemies_)
        {
            UpdateEnemyEnttState(g_EnttMgr.GetEntt(id));
            UpdateCollision(id);
        }
    }

    //
    // triggers stuff
    //
    GatherVisibleTriggers();

    if (g_GameState.bGameMode)
    {
        TriggerVisibleTriggers();
    }


    // remove entities whose life has expired
    for (Entity* pEntt : visEntts_[LAYER_ENTITY].entts)
    {
        if (pEntt->HasComponent<LifeTimer>())
        {
            const LifeTimer* t = pEntt->GetComponent<LifeTimer>();
            if (t->GetRemainLifeTime() <= 0)
                g_EnttMgr.SetToRemove(pEntt);
        }
    }

    g_Text.Update(g_GameState.deltaTime);
}

//---------------------------------------------------------
// Desc:  render the game level onto the screen
//---------------------------------------------------------
void Game::Render()
{
    // clear the render lists before the frame
    ClearRenderLists();

    // set the background color 
    g_Render.SetDrawColor(0, 0, 0, 255);

    // clear the back buffer
    g_Render.BeginFrame();

    if (g_GameState.bGameMode)
    {
        if (g_GameState.bGamePaused)
            pUiGame_->RenderGameMenu();
        else
            RenderScene();
    }
    else
    {
        // in editor we render scene into a specific render target
        g_Render.AttachRenderTarget();
        
        RenderScene();
    }

    //
    // render debugging stuff
    //
    dbgRender_.Render(visEntts_[LAYER_ENTITY].entts,
                      visTriggers_,
                      visEnemies_);

    if (g_GameState.bGameMode)
    {
        LuaCall(L, "RenderSceneUI");
    }

    
    g_Text.Render();


    if (!g_GameState.bGameMode)
    {
        g_Render.DetachRenderTarget();
        RenderEditor();
    }

    if (g_GameState.bUseConsole)
    {
        ConsoleRender();
    }

    if (g_GameState.bGameMode)
    {
        pUiGame_->RenderMouseCursor();
    }


    // swap front and back buffers
    g_Render.RenderPresent();
}

//---------------------------------------------------------
// Desc:  release the game resources, entities, render, etc.
//---------------------------------------------------------
void Game::Shutdown()
{
    SafeDelete(pMap_);
    SafeDelete(pUiEditor_);
    g_AssetMgr.ClearData();
    g_EnttMgr.ClearData();
}

//---------------------------------------------------------
// create a new empty level
//---------------------------------------------------------
void Game::CreateLevel(const char* levelName)
{
    assert(levelName && levelName[0] != '\0');

    // create a directory for this level
    snprintf(levelDirPath_, sizeof(levelDirPath_), "%s%s/", PATH_LEVELS_DIR, levelName);
    if (!FileSys::CreateDir(levelDirPath_))
    {
        LogErr(LOG, "can't create new a level: %s", levelName);
        return;
    }

    // setup a name of the level
    snprintf(levelName_, sizeof(levelName_), "%s", levelName);

    GameInit gameInit;
    ReadGameInitParams("init.cfg", gameInit);

    ResetLevel();                   // release all the previous resources
    LoadLevelSandbox(gameInit);     // load empty level
    SaveLevel();                    // and save it to create related files
    CreatePrefabs();
    WantLoadLevel(levelName_);
}

//---------------------------------------------------------
// set that before the next update we load another level by name
//---------------------------------------------------------
void Game::WantLoadLevel(const char* levelName)
{
    assert(levelName && levelName[0] != '\0');

    if (!IsLevelExist(levelName))
        return;

    snprintf(levelToLoad_, sizeof(levelToLoad_), "%s", levelName);
    bWantLoadLevel_ = true;
}

//---------------------------------------------------------
// check if we have such a game level by input name
//---------------------------------------------------------
bool Game::IsLevelExist(const char* levelName) const
{
    assert(levelName && levelName[0] != '\0');

    char pathToLevelFile[128];
    snprintf(pathToLevelFile, sizeof(pathToLevelFile), "%s%s", PATH_LEVELS_DIR, levelName);

    if (FileSys::IsDirExist(pathToLevelFile))
        return true;
    
    LogErr(LOG, "DUDE, there is no level by name: %s", levelName);
    return false;
}

//---------------------------------------------------------
// read game initialization parameters from a file;
// and setup some global states of the game
//---------------------------------------------------------
void Game::ReadGameInitParams(const char* filepath, GameInit& params)
{
    assert(filepath && filepath[0] != '\0');
    
    memset(&params, 0, sizeof(params));

    FILE* fp = fopen(filepath, "r");
    if (!fp)
        LogFatal(LOG, "%s%s", ERR_OPEN_FILE, filepath);

    int count = 0;
    char buf[128];
    char key[32];
    char value[64];

    while (fgets(buf, sizeof(buf), fp))
    {
        count = sscanf(buf, "%s %s", key, value);
        if (count != 2)
            LogFatal(LOG, "can't parse a str '%s' from file: %s", buf, filepath);


        if (strcmp(key, "start_level") == 0)
            strcpy(params.startGameLevel, value);

        else if (strcmp(key, "b_start_in_game_mode") == 0)
            params.bStartInGameMode = (bool)atoi(value);

        else if (strcmp(key, "dbg_font") == 0)
            strcpy(params.dbgFontName, value);

        else if (strcmp(key, "dbg_font_data") == 0)
            strcpy(params.dbgFontDataPath, value);

        else if (strcmp(key, "game_font") == 0)
            strcpy(params.gameFontName, value);

        else if (strcmp(key, "game_font_data") == 0)
            strcpy(params.gameFontDataPath, value);

        else if (strcmp(key, "cam_shake_inc") == 0)
            g_GameState.camShakeInc = (float)atof(value);

        else if (strcmp(key, "cam_shake_max_angle") == 0)
            g_GameState.camShakeMaxAngle = (float)atof(value);

        else if (strcmp(key, "cam_shake_max_offset") == 0)
            g_GameState.camShakeMaxOffset = (float)atof(value);

        else if (strcmp(key, "cam_asymptotic_avg_weight") == 0)
            g_GameState.camAsymptoticAvgWeight = (float)atof(value);

        else if (strcmp(key, "b_cam_smooth_move") == 0)
            g_GameState.bUseSmoothCamMove = (bool)atoi(value);

        else if (strcmp(key, "b_cam_shaking") == 0)
            g_GameState.bUseCamShaking = (bool)atoi(value);

        else if (strcmp(key, "world_pad_x") == 0)
            g_GameState.iWorldPadding.x = atoi(value);

        else if (strcmp(key, "world_pad_y") == 0)
            g_GameState.iWorldPadding.y = atoi(value);

        else
            LogErr(LOG, "unknown key '%s' from the init file: %s", key, filepath);
    }


    g_GameState.camSize       = g_Render.GetWndSize();
    g_GameState.wndSize       = g_Render.GetWndSize();
    g_GameState.screenSize    = g_Render.GetFullscreenSize();
    g_GameState.fWorldPadding = ToVec2(g_GameState.iWorldPadding);
    
    assert(g_GameState.camShakeInc >= 0);
    assert(g_GameState.iWorldPadding.x >= 0);
    assert(g_GameState.iWorldPadding.y >= 0);
    assert(g_GameState.camAsymptoticAvgWeight > 0);
    assert(g_GameState.camSize.w > 0 && g_GameState.camSize.h > 0);
    assert(g_GameState.wndSize.w > 0 && g_GameState.wndSize.h > 0);

    fclose(fp);
}

//---------------------------------------------------------
// Desc:  load a level by name
//---------------------------------------------------------
void Game::LoadLevel(const char* levelName, const GameInit& gameInit)
{
    assert(levelName && levelName[0] != '\0');
    LogMsg(LOG, "start loading of a level: %s", levelName);

    ResetLevel();
    
    char levelFilepath[128];
    char mapPath[128];
    char enttsFilepath[64];
    char soundsFilepath[64];

    snprintf(levelName_,      sizeof(levelName_),      levelName);
    snprintf(levelDirPath_,   sizeof(levelDirPath_),   "%s%s/",          PATH_LEVELS_DIR, levelName);
    snprintf(levelFilepath,   sizeof(levelFilepath),   "%s/level.lvl",   levelDirPath_);
    snprintf(enttsFilepath,   sizeof(enttsFilepath),   "%s/entts.entt",  levelDirPath_);
    snprintf(soundsFilepath,  sizeof(soundsFilepath),  "%s/sounds.cfg",  levelDirPath_);

    // load level's data and assets
    LoadLevelMetadata(levelFilepath, &mapPath);
    g_EnttMgr.LoadEntts(enttsFilepath);
    g_EnttMgr.LoadEntts("assets/common/ui.entt");
    g_AssetMgr.LoadSounds(soundsFilepath, AssetMgr::ASSET_ATTR_NONE);

    // load another map
    LoadMap(mapPath);

    Entity* pPlayer = g_EnttMgr.GetEntt("player");
    assert(pPlayer);
    playerId_ = pPlayer->GetId();

    // setup the fog
    if (g_EnttMgr.HasEntt("fog"))
    {
        fogEnttId_ = g_EnttMgr.GetEntt("fog")->GetId();
        g_GameState.bShowFog = true;
    }

    bWantLoadLevel_ = false;
    memset(levelToLoad_, 0, sizeof(levelToLoad_));

    //
    // load fonts
    //
    TexID dbgFontTexId  = g_AssetMgr.GetTexId(gameInit.dbgFontName);
    TexID gameFontTexId = g_AssetMgr.GetTexId(gameInit.gameFontName);
    
    g_Text.InitFont(FONT_TYPE_DEBUG, dbgFontTexId,  gameInit.dbgFontDataPath);
    g_Text.InitFont(FONT_TYPE_GAME,  gameFontTexId, gameInit.gameFontDataPath);
    g_Text.SetFontType(FONT_TYPE_GAME);

    //
    // manually create some stuff
    // 
    //CreateUI();
    UpdateEditor(1.0f);
    CreatePrefabs();

    // by default hide editor's cursor (a red square) 
    GetEditor()->SetShowEditorCursor(false);

    // do some additional setup using scripts
    LuaCall(L, "OnInit");

    LogMsg(LOG, "level is loaded: %s", levelName_);
}

//---------------------------------------------------------
// save current level
//---------------------------------------------------------
void Game::SaveLevel()
{
    // don't save a sandbox level
    if (strcmp(levelName_, "sandbox") == 0)
        return;

    char filepath[128];

    g_AssetMgr.SaveCommonAssets();

    // save data related to this level
    snprintf(filepath, sizeof(filepath), "%s%s", levelDirPath_, "level.lvl");
    SaveLevelMetadata(filepath);

    snprintf(filepath, sizeof(filepath), "%s%s", levelDirPath_, "entts.entt");
    g_EnttMgr.SaveEntts(filepath);

    snprintf(filepath, sizeof(filepath), "%s%s", levelDirPath_, "sounds.cfg");
    g_AssetMgr.SaveSounds(filepath);

    assert(pMap_);
    pMap_->SaveMap();

    LogMsg(LOG, "level (%s) was saved", levelName_);
}

//---------------------------------------------------------
// load a map from a file by input path
// and setup some params dependent on the map
//---------------------------------------------------------
void Game::LoadMap(const char* mapPath)
{
    if (pMap_ == nullptr)
        pMap_ = new Map();

    pMap_->LoadMap(mapPath);
    UpdateWorldSize();
}

//---------------------------------------------------------
// return the number of rendered items related to the layer
//---------------------------------------------------------
uint Game::GetNumRenderItems(const eLayerType layer) const
{
    assert(layer >= 0 && layer < NUM_LAYERS);
    return (uint)g_RenderLists[layer].items.size();
}

//---------------------------------------------------------
// return an ID of the main player
//---------------------------------------------------------
EntityID Game::GetPlayerId()
{
    return g_EnttMgr.GetPlayerId();
}


//==================================================================================
//==================================================================================
// 
//                             PRIVATE HELPERS
// 
//==================================================================================
//==================================================================================

//---------------------------------------------------------
// cache all the currenly visible entities;
// entity is visible if it is within camera's boundaries;
//---------------------------------------------------------
void Game::FrustumCulling()
{
    for (int i = 0; i < NUM_LAYERS; ++i)
        visEntts_[i].entts.clear();

    // build camera's rectangle for culling
    const Vec2i camPos  = g_GameState.camPos;
    const int scale = g_Render.GetDrawScale();
    const Vec2i camSz   = Vec2iMul(g_GameState.camSize, scale);
    const Recti camRect = { camPos.x, camPos.x+camSz.w, camPos.y, camPos.y+camSz.h };
    

    // go through each layer of entities and do culling
    for (int i = 0; i < NUM_LAYERS; ++i)
    {
        const eLayerType layer = eLayerType(i);

        if (!g_EnttMgr.IsShowLayer(layer))
            continue;

        // tiles of the map we handle in a specific way
        if (layer == LAYER_TILEMAP)
        {
            for (Entity* pEntt : g_EnttMgr.GetEnttsByLayer(layer))
            {
                const Tile* t = pEntt->GetComponent<Tile>();
                if (CollisionRectiRecti(camRect, t->GetRectInWorld()))
                    visEntts_[i].entts.push_back(pEntt);
            }
        }

        else if (layer == LAYER_ENTITY || layer == LAYER_GND_DECAL)
        {
            for (Entity* pEntt : g_EnttMgr.GetEnttsByLayer(layer))
            {
                const Recti& r = pEntt->GetComponent<Transform>()->GetRecti();
                if (CollisionRectiRecti(camRect, r))
                    visEntts_[i].entts.push_back(pEntt);
            }
        }

        // push UI elemenets without any test
        else
        {
            visEntts_[i].entts = g_EnttMgr.GetEnttsByLayer(layer);
        }
    }
}

//---------------------------------------------------------
// gather all the currenly visible and SOLID (can't move through)
// entities with colliders
// (call it only after frustum culling for this frame since we use its results)
//---------------------------------------------------------
void Game::GatherVisibleColliders()
{
    circleCollidersIds_.clear();
    circleColliders_.clear();
    boxCollidersIds_.clear();
    boxColliders_.clear();

    // gather entities with colliders
    for (const Entity* pEntt : visEntts_[LAYER_ENTITY].entts)
    {
        if (!pEntt->HasComponent<Collider>())
            continue;

        const Collider* c = pEntt->GetComponent<Collider>();

        if (!c->IsSolid())
            continue;

        switch (c->GetType())
        {
            case COLLIDER_BOX:
            {
                boxCollidersIds_.push_back(pEntt->GetId());
                boxColliders_.push_back(c->GetBoxInWorld());
                break; 
            }
            case COLLIDER_CIRCLE:
            {
                circleCollidersIds_.push_back(pEntt->GetId());
                circleColliders_.push_back(c->GetCircleInWorld());
                break;
            }
            default:
                LogErr(LOG, "unknown collider type (%d) for entt (%s)", (int)c->GetType(), pEntt->GetName());
        }
    }
}

//---------------------------------------------------------
// update collision of entity by input ID
//---------------------------------------------------------
void Game::UpdateCollision(const EntityID id)
{
    Entity* pEntt = g_EnttMgr.GetEntt(id);

    assert(pEntt);
    assert(pEntt->HasComponent<Transform>());
    assert(pEntt->HasComponent<Collider>());

    Transform*                t = pEntt->GetComponent<Transform>();
    const Collider*        pCol = pEntt->GetComponent<Collider>();
    const eColliderType colType = pCol->GetType();
    Vec2 vel = { 0,0 };

    // get current velocity of the entity
    if (pEntt->HasComponent<Player>())
    {
        vel = pEntt->GetComponent<Player>()->GetVelocity();
    }
    else if (pEntt->HasComponent<Enemy>())
    {
        vel = pEntt->GetComponent<Enemy>()->GetVelocity();
    }

    // we do several steps (iterations) for collision tests
    const int steps = 3;
    const float deltaStride = g_GameState.deltaTime / steps;

    if (colType == COLLIDER_CIRCLE)
    {
        const Vec2         offsetBase = Vec2Scale(vel, deltaStride);
        const CircleFixed circleLocal = pCol->GetCircle();

        // do several iterations for collision tests
        for (int it = 1; it <= steps; ++it)
        {
            Vec2           pos = t->GetPos();
            Vec2        offset = offsetBase;
            CircleFixed circle = circleLocal;

            // calc colliders in world space (separately horizontal and vertical offset)
            CircleFixed circleH = circle;
            CircleFixed circleV = circle;

            const Vec2i iPosOffset = ToVec2i(Vec2Add(pos, offset));

            // horizonal offset
            circleH.cx += iPosOffset.x;
            circleH.cy += (int)(pos.y);

            // vertical offset
            circleV.cx += (int)(pos.x);
            circleV.cy += iPosOffset.y;

            // full offset
            circle.cx += iPosOffset.x;
            circle.cy += iPosOffset.y;

            //
            // do collision tests against circular colliders
            //
            for (index i = 0; i < circleColliders_.size(); ++i)
            {
                // prevent collision with itself
                if (circleCollidersIds_[i] == id)
                    continue;

                if (CollisionCircleCircle(circle, circleColliders_[i]))
                {
                    const CircleFixed& target = circleColliders_[i];
                    const Vec2i        center = { circle.cx, circle.cy };
                    const Vec2i  targetCenter = { target.cx, target.cy };
                    const Vec2 targetToCircle = ToVec2(Vec2iSub(center, targetCenter));
                    const float   distCenters = Vec2Length(targetToCircle);

                    if (distCenters < EPSILON_E3)
                        continue;

                    const float intersectDist = (float)(circle.r + target.r) - distCenters;
                    const float invDist = 1.0f / distCenters;

                    // adjust offset by distance of intersection 
                    offset.x += (targetToCircle.x * invDist * intersectDist);
                    offset.y += (targetToCircle.y * invDist * intersectDist);
                }
            }

            //
            // do collision tests against rectangular colliders
            //
            for (index i = 0; i < boxColliders_.size(); ++i)
            {
                if (CollisionBoxCircle(boxColliders_[i], circleH))
                    offset.x = 0;

                if (CollisionBoxCircle(boxColliders_[i], circleV))
                    offset.y = 0;
            }

            // offset the entity a bit
            t->Translate(offset);
        }
    }


    if (colType == COLLIDER_BOX)
    {
        const Vec2 offsetBase = Vec2Scale(vel, deltaStride);
        const Recti  boxLocal = pCol->GetBox();

        // do several iterations for collision tests 
        for (int it = 1; it <= steps; ++it)
        {
            Vec2 offset = offsetBase;

            // calc colliders in world space (separately horizontal and vertical offset)
            Recti boxH = boxLocal;
            Recti boxV = boxLocal;

            const Vec2        fPos = t->GetPos();
            const Vec2i       iPos = ToVec2i(fPos);
            const Vec2i iPosOffset = ToVec2i(Vec2Add(fPos, offset));

            boxH.x0 += iPosOffset.x;
            boxH.x1 += iPosOffset.x;
            boxH.y0 += iPos.y;
            boxH.y1 += iPos.y;

            boxV.x0 += iPos.x;
            boxV.x1 += iPos.x;
            boxV.y0 += iPosOffset.y;
            boxV.y1 += iPosOffset.y;

            // do collision tests
            for (index i = 0; i < boxColliders_.size(); ++i)
            {
                // prevent collision with itself
                if (boxCollidersIds_[i] == id)
                    continue;

                if (CollisionRectiRecti(boxH, boxColliders_[i]))
                    offset.x = 0;

                if (CollisionRectiRecti(boxV, boxColliders_[i]))
                    offset.y = 0;
            }

            //
            // do collision tests against circular colliders
            //
            for (index i = 0; i < circleColliders_.size(); ++i)
            {
                if (CollisionBoxCircle(boxH, circleColliders_[i]))
                    offset.x = 0;

                if (CollisionBoxCircle(boxV, circleColliders_[i]))
                    offset.y = 0;
            }

            // offset the entity a bit
            t->Translate(offset);
        }
    }
}

//---------------------------------------------------------
// Finite State Machine (FSM) for the player
//---------------------------------------------------------
void Game::UpdatePlayerEnttState(Entity* pEntt)
{
    assert(pEntt);

    switch (pEntt->GetState())
    {
        case ENTT_IDLE:     // default state (staying at one place and do nothing)
        {
            LuaCallVa(L, "UpdatePlayerIdleState", "i", playerId_);
            break;
        }
        case ENTT_MOVE:      // movement: walk, run, etc.
        {
            LuaCallVa(L, "UpdatePlayerMoveState", "i", playerId_);
            break;
        }
        case ENTT_ATTACK:   // attacking with weapon
        {
            LuaCallVa(L, "UpdatePlayerAttackState", "i", playerId_);
            break;
        }
        case ENTT_DEATH:     // dies
            break;
        case ENTT_DAMAGE:    // takes damage
            break;
        case ENTT_BLOCK:     // block income attacks
            break;
        default:
            LogErr(LOG, "unknown state of player");
    }
}

//---------------------------------------------------------
// Finite State Machine (FSM) for enemies
//---------------------------------------------------------
void Game::UpdateEnemyEnttState(Entity* pEntt)
{
    assert(pEntt);
    assert(pEntt->HasComponent<Transform>());
    assert(pEntt->HasComponent<Enemy>());
    
    switch (pEntt->GetComponent<Enemy>()->GetCurrState())
    {
        // default state (staying at one place and do nothing)
        case AI_IDLE:
        {
            LuaCallVa(L, "UpdateEnemyIdleState", "i", pEntt->GetId());
            break;
        }
        case AI_MOVE: 
        { 
            LuaCallVa(L, "UpdateEnemyMoveState", "i", pEntt->GetId());
            break; 
        }
        case AI_ATTACK: 
        {
            LuaCallVa(L, "UpdateEnemyAttackState", "i", pEntt->GetId());
            break;
        }
        case AI_DEATH: 
        { 
            break;
        }
        case AI_DAMAGE: 
        {
            break;
        }
        case AI_BLOCK: 
        { 
            break;
        }
        case AI_CHASE:   // chase the player 
        {
            LuaCallVa(L, "UpdateEnemyChasingState", "i", pEntt->GetId());
            break;
        }
        default:
        {
            LogErr(LOG, "unknown state for enemy");
        }
    }
}

//---------------------------------------------------------
// move selected entity in the editor using mouse
//---------------------------------------------------------
void Game::ChangeSelectedEnttPos(Vec2i center)
{
    const EntityID id = GetEditor()->GetSelectedEnttId();

    // no entity is selected
    if (id == INVALID_ENTT_ID)  
        return;

    Transform* t = g_EnttMgr.GetEntt(id)->GetComponent<Transform>();
    Vec2i     sz = t->GetWorldSize();
    Vec2i offset = Vec2iSub(center, { sz.x >> 1, sz.y >> 1 });

    // setup position of the entt's top-left corner 
    t->SetPos(ToVec2(offset));
}

//---------------------------------------------------------
// helper for processing input in the console
//---------------------------------------------------------
void Game::ProcessInputConsole()
{
    InputEvent e;

    while (InputGetEvent(&e))
    {
        if (e.type == EVENT_KEYDOWN)
        {
            const bool bRepeat = e.keyboard.bRepeat;

            if (e.keyboard.code == KEY_NUMPAD0 && !bRepeat)
                ToggleConsole();

            else if (e.keyboard.code == KEY_ENTER && !bRepeat)
                ConsoleProc();

            else if (!bRepeat)
                ConsoleInput(e.keyboard.code);
        }
    }
}

//---------------------------------------------------------
// helper for processing input in the game mode
//---------------------------------------------------------
void Game::ProcessInputGame()
{
    if (IsPaused())
    {
        pUiGame_->ProcessInputGameMenu();
        return;
    }

    if (g_GameState.bUseConsole)
    {
        ProcessInputConsole();
        return;
    }

    InputEvent e;

    while (InputGetEvent(&e))
    {
        if (e.type == EVENT_QUIT)
        {
            g_GameState.bRunning = false;
        }

        if (e.type == EVENT_KEYDOWN)
        {
            LuaCall(L, "PlayerInput");

            const bool bRepeat = e.keyboard.bRepeat;

            if (e.keyboard.code == KEY_ESCAPE && !bRepeat)
            {
                g_GameState.bGamePaused = true;
                g_Text.ClearFadedTexts();
            }

            else if (e.keyboard.code == KEY_F1 && !bRepeat)
                ToggleEditorMode();

            else if (e.keyboard.code == KEY_F2 && !bRepeat)
                GetEditor()->ToggleCameraNoClip();

            else if (e.keyboard.code == KEY_F3 && !bRepeat)
                g_GameState.bShowDebugInfo = !g_GameState.bShowDebugInfo;

            else if (e.keyboard.code == KEY_F4 && !bRepeat)
            {
                // hot-reload of Lua-scripts
                LoadLuaScripts();
            }

            else if (e.keyboard.code == KEY_F5 && !bRepeat)
            {
                g_GameState.bDebug = true;
            }

            else if (e.keyboard.code == KEY_NUMPAD0 && !bRepeat)
                ToggleConsole();
        }

        else if (e.type == EVENT_KEYUP)
        {
        }

        else if (e.type == EVENT_MOUSE)
        {
            switch (e.mouse.type)
            {
                case MOUSE_LPRESS:          break;
                case MOUSE_LRELEASE:        break;
                case MOUSE_RPRESS:          break;
                case MOUSE_RRELEASE:        break;
                case MOUSE_MPRESS:          break;
                case MOUSE_MRELEASE:        break;
                case MOUSE_WHEEL_UP:        break;
                case MOUSE_WHEEL_DOWN:      break;

                case MOUSE_MOVE:
                {
                    g_GameState.mousePos  = { e.mouse.x, e.mouse.y };
                    g_GameState.mousePosW = Vec2iAdd(g_GameState.camPos, g_GameState.mousePos);
                    break;
                }
            } // switch (mouse_event_type)
        } // if mouse_event
    } // while
}

//---------------------------------------------------------
// Desc:  show a message box before closing the engine
// Ret:   true - continue work, false - exit
//---------------------------------------------------------
bool Game::ShowClosingEngineMessageBox()
{
    const int defaultBtn = 1;   // 0 for cancel/no , 1 for ok/yes , 2 for no in yesnocancel

    int res = tinyfd_messageBox("Save?",
                                "Save current changes before closing?",
                                "yesnocancel",
                                "question",
                                defaultBtn);
    // cancel
    if (res == 0)
        return true;

    // yes - save
    if (res == 1)
    {
        if (strcmp(levelName_, "sandbox") == 0)
        {
            tinyfd_messageBox("dude...", "You cannot save level as a sandbox, create another level!", "ok", "error", 1);
            return true;
        }
        else
        {
            SaveLevel();
            return false;
        }
    }

    // no - don't save
    else if (res == 2)
    {
        return false;
    }

    return true;
}

//---------------------------------------------------------
// helper for processing input in the editor mode
//---------------------------------------------------------
void Game::ProcessInputEditor()
{
    static bool bEntityCreated = false;

    InputEvent e;

    while (InputGetEvent(&e))
    {
        if (e.type == EVENT_QUIT)
        {
            g_GameState.bRunning = ShowClosingEngineMessageBox();
        }

        else if (e.type == EVENT_MOUSE)
        {
            const Mouse& mouse = GetMouseState();

            if (e.mouse.type == MOUSE_MOVE)
            { 
                Vec2i m = { e.mouse.x, e.mouse.y };
                g_GameState.mousePos = m;

                if (!GetEditor()->IsSceneViewportHovered())
                    continue;

                // this shit is done to adjust mouse position in world
                // if we scale down the world
                const int s = g_Render.GetDrawScale();
                if (s != 1)
                {
                    m.x *= s;
                    m.y *= s;
                    m.x -= (10 * s);
                    m.y -= (25 * s);
                }

                // calculate position of the mouse cursor in the world
                const Vec2i workAreaPos = ToVec2i(g_GameState.viewportTopLeft);
                g_GameState.mousePosW.x = m.x + g_GameState.camPos.x - workAreaPos.x;
                g_GameState.mousePosW.y = m.y + g_GameState.camPos.y - workAreaPos.y;
               
                if (pUiEditor_->IsAnyModalWndOpened())
                    continue;

                switch (pUiEditor_->GetActiveEditor())
                {
                    case EDITOR_PREFABS:
                    case EDITOR_ENTITIES:
                    {
                        // move the selected entity
                        if (g_GameState.bStickEnttToMouse)
                            ChangeSelectedEnttPos(g_GameState.mousePosW);
                        break;
                    }
                    case EDITOR_TILEMAP:
                    {
                        if (mouse.button[MOUSE_L].bDown)
                            GetEditor()->ModifyMapTile(g_GameState.mousePosW);
                        break;
                    }
                }
            }

            else if (e.mouse.type == MOUSE_LPRESS)
            { 
                g_GameState.bStickEnttToMouse = false;

                // prevent change pos of the cell cursor when mouse over the editor's window
                if (!GetEditor()->IsSceneViewportHovered())
                    continue;

                if (GetEditor()->IsAnyModalWndOpened())
                    continue;

                // if we clicked out of the scene
                if (g_GameState.mousePosW.x < 0 || g_GameState.mousePosW.y < 0)
                    continue;
               

                switch (pUiEditor_->GetActiveEditor())
                {
                    case EDITOR_TILEMAP:
                    {
                        if (mouse.button[MOUSE_L].bPressed)
                        {
                            //PickEntt(g_GameState.mousePosW, LAYER_TILEMAP);
                            GetEditor()->ModifyMapTile(g_GameState.mousePosW);
                        }
                        break;
                    }

                    case EDITOR_ENTITIES:   
                    {
                        // if we are moving or spawning an entity
                        if (IsKeyPressed(KEY_LALT) || IsKeyPressed(KEY_LCTRL))
                            break;

                        const eLayerType selectableLayer = GetEditor()->GetSelectableLayer();
                        const EntityID pickedId = PickEntt(g_GameState.mousePosW, selectableLayer);
                        GetEditor()->SetSelectedEntt(pickedId);
                        
                        break;
                    }

                    case EDITOR_PREFABS:    
                    {
                        // spawn an entity:
                        // 1. select a prefab
                        // 2. LCtrl + LMB onto the scene

                        if (!IsKeyPressed(KEY_LCTRL))
                            break;

                        // only when the scene viewport is hovered by mouse
                        if (!GetEditor()->IsSceneViewportHovered())
                            break;

                        // prevent creation of a new entity each frame
                        if (bEntityCreated)
                            break;

                        const Entity* pPrefab = GetEditor()->GetSelectedPrefab();

                        if (!pPrefab)
                            break;

                        Entity* pNewEntt = g_EnttMgr.Instantiate(pPrefab);

                        // setup initial position of entity
                        Transform* t = pNewEntt->GetComponent<Transform>();
                        const Vec2i      sz = t->GetWorldSize();
                        const Vec2i  offset = { sz.x >> 1, sz.y >> 1 };
                        const Vec2i initPos = Vec2iSub(g_GameState.mousePosW, offset);
                        t->SetPos(initPos);

                        if (pNewEntt->HasComponent<Sprite>())
                        {
                            // for animated sprites there can be glitch when the entity is invisible after creation
                            // so force updating its sprite to make it visible
                            Sprite* spr = pNewEntt->GetComponent<Sprite>();
                            spr->Update(1.0f);
                        }

                        bEntityCreated = true;      
                        LogDbg(LOG, "Entitity (%s) is created!", pNewEntt->GetName());

                        // set last created entity as active (selected)
                        GetEditor()->SetSelectedEntt(pNewEntt->GetId());

                        break;
                    }
                }
            }

            else if (e.mouse.type == MOUSE_LRELEASE)
            { 
                bEntityCreated = false;
            }
#if 0
            else if (e.mouse.type == MOUSE_MPRESS)
            { 
            }
            else if (e.mouse.type == MOUSE_MRELEASE)
            { 
            }
            else if (e.mouse.type == MOUSE_RPRESS)
            { 
            }
            else if (e.mouse.type == MOUSE_RRELEASE)
            { 
            }
            else if (e.mouse.type == MOUSE_WHEEL_UP)
            { 
            }
            else if (e.mouse.type == MOUSE_WHEEL_DOWN)
            { 
            }
#endif
        }

        else if (e.type == EVENT_KEYDOWN)
        {
            const bool bRepeat = e.keyboard.bRepeat;
            const eKeyCode key = e.keyboard.code;

            if (key == KEY_ESCAPE)
            {
                g_GameState.bRunning = ShowClosingEngineMessageBox();
            }
            // Ctrl + S: save the level
            else if (IsKeyPressed(KEY_LCTRL) && (key == KEY_S && !bRepeat))
            {
                SaveLevel();
            } 
            // Ctrl + D: duplicate the active (selected) entity
            else if (IsKeyPressed(KEY_LCTRL) && (key == KEY_D && !bRepeat))
            {
                if (!GetEditor()->IsAnyEnttSelected())
                    continue;

                // prevent copying of the player
                const EntityID id = GetEditor()->GetSelectedEnttId();
                if (id == g_EnttMgr.GetPlayerId())
                    continue;


                const Entity* pEnttSrc = g_EnttMgr.GetEntt(id);
                const Entity* pEntt = g_EnttMgr.Instantiate(pEnttSrc);
                g_GameState.bStickEnttToMouse = true;

                // set the new entity as picked
                GetEditor()->SetSelectedEntt(pEntt->GetId());
            }

            else if (key == KEY_G && !bRepeat)
            {
                // if any entity is selected, then we will move it with mouse
                if (GetEditor()->IsAnyEnttSelected())
                    g_GameState.bStickEnttToMouse = true;
            }

            else if (key == KEY_H && !bRepeat)
            {
                // hide the selected entity
                if (!IsKeyPressed(KEY_LALT))
                {
                    if (!GetEditor()->IsAnyEnttSelected())
                        continue;

                    if (hiddenEntts_[ARRAY_SIZE(hiddenEntts_) - 1] != 0)
                    {
                        LogErr(LOG, "can't hide on more entity (limit: %d)", ARRAY_SIZE(hiddenEntts_));
                        continue;
                    }

                    const EntityID id = GetEditor()->GetSelectedEnttId();
                    Entity* pEntt = g_EnttMgr.GetEntt(id);

                    Sprite* pSpr = pEntt->GetComponent<Sprite>();
                    if (pSpr)
                        pSpr->SetShow(false);

                    GetEditor()->SetSelectedEntt(0);
                    GetEditor()->SetShowEditorCursor(false);

                    // push this entity into the hidden entities list
                    for (size_t i = 0; i < ARRAY_SIZE(hiddenEntts_); ++i)
                    {
                        if (hiddenEntts_[i] == 0)
                        {
                            hiddenEntts_[i] = pEntt->GetId();
                            break;  // interrupt the for-loop
                        }
                    }
                }

                // show all the hidden entities
                else
                {
                    for (size_t i = 0; i < ARRAY_SIZE(hiddenEntts_); ++i)
                    {
                        if (hiddenEntts_[i] == 0)
                            break;  // interrupt the for-loop

                        Entity* pEntt = g_EnttMgr.GetEntt(hiddenEntts_[i]);
                        Sprite* pSpr = pEntt->GetComponent<Sprite>();
                        if (pSpr)
                            pSpr->SetShow(true);
                    }

                    memset(hiddenEntts_, 0, sizeof(hiddenEntts_));
                }
            }

            else if (key == KEY_F1 && !bRepeat)
            {
                ToggleGameMode();
            }
            else if (key == KEY_F2 && !bRepeat)
            {
                GetEditor()->ToggleCameraNoClip();
            }
            else if (key == KEY_DELETE && !bRepeat)
            {
                if (!GetEditor()->IsAnyEnttSelected())
                    continue;

                // remove currently selected entity
                const EntityID id = GetEditor()->GetSelectedEnttId();
                g_EnttMgr.SetToRemove(g_EnttMgr.GetEntt(id));

                GetEditor()->SetSelectedEntt(0);
            }
            // make the world visually smaller
            else if (IsKeyPressed(KEY_LALT))
            {
                // scene will be rendered 1,2,4,8 times smaller
                if      (key == KEY_1)  g_Render.SetDrawScale(1);
                else if (key == KEY_2)  g_Render.SetDrawScale(2);
                else if (key == KEY_3)  g_Render.SetDrawScale(4);
                else if (key == KEY_4)  g_Render.SetDrawScale(8);
            }
        }

#if 0
        else if (e.type == EVENT_KEYUP)
        {
        }
#endif
    }
}

//---------------------------------------------------------
// Desc:  change selection of entity on the scene (picking)
// Args:  pos    - position in world to test for picking
//        layer  - test entities which belongs only to this layer
// Ret:   ID of picked entity or 0 if no selection
//---------------------------------------------------------
EntityID Game::PickEntt(const Vec2i pos, const eLayerType layer)
{
    // picking a tile of the map (ground) isn't allowed
    if (layer == LAYER_TILEMAP)
        return INVALID_ENTT_ID;

    //
    // pick a scene object, UI element, etc.
    //
    Entity* pCursor = GetEditor()->GetCursor();
    Entity*    pFog = g_EnttMgr.GetEntt(fogEnttId_);

    assert(pCursor);
    assert(pFog);

    for (const Entity* pEntt : g_EnttMgr.GetEnttsByLayer(layer))
    {
        if (!pEntt)
            continue;

        // and prevent selection of some entities
        if (pEntt == pCursor || pEntt == pFog)
            continue;

        // prevent selection of the hidden entity
        if (hiddenEntts_[0] != 0)
        {
            bool bNonPickable = false;

            for (size_t i = 0; i < ARRAY_SIZE(hiddenEntts_); ++i)
                bNonPickable |= (hiddenEntts_[i] == pEntt->GetId());
            
            if (bNonPickable)
                continue;
        }

        // test picking
        const Transform* t = pEntt->GetComponent<Transform>();

        if (!CollisionRectPoint(t->GetRecti(), pos))
            continue;
        
        return pEntt->GetId();
    }

    return INVALID_ENTT_ID;
}

//---------------------------------------------------------
// updating when we're in the game mode
//---------------------------------------------------------
void Game::UpdateGame(const float dt)
{
    // update entities in the entity manager
    g_EnttMgr.Update(dt);

    HandleCameraMovement();

    // update info about player's position
    const Entity*   player = g_EnttMgr.GetEntt(playerId_);
    g_GameState.fPlayerPos = player->GetComponent<Transform>()->GetPos();
    g_GameState.iPlayerPos = ToVec2i(g_GameState.fPlayerPos);

    if (g_GameState.bShowFog && fogEnttId_ != INVALID_ENTT_ID)
        UpdateFog();

    pUiGame_->Update(dt);

    // execute game scripts stuff for each frame
    LuaCall(L, "OnUpdate");
}

//---------------------------------------------------------
// updating when we're in the editor mode
//---------------------------------------------------------
void Game::UpdateEditor(const float dt)
{
    // update entities in the entity manager
    g_EnttMgr.Update(dt);

    if (g_GameState.bShowFog && fogEnttId_ != INVALID_ENTT_ID)
        UpdateFog();

    HandleCameraMovement();

    // update position of the "editor cursor" (to follow entity that moves)
    if (GetEditor()->IsAnyEnttSelected())
    {
        const EntityID   id = GetEditor()->GetSelectedEnttId();
        const Entity* pEntt = g_EnttMgr.GetEntt(id);
        const Vec2      pos = pEntt->GetComponent<Transform>()->GetPos();
        GetEditor()->SetEditorCursorPos(pos);
    }
}

//---------------------------------------------------------
// update fog translation and position around the camera's center;
// we will have 4 fog rectangles so we have to calculated its positions
//---------------------------------------------------------
void Game::UpdateFog()
{
    // update a "camera rectangle" to be always around the player
    Entity*             pFog = g_EnttMgr.GetEntt(fogEnttId_);
    Entity*          pPlayer = g_EnttMgr.GetEntt(playerId_);

    Transform*     pFogTrans = pFog->GetComponent<Transform>();
    Transform*  pPlayerTrans = pPlayer->GetComponent<Transform>();

    const Recti      camRect = RectiInit(g_GameState.camPos, g_GameState.camSize);
    const Vec2    fPlayerPos = pPlayerTrans->GetCenter();

    // do hacky correction of the fog position if we just turned on the fog 
    if (!CollisionRectiRecti(pFogTrans->GetRecti(), pPlayerTrans->GetRecti()))
    {
        pFogTrans->SetPos(fPlayerPos);
    }

    const Vec2        fogPos = pFogTrans->GetPos();
    const Vec2         fogSz = ToVec2(pFogTrans->GetWorldSize());
    const Rect       fogRect = RectInit(fogPos, fogSz);

    Rect fogRects[NUM_FOG_RECTS] = { fogRect, fogRect, fogRect, fogRect };

    const bool bAppendRight = (camRect.x1 > fogRect.x1);
    const bool bAppendDown  = (camRect.y1 > fogRect.y1);

    // horizontal rectangle append
    fogRects[1].x0 += (bAppendRight) ? fogSz.w : -fogSz.w;
    fogRects[1].x1 += (bAppendRight) ? fogSz.w : -fogSz.w;
   
    // vertical rectangle append
    fogRects[2].y0 += (bAppendDown) ? fogSz.h : -fogSz.h;
    fogRects[2].y1 += (bAppendDown) ? fogSz.h : -fogSz.h;

    // corner rectangle append
    fogRects[3] = RectInit({ fogRects[1].x0, fogRects[2].y0}, fogSz);


    // calc fog base rectangle for the next frame
    for (int i = 0; i < NUM_FOG_RECTS; ++i)
    {
        if (CollisionRectPoint(fogRects[i], fPlayerPos))
        {
            const Vec2 pos = { fogRects[i].x0, fogRects[i].y0 };
            pFogTrans->SetPos(pos);
            i = NUM_FOG_RECTS;
        }
    }

    //
    // calc destination rectangles to render the fog into
    //

    // do calculations using integers to prevent gaps btw fog cells
    const Vec2i iFogPos   = ToVec2i({ floorf(fogPos.x), floorf(fogPos.y) });
    const Vec2i iFogSz    = pFogTrans->GetWorldSize();
    const Recti iBaseRect = RectiInit(iFogPos, iFogSz);

    Recti iDstRects[NUM_FOG_RECTS] = { iBaseRect, iBaseRect, iBaseRect, iBaseRect };
    
    // horizontal rectangle append
    iDstRects[1].x0 += (bAppendRight) ? iFogSz.w : -iFogSz.w;
    iDstRects[1].x1 += (bAppendRight) ? iFogSz.w : -iFogSz.w;

    // vertical rectangle append
    iDstRects[2].y0 += (bAppendDown) ? iFogSz.h : -iFogSz.h;
    iDstRects[2].y1 += (bAppendDown) ? iFogSz.h : -iFogSz.h;

    // corner rectangle append
    iDstRects[3] = RectiInit({ iDstRects[1].x0, iDstRects[2].y0 }, iFogSz);


    // calculate final position of fog on the screen
    for (int i = 0; i < NUM_FOG_RECTS; ++i)
    {
        fogDstRects_[i] = iDstRects[i];

        // adjust fog pos relatively to the camera
        fogDstRects_[i].x0 -= g_GameState.camPos.x;
        fogDstRects_[i].x1 -= g_GameState.camPos.x;
        fogDstRects_[i].y0 -= g_GameState.camPos.y;
        fogDstRects_[i].y1 -= g_GameState.camPos.y;
    }
}

//---------------------------------------------------------
// render all the input items onto the screen
//---------------------------------------------------------
void DrawRenderItems(const cvector<RenderItem>& items)
{
    TexID texId = -1;
    Texture* pTex = nullptr;

    // rotation params
    const Vec2i* pCenter = nullptr;
    const float angle = 0.0f;

    for (const RenderItem& it : items)
    {
        // select another texture if need
        if (texId != it.texId)
        {
            texId = it.texId;
            pTex = g_AssetMgr.GetTex(texId);
        }

        g_Render.DrawRect(pTex, it.srcRect, it.dstRect, it.color, it.flipMode, pCenter, angle);
    } 
}

//---------------------------------------------------------
// render a translucent fog over the main scene
//---------------------------------------------------------
void Game::RenderFogOverScene()
{
    if (fogEnttId_ == INVALID_ENTT_ID)
        return;

    Sprite* pSpr = g_EnttMgr.GetEntt(fogEnttId_)->GetComponent<Sprite>();

    Texture*     pTex    = g_AssetMgr.GetTex(pSpr->GetTex());
    const Recti  srcRect = pSpr->GetSrcRect();
    const Color  color   = pSpr->GetColor();
    const uint32 flip    = 0;
    const Vec2i* center  = NULL;
    const float  angle   = 0.0f;

    SetTextureBlendMode(pTex, BLENDMODE_BLEND);
    SetTextureAlphaMod(pTex, 255);

    // render 4 fog rectangles around the player
    for (int i = 0; i < NUM_FOG_RECTS; ++i)
    {
        g_Render.DrawRect(pTex, srcRect, fogDstRects_[i], color, flip, center, angle);
    }
}

//---------------------------------------------------------
// push entities' rendering data into the render lists
//---------------------------------------------------------
void Game::PushVisibleEnttsToRender()
{
    for (int i = 0; i < NUM_LAYERS; ++i)
    {
        const eLayerType layer = eLayerType(i);

        if (layer == LAYER_TILEMAP)
            for (Entity* pEntt : visEntts_[LAYER_TILEMAP].entts)
            {
                pEntt->GetComponent<Tile>()->Render();
            }

        else
            for (Entity* pEntt : visEntts_[layer].entts)
            {
                if (pEntt->HasComponent<Sprite>())
                    pEntt->GetComponent<Sprite>()->Render();
            }
    }
}

void Game::RenderScene()
{
    if (!g_EnttMgr.HasEntts())
        return;

    PushVisibleEnttsToRender();
    SortRenderListItemsByY(LAYER_ENTITY);
    SortRenderListItemsByZindex(LAYER_GND_DECAL);
    SortRenderListItemsByZindex(LAYER_UI);



    // render map tiles (ground/water by itself)
    DrawRenderItems(g_RenderLists[LAYER_TILEMAP].items);

    // render decals over the ground
    DrawRenderItems(g_RenderLists[LAYER_GND_DECAL].items);

    // render scene objects
    DrawRenderItems(g_RenderLists[LAYER_ENTITY].items);


    if (g_GameState.bShowFog)
        RenderFogOverScene();

    if (g_EnttMgr.IsShowLayer(LAYER_UI))
    {
        // push game's UI elements for rendering
        pUiGame_->RenderSceneUI();
    }

    // if we want to render editor's cursor (only for editor mode)
    if (!g_GameState.bGameMode && GetEditor()->IsShowEdCursor())
    {

        Entity* pCursor = GetEditor()->GetCursor();
        assert(pCursor && pCursor->HasComponent<Sprite>());
        pCursor->GetComponent<Sprite>()->Render();           // push cursor into the render list

        DrawRenderItems(g_RenderLists[LAYER_UI].items);
    }

    // render game UI elements
    DrawRenderItems(g_RenderLists[LAYER_UI].items);

}

//---------------------------------------------------------
// rendering the editor's panels (is rendered after the scene)
//---------------------------------------------------------
void Game::RenderEditor()
{
    imGuiLayer_.Begin();
    GetEditor()->Render(g_Render.GetRenderTarget());
    imGuiLayer_.End();
}

//---------------------------------------------------------
// update main camera position
//---------------------------------------------------------
void Game::HandleCameraMovement()
{
    GameState& state = g_GameState;
    const Mouse& mouse = GetMouseState();

    // free-fly mode (no clip)
    if (GetEditor()->IsCameraNoClip())
    {
        static Vec2i camPos = state.camPos;
        const MouseBtnState& mouseWheel = mouse.button[MOUSE_M];

        // move camera using pressing mouse wheel and moving mouse
        if (mouseWheel.bDown)
        {
            const Vec2i m = state.mousePos;
            const Vec2i w = { (int)mouseWheel.clickX, (int)mouseWheel.clickY };
            const Vec2i d = Vec2iSub(w, m);

            // update camera global pos
            state.camPos = Vec2iAdd(camPos, d);
        }
        else
        {
            camPos = state.camPos;
        }
    }

    // stick to the player
    else
    {
        Entity* pPlayer = g_EnttMgr.GetEntt(playerId_);
        assert(pPlayer && pPlayer->GetId() != INVALID_ENTT_ID);

        // calc camera top-left pos in world (its center match with player's center)
        const Transform* tr = pPlayer->GetComponent<Transform>();
        const Vec2i  wndMid = { state.wndSize.w >> 1, state.wndSize.h >> 1 };
        const Vec2i  center = ToVec2i(tr->GetCenter());
        Vec2i        camPos = Vec2iSub(center, wndMid);

        // clamp camera pos (so it sticks to the world's edges)
        const Vec2i camMinPos = state.camPadding;
        const Vec2i camMaxPos = {
            state.iWorldSize.w - state.camSize.w - state.camPadding.w,
            state.iWorldSize.h - state.camSize.h - state.camPadding.h,
        };
        
        // make to be within the world
        camPos = Vec2iClamp(camPos, camMinPos, camMaxPos);

        if (state.bUseSmoothCamMove)
        {
            // direction from old_pos => new_pos
            static Vec2 s_CamPos = ToVec2(state.camPos);
            const Vec2 v = ToVec2(Vec2iSub(camPos, state.camPos));

            s_CamPos.x += (v.x * state.camAsymptoticAvgWeight);
            s_CamPos.y += (v.y * state.camAsymptoticAvgWeight);
            state.camPos = ToVec2i(s_CamPos);
        }
        else
        {
            state.camPos = camPos;
        }


        if (state.bUseCamShaking)
        {
            state.camShake -= (state.camShakeInc * state.deltaTime);
            state.camShake = clampf(state.camShake, 0, 1);

            if (state.camShake < EPSILON_E5)
                return;

            const float shake = state.camShakeMaxOffset * SQR(state.camShake);

#if 0
            //
            // calc offset based on random values in range [0,1]
            //
            const float offsetX = shake * RandF(0, 1);
            const float offsetY = shake * RandF(0, 1);
#else
            //
            // calc offset based on Perlin Noise
            //
            const float x = g_GameState.gameTime * 200.0f;
            const float y = g_GameState.gameTime * 200.0f;

            Noise::PerlinConfig cfg;
            cfg.octaves = 16;
            cfg.scale = 5;
            cfg.seed = 0;
            const float offsetX = shake * Noise::PerlinFBM(x, y, cfg);

            cfg.seed++;
            const float offsetY = shake * Noise::PerlinFBM(x, y, cfg);
#endif

            //state.camAngle += angle;
            state.camPos.x += (int)offsetX;
            state.camPos.y += (int)offsetY;
        }
    }
}

//---------------------------------------------------------
// world size is dependent on the size of the map (has the same size)
//---------------------------------------------------------
void Game::UpdateWorldSize()
{
    if (!pMap_)
    {
        LogErr(LOG, "init the map first!");
        return;
    }
    g_GameState.iWorldSize = pMap_->GetMapSize();
    g_GameState.fWorldSize = ToVec2(g_GameState.iWorldSize);
}

//---------------------------------------------------------
// Desc:  load level's common data from file by filepath
//---------------------------------------------------------
void Game::LoadLevelMetadata(const char* filepath, char(*outMapPath)[128])
{
    assert(filepath && filepath[0] != '\0');
    assert(outMapPath && *outMapPath);

    int count = 0;
    char buf[128];
    FILE* fp = fopen(filepath, "r");
    if (!fp)
    {
        LogErr(LOG, "can't open level file: %s", filepath);
        return;
    }

    fgets(buf, sizeof(buf), fp);
    assert(strcmp(buf, "level {\n") == 0);

    // level's name
    char lvlName[64];
    count = fscanf(fp, "\tname: %s\n", lvlName);
    assert(count == 1);
    assert(strcmp(lvlName, levelName_) == 0);

    // level's map (path)
    count = fscanf(fp, "\tmap: %s", outMapPath);
    assert(count == 1);

    g_GameState.camPadding = FileReadInt2(fp, "camera_pad:");
    assert(g_GameState.camPadding.x >= 0 && g_GameState.camPadding.y >= 0);

    fclose(fp);
}

//---------------------------------------------------------
// sandbox is a basic empty level
//---------------------------------------------------------
void Game::LoadLevelSandbox(const GameInit& gameInit)
{
    ResetLevel();

    //
    // create an empty map
    //
    const Vec2i numMapCells = { 20, 20 };
    assert(pMap_ == nullptr);
    pMap_ = new Map();
    pMap_->CreateNewMap(levelName_, numMapCells.x, numMapCells.y);
    UpdateWorldSize();

    CreatePlayer();
    CreateFog();

    Entity* pPlayer = g_EnttMgr.GetEntt("player");
    assert(pPlayer);
    playerId_ = pPlayer->GetId();

    UpdateEditor(1.0f);
    CreatePrefabs();

    //
    // load fonts
    //
    TexID dbgFontTexId  = g_AssetMgr.GetTexId(gameInit.dbgFontName);
    TexID gameFontTexId = g_AssetMgr.GetTexId(gameInit.gameFontName);
    
    g_Text.InitFont(FONT_TYPE_DEBUG, dbgFontTexId,  gameInit.dbgFontDataPath);
    g_Text.InitFont(FONT_TYPE_GAME,  gameFontTexId, gameInit.gameFontDataPath);
    g_Text.SetFontType(FONT_TYPE_GAME);

    // by default hide editor's cursor (a red square) 
    GetEditor()->SetShowEditorCursor(false);
}

//---------------------------------------------------------
// create a fog (we will render it separately after the main scene)
//---------------------------------------------------------
void Game::CreateFog()
{
    const TexID texId = g_AssetMgr.LoadTexture("fog", "assets/images/common/fog_noise.png");
    const Vec2  pos = { 0, 0 };
    const Vec2  vel = { -30, 10 };
    const Vec2i size{ 2048, 2048 };
    const int   scale = 1;
    const bool  bFixedOnScreen = false;

    Entity* pFog = g_EnttMgr.AddEntt("fog", LAYER_ENTITY);
    assert(pFog->GetId() != INVALID_ENTT_ID);

    fogEnttId_ = pFog->GetId();

                  pFog->AddComponent<Movement>(vel.x, vel.y);
                  pFog->AddComponent<Transform>(pos, size, scale);
    Sprite& spr = pFog->AddComponent<Sprite>(texId, bFixedOnScreen);

    spr.SetColor(255, 255, 255, 100);
    spr.SetShow(false);
}

//---------------------------------------------------------
// cleanup and some init before switching the level (or creation a new one)
//---------------------------------------------------------
void Game::ResetLevel()
{
    // before level loading we release all the prev data
    if (pMap_)
    {
        pMap_->ReleaseMap();
        SafeDelete(pMap_);
    }

    // clear all the cached data of the game
    playerId_ = 0;
    fogEnttId_ = 0;
    circleCollidersIds_.clear();
    circleColliders_.clear();
    boxCollidersIds_.clear();
    boxColliders_.clear();
    visEnemies_.clear();
    visTriggers_.clear();

    // reinit default assets (the "dummy" texture, animation, etc.)
    g_AssetMgr.ClearData();
    g_AssetMgr.Init();

    g_EnttMgr.ClearData();
    g_EnttMgr.CreateDummyStuff();
    eventMgr_.ClearEventsQueue();


    GetEditor()->SetSelectedEntt(0);
}

//---------------------------------------------------------
// manually generate prefabs
//---------------------------------------------------------
void Game::CreatePrefabs()
{
    CreateAllPrefabs();

    CreatePrefabTeleport(eventMgr_.GetEventIdByName("teleport_dungeon"), "dungeon");
    CreatePrefabTeleport(eventMgr_.GetEventIdByName("teleport_meadows"), "meadows");
    CreatePrefabTeleport(eventMgr_.GetEventIdByName("teleport_graveyard"), "graveyard");
}

//---------------------------------------------------------
// save some common data related to this level
//---------------------------------------------------------
void Game::SaveLevelMetadata(const char* filepath)
{
    assert(filepath && filepath[0] != '\0');

    char mapPath[128];
    snprintf(mapPath, sizeof(mapPath), "%s%s.map", pathToMapsDir, GetMap()->GetMapName());

    FILE* fp = fopen(filepath, "w");
    if (!fp)
    {
        LogErr(LOG, "can't open file for level metadata saving: %s", filepath);
        return;
    }

    fprintf(fp, "level {\n");
    fprintf(fp, "\tname:       %s\n", levelName_);
    fprintf(fp, "\tmap:        %s\n", mapPath);
    fprintf(fp, "\tcamera_pad: %d %d\n", g_GameState.camPadding.x, g_GameState.camPadding.y);
    fprintf(fp, "}\n");

    fclose(fp);
}

//---------------------------------------------------------
// switch from the editor to game mode
//---------------------------------------------------------
void Game::ToggleGameMode()
{
    LogDbg(LOG, "switch to game mode");
    GetEditor()->SetShowEditorCursor(false);
    g_Render.SetFullscreen(true);
    g_Render.ShowSysCursor(false);

    g_GameState.camSize = g_Render.GetWndSize();
    g_GameState.wndSize = g_Render.GetWndSize();
    g_GameState.bGameMode = !g_GameState.bGameMode;
    g_GameState.viewportTopLeft = { 0,0 };
}

//---------------------------------------------------------
// switch from the game to editor mode
//---------------------------------------------------------
void Game::ToggleEditorMode()
{
    LogDbg(LOG, "switch to editor mode");
    GetEditor()->SetShowEditorCursor(true);
    g_Render.SetFullscreen(false);
    g_Render.ShowSysCursor(true);

    g_GameState.camSize = g_Render.GetWndSize();
    g_GameState.wndSize = g_Render.GetWndSize();
    g_GameState.bGameMode = !g_GameState.bGameMode;
}

//---------------------------------------------------------
// turn on/off rendering of the console
//---------------------------------------------------------
void Game::ToggleConsole()
{
    g_GameState.bUseConsole = !g_GameState.bUseConsole;
}


//---------------------------------------------------------

void CreatePlayerAnimations()
{
    const TexID idleTexId   = g_AssetMgr.LoadTexture("knight_idle",   "assets/images/knight/idle.png");
    const TexID moveTexId   = g_AssetMgr.LoadTexture("knight_move",   "assets/images/knight/walk.png");
    const TexID attackTexId = g_AssetMgr.LoadTexture("knight_attack", "assets/images/knight/attack.png");
    const TexID deathTexId  = g_AssetMgr.LoadTexture("knight_death",  "assets/images/knight/death.png");
    const TexID damageTexId = g_AssetMgr.LoadTexture("knight_damage", "assets/images/knight/damage.png");
    const TexID blockTexId  = g_AssetMgr.LoadTexture("knight_block",  "assets/images/knight/block.png");

    const uint16 idleAnimSpeed   = 400;
    const uint16 idleNumFrames   = 4;
    const uint16 moveAnimSpeed   = 100;
    const uint16 moveNumFrames   = 4;
    const uint16 attackAnimSpeed = 50;
    const uint16 attackNumFrames = 8;
    const uint16 deathAnimSpeed  = 150;
    const uint16 deathNumFrames  = 11;
    const uint16 damageAnimSpeed = 100;
    const uint16 damageNumFrames = 3;
    const uint16 blockAnimSpeed  = 100;
    const uint16 blockNumFrames  = 3;
    const bool bLooped = true;

    if (!g_AssetMgr.HasAnimByName("player_idle"))
        g_AssetMgr.AddAnim("player_idle",   idleTexId,   idleAnimSpeed,   idleNumFrames,   bLooped);
    
    if (!g_AssetMgr.HasAnimByName("player_move"))
        g_AssetMgr.AddAnim("player_move",   moveTexId,   moveAnimSpeed,   moveNumFrames,   bLooped);
    
    if (!g_AssetMgr.HasAnimByName("player_attack"))
        g_AssetMgr.AddAnim("player_attack", attackTexId, attackAnimSpeed, attackNumFrames, bLooped);
        
    if (!g_AssetMgr.HasAnimByName("player_death"))
        g_AssetMgr.AddAnim("player_death",  deathTexId,  deathAnimSpeed,  deathNumFrames,  !bLooped);

    if (!g_AssetMgr.HasAnimByName("player_damage"))
        g_AssetMgr.AddAnim("player_damage", damageTexId, damageAnimSpeed, damageNumFrames, bLooped);

    if (!g_AssetMgr.HasAnimByName("player_block"))
        g_AssetMgr.AddAnim("player_block",  blockTexId,  blockAnimSpeed,  blockNumFrames,  bLooped);
}

//---------------------------------------------------------
// create and setup an entity of the player
//---------------------------------------------------------
void Game::CreatePlayer()
{
    //CreatePlayerAnimations();
    const Animation& attackAnim = g_AssetMgr.GetAnim("player_attack");

    // transformation data
    const Vec2 pos = { 0, 0 };
    const Vec2i sz = { 32, 32 };
    const int scale = 7;
 
    // collider data (circle)
    const Vec2i colliderCenter = { 107, 200 };
    const int colliderRadius = 21;

    // health / damage
    const int   currHP = 100;
    const int   maxHP = 100;
    const int   healPerSec = 1;
    const int   attackDurationMs = (attackAnim.speed * attackAnim.numFrames);
    const float attackCooldownSec = (float)attackDurationMs * 0.001f;
    const float damage = 10.0f;

    // player params
    const float  playerSpeed = 300.0f;
    const WeaponID currWpnId = g_AssetMgr.GetWpnIdByName("wpn_sword");
    const Weapon*    pWeapon = g_AssetMgr.GetWpnById(currWpnId);

    //
    // create and setup the main player
    //
    Entity* pPlayer = g_EnttMgr.AddEntt("player", LAYER_ENTITY);
    assert(pPlayer->GetId() != INVALID_ENTT_ID);

    playerId_ = pPlayer->GetId();

    pPlayer->AddComponent<Transform>(pos, sz, scale);
    pPlayer->AddComponent<Keyboard>("w", "a", "s", "d");
    pPlayer->AddComponent<Player>(playerSpeed, currWpnId);
    pPlayer->AddComponent<Damage>(pWeapon->cooldownSec, pWeapon->damage);
    pPlayer->AddComponent<Health>(currHP, maxHP, healPerSec);
    pPlayer->AddComponent<Collider>(colliderCenter, colliderRadius);

    // setup a player's sprite (with multiple directions)
    const bool bFixedOnScreen = false;
    
    Sprite& spr = pPlayer->AddComponent<Sprite>(
        g_AssetMgr.GetAnim("player_idle"),
        g_AssetMgr.GetAnim("player_move"),
        attackAnim,
        g_AssetMgr.GetAnim("player_death"),
        g_AssetMgr.GetAnim("player_damage"),
        g_AssetMgr.GetAnim("player_block"),
        bFixedOnScreen);

    spr.SetAnimDir(ANIM_DOWN);
    spr.SetSrcRectSize({ 96,96 });
}

//---------------------------------------------------------
// gather all the visible triggers
//---------------------------------------------------------
void Game::GatherVisibleTriggers()
{
    // gather visible triggers
    visTriggers_.clear();

    for (Entity* pEntt : visEntts_[LAYER_ENTITY].entts)
    {
        if (pEntt->HasComponent<Trigger>())
            visTriggers_.push_back(pEntt->GetId());
    }
}

//---------------------------------------------------------
// trigger the visible and intersected triggers
//---------------------------------------------------------
void Game::TriggerVisibleTriggers()
{
    Entity* pPlayer = g_EnttMgr.GetEntt(playerId_);
    Collider* pPlayerCollider = pPlayer->GetComponent<Collider>();

    // iterate through all the visible triggers
    for (EntityID trigId : visTriggers_)
    {
        Entity* pTrigEntt = g_EnttMgr.GetEntt(trigId);
        assert(pTrigEntt->HasComponent<Trigger>());
        assert(pTrigEntt->HasComponent<Collider>());
        
        Trigger* pTrig = pTrigEntt->GetComponent<Trigger>();
        eTriggerType  type = pTrig->GetType();
        eTriggerState state = pTrig->GetState();

        // check collision against the player
        const bool bCollide = IsCollide(pTrigEntt->GetComponent<Collider>(), pPlayerCollider);

        // set that this trigger is triggered by the entity
        EventData eventData;
        memset(&eventData, 0, sizeof(eventData));
        eventData.triggeredBy = playerId_;
        eventData.enttId      = trigId;              // owner or this trigger

        // switch states of the trigger
        switch (state)
        {
            case TRIGGER_INACTIVE:     
            {
                if (bCollide)
                {
                    EventMgr::PostEvent(pTrig->GetOnEnter(), &eventData);
                    pTrig->SetState(TRIGGER_ON_ENTER);
                }
                break;
            }

            case TRIGGER_ON_ENTER:     
            {
                if (bCollide)
                {
                    pTrig->SetState(TRIGGER_ON_COLLIDE);
                }
                else
                {
                    pTrig->SetState(TRIGGER_ON_LEAVE);
                }
                break;
            }

            case TRIGGER_ON_COLLIDE:  
            {
                if (bCollide)
                {
                    assert(pTrig->GetType() != TRIGGER_ONCE);

                    EventMgr::PostEvent(pTrig->GetOnCollide(), &eventData);
                }
                else
                {
                    pTrig->SetState(TRIGGER_ON_LEAVE);
                }
                break;
            }

            case TRIGGER_ON_LEAVE:     
            {
                EventMgr::PostEvent(pTrig->GetOnLeave(), &eventData);
                pTrig->SetState(TRIGGER_INACTIVE);
                break;
            }
        }
    }
}