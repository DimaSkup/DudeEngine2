/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: event_mgr.cpp
    Desc:     a manager for events;
              here we register events, bind listeners to them, and trigger events

    Created:  26.05.2026 by DimaSkup
\**********************************************************************************/
#include "event_mgr.h"
#include "log.h"
#include "lua_stuff.h"
#include "game.h"
#include <assert.h>

#define ENABLE_CPP_EVENT_LISTENERS 0


//---------------------------------------------------------
// static stuff
//---------------------------------------------------------
Event EventMgr::s_EventsQueue_[EventMgr::MAX_EVENTS];
static int head = 0;    // a head of the queue (ring buffer)
static int tail = 0;    // a tail of the queue (ring buffer)


//=============================================================================
// GAME EVENTS FUNCTIONAL: ADD, REMOVE, TRIGGER LISTENERS; POST (SEND) EVENTS
//=============================================================================

//---------------------------------------------------------
// default constructor
//---------------------------------------------------------
GameEvent::GameEvent()
{
    memset(listeners_, 0, sizeof(listeners_));
}

//---------------------------------------------------------
// Desc:  bind a listener (a Lua-function) by input name
//---------------------------------------------------------
void GameEvent::AddListener(const char* funcName)
{
    if (!funcName || funcName[0] == '\0')
    {
        LogErr(LOG, ERR_EMPTY_NAME);
        return;
    }

    if (numListeners_ >= MAX_NUM_LISTENERS)
    {
        LogErr(LOG, "can't bind one more listener for event (not enought slots, max: %d)", MAX_NUM_LISTENERS);
        return;
    }

    if (strlen(funcName) >= MAX_LEN_EVENT_LISTENER_NAME)
    {
        LogErr(LOG, "input func name is too long (max: %d)", MAX_LEN_EVENT_LISTENER_NAME);
        return;
    }

    strcpy(listeners_[numListeners_].name, funcName);
    numListeners_++;
}

//---------------------------------------------------------
// Desc:  unbind a listener (a Lua-function) by input name
//---------------------------------------------------------
void GameEvent::RemoveListener(const char* funcName)
{
    if (!funcName || funcName[0] == '\0')
    {
        LogErr(LOG, ERR_EMPTY_NAME);
        return;
    }

    for (int i = 0; i < numListeners_; ++i)
    {
        if (strcmp(listeners_[i].name, funcName) == 0)
        {
            // swap & pop
            listeners_[i] = listeners_[numListeners_ - 1];
            numListeners_--;
            return;
        }
    }
}

#if ENABLE_CPP_EVENT_LISTENERS
//---------------------------------------------------------
// Desc:  bind a listener for this event
//---------------------------------------------------------
void GameEvent::AddListener(EventListenerFunc funcPtr)
{
    if (!funcPtr)
    {
        LogErr(LOG, "input ptr to func == NULL");
        return;
    }

    if (numListeners_ >= MAX_NUM_LISTENERS)
    {
        LogErr(LOG, "can't bind one more listener for event (not enought slots, max: %d)", MAX_NUM_LISTENERS);
        return;
    }

    listeners_[numListeners_++] = funcPtr;
}

//---------------------------------------------------------
// Desc:  unbind a listener from this event
//---------------------------------------------------------
void GameEvent::RemoveListener(EventListenerFunc funcPtr)
{
    for (int i = 0; i < numListeners_; ++i)
    {
        if (listeners_[i] == funcPtr)
        {
            // swap & pop
            listeners_[i] = listeners_[numListeners_ - 1];
            numListeners_--;
            return;
        }
    }
}
#endif  // ENABLE_CPP_EVENT_LISTENERS

//---------------------------------------------------------
// Desc:  this game event has happened so notify (call) all its listeners
// Args:  pData - data for listeners (can be NULL if listeners don't need any data)
//---------------------------------------------------------
void GameEvent::Notify(Game* pGame, const EventData* pData)
{
#if ENABLE_CPP_EVENT_LISTENERS
    for (int i = 0; i < numListeners_; ++i)
    {
        assert(listeners_[i]);
        listeners_[i](pGame, pData);
    }
#endif

    // call all the handlers of this event from the Lua's side
    lua_State* L = pGame->GetLuaState();

    for (int i = 0; i < numListeners_; ++i)
        LuaCall(L, listeners_[i].name);
}

