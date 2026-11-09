/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   input.cpp
    Desc:       a wrapper for different input backends (SDL, LDL, etc.)
    Created:    07.06.2026 by DimaSkup
\**********************************************************************************/
#include "input.h"
#include "defines.h"
#include <assert.h>

#if USE_SDL2
    #include "backends/SDL/sdl_input.cpp"
#elif USE_LDL
    #include "backends/LDL/ldl_input.cpp"
#else
    #error "No implementation selected"
#endif


//-------------------------------------
// queue of input events in the internal representation, and arr of states of each key
//-------------------------------------
static InputEvent s_EventQueue[16];
static int s_NumEvents;
static uint8 s_KeyStates[256];
static Mouse s_MouseState;


//---------------------------------------------------------
// initialize input stuff
//---------------------------------------------------------
void InputInit()
{
    memset(s_KeyStates, 0, sizeof(s_KeyStates));
    memset(&s_MouseState, 0, sizeof(s_MouseState));
}

//---------------------------------------------------------
// called each frame;
// convert all the input events into internal representation and push them into queue
//---------------------------------------------------------
void InputProcess()
{
    //
    // update mouse buttons states
    //
    for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i)
    {
        MouseBtnState& btn = s_MouseState.button[i];

        if (btn.bPressed)        // was pressed in the prev frame
        {
            btn.bPressed = false;
            btn.bDown = true;
        }
        else if (btn.bReleased)  // was released in the prev frame
        {
            btn.bReleased = false;
        }
    }
    
    //
    // handle new input events
    //
    s_NumEvents = 0;
    InputEvent* e = &s_EventQueue[s_NumEvents];

    while (ProcessInput(e))
    {
        // switch keys states (if keyboard events)
        if (e->type == EVENT_KEYDOWN || e->type == EVENT_KEYUP)
        {
            if (e->keyboard.code == KEY_MAX)
                continue;

            // true - keydown, false - keyup
            s_KeyStates[e->keyboard.code] = (e->type == EVENT_KEYDOWN);
        }
        // update mouse state
        else if (e->type == EVENT_MOUSE)
        {
            switch (e->mouse.type)
            {
                case MOUSE_LPRESS:
                {
                    s_MouseState.button[MOUSE_L].clickX = e->mouse.x;
                    s_MouseState.button[MOUSE_L].clickY = e->mouse.y;
                    s_MouseState.button[MOUSE_L].bPressed = true;
                    break;
                }
                case MOUSE_LRELEASE:
                {
                    s_MouseState.button[MOUSE_L].bDown = false;
                    s_MouseState.button[MOUSE_L].bReleased = true;
                    break;
                }
                case MOUSE_RPRESS:
                {
                    s_MouseState.button[MOUSE_R].clickX = e->mouse.x;
                    s_MouseState.button[MOUSE_R].clickY = e->mouse.y;
                    s_MouseState.button[MOUSE_R].bPressed = true;
                    break;
                }
                case MOUSE_RRELEASE:
                {
                    s_MouseState.button[MOUSE_R].bDown = false;
                    s_MouseState.button[MOUSE_R].bReleased = true;
                    break;
                }
                case MOUSE_MPRESS:
                {
                    s_MouseState.button[MOUSE_M].clickX = e->mouse.x;
                    s_MouseState.button[MOUSE_M].clickY = e->mouse.y;
                    s_MouseState.button[MOUSE_M].bPressed = true;
                    break;
                }
                case MOUSE_MRELEASE:
                {
                    s_MouseState.button[MOUSE_M].bDown = false;
                    s_MouseState.button[MOUSE_M].bReleased = true;
                    break;
                }
                case MOUSE_WHEEL_UP:
                case MOUSE_WHEEL_DOWN:
                {
                    break;
                }
                case MOUSE_MOVE:
                {
                    s_MouseState.px = e->mouse.x;
                    s_MouseState.py = e->mouse.y;
                    break;
                }
            }
        }

        s_NumEvents++;
        e = &s_EventQueue[s_NumEvents];
    }
}

