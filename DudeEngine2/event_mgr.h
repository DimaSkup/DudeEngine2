/**********************************************************************************\

    ******    **    **  ******    ********   
    **    **  **    **  **    **  **      
    **    **  **    **  **    **  **    
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **    
    ******     ******   ******    ********

    Filename: event_mgr.h
    Desc:     a manager for events;
              here we register events, bind listeners to them, and trigger events

    Created:  26.05.2026 by DimaSkup
\**********************************************************************************/
#ifndef EVENT_MGR_H
#define EVENT_MGR_H

#include "types.h"
#include "cvector.h"


// forward declaration (pointer use only)
class Game;

// typedef for event listeners functions
typedef void (*EventListenerFunc)(Game* pGame, const EventData* pData);

struct Event
{
    EventID   id;
    char      name[MAX_LEN_EVENT_NAME];
    EventData data;
};

//---------------------------------------------------------
// class name:  GameEvent
// desc:        a single event which has its own set of listeners (handlers)
//---------------------------------------------------------
class GameEvent
{
private:
    struct FuncName
    {
        char name[MAX_LEN_EVENT_LISTENER_NAME];
    };

private:
    static const int MAX_NUM_LISTENERS = 4;

    // names of Lua-functions which are listening to this event
    FuncName listeners_[MAX_NUM_LISTENERS];
    int numListeners_ = 0;

    // pointers to C++ functions which are listening to this event
    //EventListenerFunc listeners_[MAX_NUM_LISTENERS] = { nullptr };
    //int numListeners_ = 0;

public:
    GameEvent();

    void AddListener    (const char* funcName);
    void RemoveListener (const char* funcName);

    //void AddListener    (EventListenerFunc funcPtr);
    //void RemoveListener (EventListenerFunc funcPtr);
    void Notify         (Game* pGame, const EventData* pData);

    inline int GetNumListeners() const { return numListeners_; }
};

//---------------------------------------------------------
// class name:  EventMgr
// desc:        storage for events; functional for events registration,
//              binding listeners to them and posting events
//---------------------------------------------------------
class EventMgr
{
private:
    cvector<EventName>  eventNames_;        // names of registered events
    cvector<EventID>    ids_;               // ids of registered events
    cvector<GameEvent>  events_;            // registered events
    const EventData*    pEventData_;        // a ptr to data of currently handled event (or nullptr if no data for event is passed)
    EventID             eventId_ = 0;

    // events queue (a ring buffer)
    static const int MAX_EVENTS = 16;
    static Event s_EventsQueue_[MAX_EVENTS];

public:
    EventMgr();

    const Event* GetNextEvent();
    static void  PostEvent(const EventID eventId, const EventData* pData);
    static void  PostEvent(const char* eventName, const EventData* pData);

    EventID      GetEventIdByName(const char* eventName) const;
    const char*  GetEventNameById(const EventID id)      const;
                 
    //EventID      Subscribe  (const char* eventName, EventListenerFunc listener);
    //void         Unsubscribe(const char* eventName, EventListenerFunc listener);

    EventID      Subscribe  (const char* eventName, const char* luaListenerName);
    void         Unsubscribe(const char* eventName, const char* luaListenerName);
                 
    void         TriggerEvent(const EventID id, Game* pGame, const EventData* pData);

    void         ClearEventsQueue();

    const cvector<EventName>& GetEventsNames(void) const;
    const cvector<EventID>&   GetEventsIds(void)   const;
    const EventData*          GetEventData(void)   const { return pEventData_; }

private:
    index GetIdxByName(const char* eventName) const;
    index GetIdxById  (const EventID id) const;
};

#endif