//=============================================================================
// EVENTS MANAGER IMPLEMENTATION
//=============================================================================

//---------------------------------------------------------
// constructor
//---------------------------------------------------------
EventMgr::EventMgr()
{
    // register a "dummy" event (so it will have ID == 0)
    Subscribe("dummy", "DummyEventListener");
}

//---------------------------------------------------------
// Desc:  get an index to game event data by input name
//---------------------------------------------------------
index EventMgr::GetIdxByName(const char* eventName) const
{
    if (!eventName || eventName[0] == '\0')
    {
        LogErr(LOG, ERR_EMPTY_NAME);
        return 0;
    }

    // start from 1 to skip a "dummy" event by index 0
    for (index i = 1; i < ids_.size(); ++i)
    {
        if (strcmp(eventNames_[i].name, eventName) == 0)
            return i;
    }

    return 0;
}

//---------------------------------------------------------
// Desc:  get an index to game event data by input id
//---------------------------------------------------------
index EventMgr::GetIdxById(const EventID id) const
{
    const index idx = ids_.get_idx(id);
    return (ids_.is_valid_index(idx)) ? idx : 0;
}

//---------------------------------------------------------
// Desc:  return an Id of event by input name, if there is
//        no event by this name we return an ID of "dummy" event
//---------------------------------------------------------
EventID EventMgr::GetEventIdByName(const char* eventName) const
{
    return ids_[GetIdxByName(eventName)];
}

//---------------------------------------------------------
// Desc:  return a name of event by input ID, if there is 
//        no event by this id we return a name of "dummy" event
//---------------------------------------------------------
const char* EventMgr::GetEventNameById(const EventID id) const
{
    return eventNames_[GetIdxById(id)].name;
}

//---------------------------------------------------------
// Desc:  1. if there is no event by input name yet, we create it
//        2. bind a listener to this event (its a name of Lua-function)
// 
// Ret:   identifier of event
//---------------------------------------------------------
EventID EventMgr::Subscribe(const char* eventName, const char* luaListenerName) 
{
    assert(eventName && eventName[0] != '\0');
    assert(strlen(eventName) < MAX_LEN_EVENT_NAME && "event name is too long");

    GameEvent* pEvent = nullptr;
    EventID eventId = INVALID_EVENT_ID;
    index idx = GetIdxByName(eventName);

    // if we currently have no event by input name...
    if (idx == 0)
    {
        // create a new event
        eventId = eventId_++;
        ids_.push_back(eventId);
        events_.push_back(GameEvent());
        pEvent = &events_.back();

        // setup a name
        eventNames_.push_back(EventName());
        char* name = eventNames_.back().name;
        strcpy(name, eventName);
    }
    else
    {
        eventId = ids_[idx];
        pEvent = &events_[idx];
    }

    assert(pEvent);
    pEvent->AddListener(luaListenerName);
    return eventId;
}

//---------------------------------------------------------
// Desc:   1. unbind a listener (Lua-function) from event by name
//         2. if the event has 0 listeners after it - we remove it
//---------------------------------------------------------
void EventMgr::Unsubscribe(const char* eventName, const char* luaListenerName) 
{
    assert(eventName && eventName[0] != '\0');

    const index idx = GetIdxByName(eventName);
    if (idx == 0)
    {
        LogErr(LOG, "no event by name: %s", eventName);
        return;
    }

    events_[idx].RemoveListener(luaListenerName);

    // remove this event if no listeners left
    if (events_[idx].GetNumListeners() == 0)
    {
        eventNames_.erase(idx);
        ids_.erase(idx);
        events_.erase(idx);
    }
}

#if ENABLE_CPP_EVENT_LISTENERS

