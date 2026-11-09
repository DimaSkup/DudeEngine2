#include "quest_sys.h"
#include "game.h"
#include "game_state.h"

inline void ResetQuest(Quest* pQuest)
{
    assert(pQuest);
    memset(pQuest, 0, sizeof(Quest));
}

void InitQuestKillBats(Quest* pQuest)
{
    assert(pQuest);
    ResetQuest(pQuest);

    Game*     pGame = (Game*)g_GameState.pGame;
    EventMgr* pEventMgr = pGame->GetEventMgr();
    Objective* pObj = nullptr;
    EventID eventIdArriveTo = pEventMgr->GetEventIdByName("REACH_LOCATION");
    EventID eventIdEnttDie  = pEventMgr->GetEventIdByName("ENTITY_DIED");

    //
    // setup the quest
    //
    pQuest->activeEventId = eventIdArriveTo;
    pQuest->state = QUEST_ACTIVE;
    strcpy(pQuest->nameKey, "$quest_name_kill_bats");
    pQuest->objectives.push_back(Objective());      // 1. arrive to graveyard
    pQuest->objectives.push_back(Objective());      // 2. kill bats

    //
    // setup objectives of the quest:
    //

    // 1. come to graveyard
    pObj = &pQuest->objectives[0];
    pObj->state = OBJECTIVE_ACTIVE;     // the first objective of a quest is active by default
    pObj->type = OBJECTIVE_REACH_LOCATION;
    pObj->eventId = eventIdArriveTo;
    strcpy(pObj->captionKey, "$quest0_obj0_caption");
    strcpy(pObj->descKey,    "$quest0_obj0_desc");

    // 2. kill bats
    pObj = &pQuest->objectives[1];
    pObj->state = OBJECTIVE_INACTIVE;
    pObj->type = OBJECTIVE_KILL;
    pObj->eventId = eventIdEnttDie;
    strcpy(pObj->captionKey, "$quest0_obj1_caption");
    strcpy(pObj->descKey,    "$quest0_obj1_desc");
}

void InitQuestKillSkeletons(Quest* pQuest)
{
    assert(pQuest);
    ResetQuest(pQuest);
}