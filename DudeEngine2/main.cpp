/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   main.cpp
    Desc:       obviously, we starts from here
    Created:    by DimaSkup
\**********************************************************************************/
#include "game.h"
#include "timer.h"

int main(int argc, char* args[])
{
    Game game;

    game.Init();

    while (game.IsRunning())
    {
        game.ProcessInput();
        game.Update();
        game.Render();
    }

    game.Shutdown();

    return 0;
}