//---------------------------------------------------------
// Desc:  1. if there is no event by input name yet, we create it
//        2. bind a listener to this event
// 
// Ret:   identifier of event
//---------------------------------------------------------
EventID EventMgr::Subscribe(const char* eventName, EventListenerFunc listener)
{
    assert(eventName && eventName[0] != '\0');
    assert(strlen(eventName) < MAX_LEN_EVENT_NAME && "your event name is too long");
    assert(listener);


    GameEvent* pEvent = nullptr;
    EventID eventId = INVALID_EVENT_ID;
    index idx = GetIdxByName(eventName);

    // if we currently have no event by input name...
    if (idx == 0)
    {
        // create a new event
        eventId = eventId_++;
        ids_.push_back(eventId);
        events_.push_back(GameEvent());
        pEvent = &events_.back();

        // setup a name
        eventNames_.push_back(EventName());
        char* name = eventNames_.back().name;
        strcpy(name, eventName);
    }
    else
    {
        eventId = ids_[idx];
        pEvent = &events_[idx];
    }

    assert(pEvent);
    pEvent->AddListener(listener);
    return eventId;
}


//---------------------------------------------------------
// Desc:   1. unbind a listener from event by name
//         2. if the event has 0 listeners after it - we remove it
//---------------------------------------------------------
void EventMgr::Unsubscribe(const char* eventName, EventListenerFunc listener)
{
    assert(eventName && eventName[0] != '\0');
    assert(listener);

    const index idx = GetIdxByName(eventName);
    if (idx == 0)
    {
        LogErr(LOG, "no event by name: %s", eventName);
        return;
    }

    events_[idx].RemoveListener(listener);

    // remove this event if no listeners left
    if (events_[idx].GetNumListeners() == 0)
    {
        eventNames_.erase(idx);
        ids_.erase(idx);
        events_.erase(idx);
    }
}

#endif  // ENABLE_CPP_EVENT_LISTENERS

//---------------------------------------------------------
// Desc:  trigger an event by id (notify all its listeners)
//---------------------------------------------------------
void EventMgr::TriggerEvent(const EventID id, Game* pGame, const EventData* pData)
{
    const index idx = GetIdxById(id);
    pEventData_ = pData;                // store a ptr to the data so we will be able to get it from the Lua side
    events_[idx].Notify(pGame, pData);
}

//---------------------------------------------------------
// Desc:  get the next event (if we have any) from the events queue
// Ret:   nullptr if there are no events left
//---------------------------------------------------------
const Event* EventMgr::GetNextEvent()
{
    // if there are no pending events, do nothing
    if (head == tail)
        return nullptr;

    Event* pEvent = &s_EventsQueue_[head];
    head = (head + 1) % MAX_EVENTS;

    return pEvent;
}

//---------------------------------------------------------
// push a new event into the events queue (so later we will handle them)
// NOTE:  pData - can be NULL (if related event doesn't require any data)
//---------------------------------------------------------
void EventMgr::PostEvent(const EventID eventId, const EventData* pData)
{
    assert((tail + 1) % MAX_EVENTS != head);

    // add to the end of the list
    Event& e = s_EventsQueue_[tail];
    tail = (tail + 1) % MAX_EVENTS;

    memset(&e, 0, sizeof(e));
    e.id = eventId;

    if (pData)
        memcpy(&e.data, pData, sizeof(EventData));
}

//---------------------------------------------------------
// push a new event into the events queue (so later we will handle them)
//---------------------------------------------------------
void EventMgr::PostEvent(const char* eventName, const EventData* pData)
{
    assert(eventName && eventName[0] != '\0');
    assert(strlen(eventName) < MAX_LEN_EVENT_NAME);
    assert((tail + 1) % MAX_EVENTS != head);

    // add to the end of the list
    Event& e = s_EventsQueue_[tail];
    tail = (tail + 1) % MAX_EVENTS;

    memset(&e, 0, sizeof(e));
    strcpy(e.name, eventName);

    if (pData)
        memcpy(&e.data, pData, sizeof(EventData));
}

//---------------------------------------------------------
// clear all the not handled events from the queue 
// (we do it, for instance, when load another game level)
//---------------------------------------------------------
void EventMgr::ClearEventsQueue()
{
    head = 0;
    tail = 0;
    memset(&s_EventsQueue_, 0, sizeof(Event) * MAX_EVENTS);
} 

//---------------------------------------------------------
// return an array of names of all the events
//---------------------------------------------------------
const cvector<EventName>& EventMgr::GetEventsNames(void) const 
{
    return eventNames_;
}

//---------------------------------------------------------
// return an array of ids of all the events
//---------------------------------------------------------
const cvector<EventID>& EventMgr::GetEventsIds(void) const 
{
    return ids_;
}