//---------------------------------------------------------
// return an arr of states of each key (true - pressed)
//---------------------------------------------------------
const uint8* GetKeyboardState()
{
    return s_KeyStates;
}

//---------------------------------------------------------
// check if a key is currenly pressed
//---------------------------------------------------------
bool IsKeyPressed(eKeyCode k)
{
    assert(k >= 0 && k < KEY_MAX);
    return s_KeyStates[k];
}

//---------------------------------------------------------
// Desc:  get a single input event and remove it from the events queue
// Ret:   false - when there is no events in the queue left
//---------------------------------------------------------
bool InputGetEvent(InputEvent* e)
{
    assert(e);
    static int eventIdx = 0;

    if (s_NumEvents <= 0)
    {
        eventIdx = 0;
        return false;
    }

    *e = s_EventQueue[eventIdx];

    s_NumEvents--;
    eventIdx++;
    return true;
}

//---------------------------------------------------------
// get a key code by input key name
//---------------------------------------------------------
eKeyCode GetKeycodeByName(const char* key)
{
    assert(key && key[0] != '\0');

    // we have only one char as keyname
    if (key[1] == '\0')
    {
        const char k = key[0];

        // if is digit or ASCII in upper case
        if ((k >= '0' && k <= '9') || (k >= 'A' && k <= 'Z'))
        {
            return (eKeyCode)(k);
        }
        else if (k >= 'a' && k <= 'z')
        {
            return (eKeyCode)(k - ' ');
        }
    }
    else
    {
        if (strcmp(key, "space") == 0)  return KEY_SPACE;
        if (strcmp(key, "tab") == 0)    return KEY_TAB;
        if (strcmp(key, "LShift") == 0) return KEY_LSHIFT;
        if (strcmp(key, "LCtrl") == 0)  return KEY_LCTRL;
    }

    LogErr(LOG, "invalid key: %s", key);
    return KEY_MAX;
}

//---------------------------------------------------------
// get a name of key by input keycode
//---------------------------------------------------------
const char* GetKeyName(const eKeyCode c)
{
    switch (c)
    {
        case KEY_SPACE:     return "space";
        case KEY_TAB:       return "tab";       
        case KEY_LSHIFT:    return "LShift";        
        case KEY_LCTRL:     return "LCtrl";     

        case KEY_A:         return "a";
        case KEY_B:         return "b";
        case KEY_C:         return "c";
        case KEY_D:         return "d";
        case KEY_E:         return "e";
        case KEY_F:         return "f";
        case KEY_G:         return "g";
        case KEY_H:         return "h";
        case KEY_I:         return "i";
        case KEY_J:         return "j";
        case KEY_K:         return "k";
        case KEY_L:         return "l";
        case KEY_M:         return "m";
        case KEY_N:         return "n";
        case KEY_O:         return "o";
        case KEY_P:         return "p";
        case KEY_Q:         return "q";
        case KEY_R:         return "r";
        case KEY_S:         return "s";
        case KEY_T:         return "t";
        case KEY_U:         return "u";
        case KEY_V:         return "v";
        case KEY_W:         return "w";
        case KEY_X:         return "x";
        case KEY_Y:         return "y";
        case KEY_Z:         return "z";

        case KEY_0:         return "0";
        case KEY_1:         return "1";
        case KEY_2:         return "2";
        case KEY_3:         return "3";
        case KEY_4:         return "4";
        case KEY_5:         return "5";
        case KEY_6:         return "6";
        case KEY_7:         return "7";
        case KEY_8:         return "8";
        case KEY_9:         return "9";

        default:            return "unknown_key";
    };
}

//---------------------------------------------------------
// return current state of the mouse
//---------------------------------------------------------
const Mouse& GetMouseState(void)
{
    return s_MouseState;
}