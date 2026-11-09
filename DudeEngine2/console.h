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
#ifndef CONSOLE_H
#define CONSOLE_H

#define CONSOLE_BUF_SIZE 64

struct Console
{
    char prevBuf[CONSOLE_BUF_SIZE];
    char buf[CONSOLE_BUF_SIZE];
    int cursorPos;
};

void ConsoleClear();
void ConsoleInput(const int keyCode);
void ConsoleProc();
void ConsoleRender();

#endif // CONSOLE_H
