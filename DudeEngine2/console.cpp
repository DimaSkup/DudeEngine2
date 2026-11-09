/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: console.h
    Desc:     an old-school console
    Created:  18.09.2026 by DimaSkup
\**********************************************************************************/
#include "console.h"
#include "input.h"
#include "log.h"
#include "r_main.h"
#include "ui_text_mgr.h"
#include "game_state.h"
#include "game.h"
#include <memory.h>


static Console s_Console;

//---------------------------------------------------------
// reset the console
//---------------------------------------------------------
void ConsoleClear()
{
    memset(&s_Console, 0, sizeof(s_Console));
}

//---------------------------------------------------------
// handle input to console except of Enter
//---------------------------------------------------------
void ConsoleInput(const int keyCode)
{
    const eKeyCode c = (eKeyCode)(keyCode);

    switch (c)
    {
        case KEY_HOME:
            s_Console.cursorPos = 0;
            break;

        case KEY_END:
        {
            int i = 0;

            while (s_Console.buf[i] != '\0')
            {
                s_Console.cursorPos = i;
                i++;
            }

            s_Console.cursorPos++;
        }

        case KEY_UP:
        {
            memset(&s_Console.buf, 0, sizeof(s_Console.buf));
            s_Console.cursorPos = (int)strlen(s_Console.prevBuf);   // place the cursor at the end of the line
            strcpy(s_Console.buf, s_Console.prevBuf);               // get a previous cmd from history
            break;
        }

        case KEY_LEFT:
            if (s_Console.cursorPos > 0)
                s_Console.cursorPos--;
            break;

        case KEY_RIGHT:
        {
            const int p = s_Console.cursorPos;

            if (s_Console.buf[p] != '\0' && p < CONSOLE_BUF_SIZE)
                s_Console.cursorPos++;

            break;
        }

        case KEY_SPACE:

        // numeric chars
        case KEY_0:
        case KEY_1:
        case KEY_2:
        case KEY_3:
        case KEY_4:
        case KEY_5:
        case KEY_6:
        case KEY_7:
        case KEY_8:
        case KEY_9:

        // alpha chars
        case KEY_A:
        case KEY_B:
        case KEY_C:
        case KEY_D:
        case KEY_E:
        case KEY_F:
        case KEY_G:
        case KEY_H:
        case KEY_I:
        case KEY_J:
        case KEY_K:
        case KEY_L:
        case KEY_M:
        case KEY_N:
        case KEY_O:
        case KEY_P:
        case KEY_Q:
        case KEY_R:
        case KEY_S:
        case KEY_T:
        case KEY_U:
        case KEY_V:
        case KEY_W:
        case KEY_X:
        case KEY_Y:
        case KEY_Z:
        {
            const int p = s_Console.cursorPos;

            if (p == CONSOLE_BUF_SIZE)
                break;

            char ch = '\0';
            
            if (keyCode == KEY_SPACE)
                ch = ' ';
            else
                ch = GetKeyName(c)[0];

            // we're in the end of the buffer's content so just push back a new char
            if (s_Console.buf[p] == '\0')
            {
                s_Console.buf[p] = ch;
            }

            // we're somewhere in the middle...
            else
            {
                int end = 0;

                // shift right all the content until the end
                while (s_Console.buf[end] != '\0')
                    end++;

                while (end != p)
                {
                    s_Console.buf[end] = s_Console.buf[end - 1];
                    end--;
                }

                s_Console.buf[p] = ch;
            }

            s_Console.cursorPos++;
            break;
        }

        case KEY_BACKSPACE:
        {
            const int p = s_Console.cursorPos;

            if (p == 0)
                break;

            // we're in the end of the buffer's content so just pop back a char
            if (s_Console.buf[p] == '\0')
            {
                s_Console.buf[p-1] = '\0';
            }

            // we're somewhere in the middle...
            else
            {
                int i = p;

                // shift left all the content from the current cursor pos until the end
                while (s_Console.buf[i] != '\0')
                {
                    s_Console.buf[i] = s_Console.buf[i + 1];
                    i++;
                }
            }

            s_Console.cursorPos--;

            break;
        }

        case KEY_DELETE:
        {
            const int p = s_Console.cursorPos;

            // we're in the end of the buffer's content so just get out
            if (s_Console.buf[p] == '\0')
                break;

            // we're somewhere in the middle...
            else
            {
                int i = p;

                // shift left all the content from the current cursor pos until the end
                while (s_Console.buf[i] != '\0')
                {
                    s_Console.buf[i] = s_Console.buf[i + 1];
                    i++;
                }
            }

            break;
        }
    }
}

//---------------------------------------------------------
// process a command from the console buffer
//---------------------------------------------------------
void ConsoleProc()
{
    const char* cmd = s_Console.buf;

    if (strcmp(cmd, "reload") == 0)
    {
        ConsoleClear();
        g_GameState.bReloadLevel = true;
        strcpy(s_Console.prevBuf, s_Console.buf);
    }
    else if (strncmp(cmd, "load level", 10) == 0)
    {
        char levelName[MAX_LEN_GAME_LVL_NAME];
        memset(levelName, 0, sizeof(levelName));

        int count = sscanf(cmd, "load level %s", levelName);
        if (count != 1)
        {
            LogErr(LOG, "can't get level's name from input cmd: %s", cmd);
            return;
        }

        Game* pGame = (Game*)g_GameState.pGame;
        pGame->WantLoadLevel(levelName);
    }

    else if (strcmp(cmd, "exit") == 0)
    {
        g_GameState.bRunning = false;
    }

    else
    {
        strcpy(s_Console.prevBuf, s_Console.buf);
        strcpy(s_Console.buf, "unknown command");
        s_Console.cursorPos = (int)strlen(s_Console.buf);   // place the cursor at the end of the line
        return;
    }

    // save for history and clear the buffer
    strcpy(s_Console.prevBuf, s_Console.buf);
    memset(&s_Console.buf, 0, sizeof(s_Console.buf));
}

//---------------------------------------------------------
// render the console onto the screen
//---------------------------------------------------------
void ConsoleRender()
{
    const Vec2i wndSz = g_GameState.wndSize;
    g_Render.SetDrawColor(0, 0, 0, 255);
    g_Render.DrawFillRect(0, 0, wndSz.w, wndSz.h / 4);

    g_Text.SetFontType(FONT_TYPE_DEBUG);
    const int charW = g_Text.GetCharWidth(FONT_TYPE_DEBUG, ' ');
    const int fontH = g_Text.GetFontHeight(FONT_TYPE_DEBUG);

    int strPosY = wndSz.h / 4 - fontH;

    // draw the buffer content (command)
    if (s_Console.buf[0] != '\0')
    {
        g_Text.FlushFlags();
        g_Text.SetTextColor(255, 255, 255, 255);
        g_Text.SetPos(0, strPosY);
        g_Text.Draw(s_Console.buf);
        g_Text.Render();
    }

    // draw a cursor
    int cursorOffsetX = s_Console.cursorPos * charW;
    int gameTimeMs = (int)(g_GameState.gameTime * 1000.0f);

    if ((gameTimeMs >> 8) & 1)  // cursor blinking each 256 ms
    {
        g_Render.SetDrawColor(255, 255, 255, 255);
        g_Render.DrawFillRect(cursorOffsetX, strPosY, cursorOffsetX + charW, strPosY + fontH);
    }
}