/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: sdl_input.cpp
    Desc:     implementation of SDL2-specific functional for handling input events
    Created:  09.06.2026 by DimaSkup
\**********************************************************************************/
#include <SDL_events.h>
#include "../../input.h"
#include "../../log.h"

#include <imgui_impl_sdl2.h>    // for processing input events in the editor
#include <assert.h>
#include <memory.h>


//---------------------------------------------------------
// is a key by input keycode pressed once?
//---------------------------------------------------------
inline bool IsKeyRepeat(const SDL_Event* e, const SDL_Keycode c)
{
    assert(e);
    return (e->key.repeat != 0);
}

//---------------------------------------------------------
// SDL keycode => internal keycode
//---------------------------------------------------------
static eKeyCode ToKeyCode(SDL_Keycode c)
{
    switch (c)
    {
        case SDLK_TAB:          return KEY_TAB;
        case SDLK_ESCAPE:       return KEY_ESCAPE;
        case SDLK_LALT:         return KEY_LALT;
        case SDLK_LCTRL:        return KEY_LCTRL;
        case SDLK_DELETE:       return KEY_DELETE;
        case SDLK_SPACE:        return KEY_SPACE;
        case SDLK_LSHIFT:       return KEY_LSHIFT;
        case SDLK_BACKQUOTE:    return KEY_NUMPAD0;
        case SDLK_BACKSPACE:    return KEY_BACKSPACE;
        case SDLK_RETURN:       return KEY_ENTER;

        case SDLK_a:            return KEY_A;
        case SDLK_b:            return KEY_B;
        case SDLK_c:            return KEY_C;
        case SDLK_d:            return KEY_D;
        case SDLK_e:            return KEY_E;
        case SDLK_f:            return KEY_F;
        case SDLK_g:            return KEY_G;
        case SDLK_h:            return KEY_H;
        case SDLK_i:            return KEY_I;
        case SDLK_j:            return KEY_J;
        case SDLK_k:            return KEY_K;
        case SDLK_l:            return KEY_L;
        case SDLK_m:            return KEY_M;
        case SDLK_n:            return KEY_N;
        case SDLK_o:            return KEY_O;
        case SDLK_p:            return KEY_P;
        case SDLK_q:            return KEY_Q;
        case SDLK_r:            return KEY_R;
        case SDLK_s:            return KEY_S;
        case SDLK_t:            return KEY_T;
        case SDLK_u:            return KEY_U;
        case SDLK_v:            return KEY_V;
        case SDLK_w:            return KEY_W;
        case SDLK_x:            return KEY_X;
        case SDLK_y:            return KEY_Y;
        case SDLK_z:            return KEY_Z;

        case SDLK_0:            return KEY_0;
        case SDLK_1:            return KEY_1;
        case SDLK_2:            return KEY_2;
        case SDLK_3:            return KEY_3;
        case SDLK_4:            return KEY_4;
        case SDLK_5:            return KEY_5;
        case SDLK_6:            return KEY_6;
        case SDLK_7:            return KEY_7;
        case SDLK_8:            return KEY_8;
        case SDLK_9:            return KEY_9;


        case SDLK_F1:           return KEY_F1;
        case SDLK_F2:           return KEY_F2;
        case SDLK_F3:           return KEY_F3;
        case SDLK_F4:           return KEY_F4;
        case SDLK_F5:           return KEY_F5;
        case SDLK_F6:           return KEY_F6;
        case SDLK_F7:           return KEY_F7;
        case SDLK_F8:           return KEY_F8;
        case SDLK_F9:           return KEY_F9;
        case SDLK_F10:          return KEY_F10;
        case SDLK_F11:          return KEY_F11;
        case SDLK_F12:          return KEY_F12;

        case SDLK_RIGHT:        return KEY_RIGHT;
        case SDLK_LEFT:         return KEY_LEFT;
        case SDLK_DOWN:         return KEY_DOWN;
        case SDLK_UP:           return KEY_UP;

        case SDLK_HOME:         return KEY_HOME;
        case SDLK_END:          return KEY_END;
    }

    LogErr(LOG, "can't convert the keycode: %d (%#03x)", (int)c, c);
    return KEY_MAX;
}

//---------------------------------------------------------
// handle SDL input events
//---------------------------------------------------------
static bool ProcessInput(InputEvent* event)
{
    assert(event);
    memset(event, 0, sizeof(*event));

    SDL_Event e;

    // no events are available
    if (!SDL_PollEvent(&e))
        return false;

    // TODO: I don't know where to put ImGui event processing so it's here
    ImGui_ImplSDL2_ProcessEvent(&e);


    if (e.type == SDL_QUIT)
    {
        event->type = EVENT_QUIT;
    }

    else if (e.type == SDL_KEYDOWN)
    {
        event->type             = EVENT_KEYDOWN;
        event->keyboard.bRepeat = IsKeyRepeat(&e, e.key.keysym.sym);
        event->keyboard.code    = ToKeyCode(e.key.keysym.sym);
    }

    else if (e.type == SDL_KEYUP)
    {
        event->type          = EVENT_KEYUP;
        event->keyboard.code = ToKeyCode(e.key.keysym.sym);
    }

    else if (e.type == SDL_MOUSEMOTION)
    {
        event->type = EVENT_MOUSE;
        event->mouse.type = MOUSE_MOVE;
        event->mouse.x = e.motion.x;
        event->mouse.y = e.motion.y;
    }

    // handle releasing of a mouse button
    else if (e.type == SDL_MOUSEBUTTONUP)
    {
        event->type = EVENT_MOUSE;

        if (e.button.button == SDL_BUTTON_LEFT)
            event->mouse.type = MOUSE_LRELEASE;

        else if (e.button.button == SDL_BUTTON_MIDDLE)
            event->mouse.type = MOUSE_MRELEASE;

        else if (e.button.button == SDL_BUTTON_RIGHT)
            event->mouse.type = MOUSE_RRELEASE;

        else
            LogErr(LOG, "unknown mouse button (releasing event): %d", e.button.button);
    }

    // handle clicking of a mouse button
    else if (e.type == SDL_MOUSEBUTTONDOWN)
    {
        event->type = EVENT_MOUSE;
        event->mouse.x = e.button.x;
        event->mouse.y = e.button.y;
        
        if (e.button.button == SDL_BUTTON_LEFT)
            event->mouse.type = MOUSE_LPRESS;

        else if (e.button.button == SDL_BUTTON_MIDDLE)
            event->mouse.type = MOUSE_MPRESS;

        else if (e.button.button == SDL_BUTTON_RIGHT)
            event->mouse.type = MOUSE_RPRESS;

        else
            LogErr(LOG, "unknown mouse button (pressing event): %d", e.button.button);
    }

    return true;
}
