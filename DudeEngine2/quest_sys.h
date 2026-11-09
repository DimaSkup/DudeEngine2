/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: quest_sys.h
    Desc:     a system for quests, here we create and setup quests

    Created:  07.09.2026 by DimaSkup
\**********************************************************************************/
#ifndef QUEST_SYS_H
#define QUEST_SYS_H

#include "types.h"
#include "cvector.h"


enum eQuestState
{
    QUEST_INACTIVE,     // available quest: you can take it somewhere in game world
    QUEST_ACTIVE,
    QUEST_COMPLETED,
    QUEST_FAILED,
};

enum eObjectiveState
{
    OBJECTIVE_INACTIVE,
    OBJECTIVE_ACTIVE,
    OBJECTIVE_COMPLETED,
    OBJECTIVE_FAILED,
};

enum eObjectiveType
{    
    OBJECTIVE_KILL,             // you need to kill some entity
    OBJECTIVE_REACH_LOCATION,   // you need to come to some trigger
    OBJECTIVE_TALK,             // you need to talk to some entity
    OBJECTIVE_FIND,             // you need to gather/find some amount of entities
};

// quest may have multiple objectives
struct Objective
{
    eObjectiveState state;
    eObjectiveType  type;
    char            captionKey[32];     // a key to localized caption of this objective
    char            descKey[32];        // a key to locatized description of this objective
    EventID         eventId;            // listen to this event
};

struct Quest
{
    char                nameKey[32];        // a key to localized name of this quest
    uint32              id;                 // id of this quest
    cvector<Objective>  objectives;
    eQuestState         state;
    int                 activeObjectiveIdx;
    EventID             activeEventId;
};



//-----------------------------------------------------------------------------
// manually initialize quests
//-----------------------------------------------------------------------------
void InitQuestKillBats     (Quest* pQuest);
void InitQuestKillSkeletons(Quest* pQuest);

#endif