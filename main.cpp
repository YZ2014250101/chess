#include <graphics.h>
#include "Game.h"

int main()
{   
    initgraph(Board::WIN_W, Board::WIN_H);
    setrendermode(RENDER_MANUAL);
    setcaption("五子棋");
    Game game;
    game.run();
    closegraph();
    return 0;
}