#pragma once	
#include"Board.h"
#include"Player.h"
#include"Ai.h"
#include <vector>

class Game {
public:
    Game() {
        scene = Scene::MENU;
        winner = Piece::Empty;
        isGameOver = false;
        exit = false;
        is_PVE = false;
        steps = 0;
        curPlayer = 1;
    }
    void run() {
        while (is_run() && !exit) {
            if (scene == Scene::MENU) {
                handleMenuInput();
                drawMenu();
            } else if (scene == Scene::PLAYING) {
                HandleGameInput();
                drawScene();
                drawInfoPanel();
            } 
            delay_fps(60);
        }

    }
private:
    enum class Scene {MENU,PLAYING};
    Scene scene;
    Board board;
    Player player[2];
    Ai ai;
    Piece winner;
    bool isGameOver;
    bool exit;
    bool is_PVE;
    int steps;
    int curPlayer;
    std::vector<Move> history;
    void handleMenuInput() {
        while (kbhit()) {
            int key = getch();
            if (key == '1') {
                player[0].setPiece(Piece::Black);
                player[0].setAI(false);
                player[0].setName("玩家1");
                player[1].setPiece(Piece::White);
                player[1].setAI(false);
                player[1].setName("玩家2");
                newGame(false);
            } else if (key == '2') {
                player[0].setPiece(Piece::Black);
                player[0].setAI(false);
                player[0].setName("玩家");
                player[1].setPiece(Piece::White);
                player[1].setAI(true);
                player[0].setName("电脑");
                newGame(true);
            } else if (key == 27) {
                exit = true;
            }
        }
    }
    void HandleGameInput() {
        while (kbhit()) {
            handleKey(getch());
        }
        while (mousemsg()&&!isGameOver) {
            auto msg = getmouse();
            if (msg.is_left() && msg.is_down()) {
                handleClick(msg.x, msg.y);
            }
        }
    }
    void handleKey(int key) {
        if (key == 27) {
            scene = Scene::MENU;
        } else if (key == 'U' || key == 'u') {
            undo();
        } else if (key == 'R' || key == 'r') {
            newGame(is_PVE);
        }
    }
    void undo() {
        if (steps > 0 && !isGameOver) {
            steps--;
            curPlayer = curPlayer == 1 ? 2 : 1;
            board.setEmpty(history.back().r, history.back().c);
            history.pop_back();
            board.setLastMove(history.back().r, history.back().c);
        }
    }
    void drawMenu() {
        setbkcolor(EGERGB(205, 170, 115));
        cleardevice();
        setbkmode(TRANSPARENT);

        setfont(64, 0, L"微软雅黑"); 
        settextcolor(EGERGB(80, 55, 35));
        outtextxy(290, 110, L"五 子 棋"); 

        setfont(20, 0, L"微软雅黑");
        outtextxy(285, 210, L"基于 EGE 图形库的 C++ 课程作业");


        setfont(28, 0, L"微软雅黑");
        outtextxy(300, 320, L"1. 双人对战");
        outtextxy(300, 380, L"2. 人机对战（玩家执黑先手）");
        outtextxy(300, 440, L"ESC. 退出程序");

        setfont(18, 0, L"微软雅黑");
        outtextxy(220, 600, L"对局中：鼠标左键落子  U 悔棋  R 重新开始  ESC 返回菜单");

    }
    void drawScene(){
        board.draw();
    }
    void drawInfoPanel()
    {
        int x = Board::PANEL_X; 
        settextcolor(EGERGB(80, 55, 35));
        setfont(30, 0, L"微软雅黑");

        setbkmode(TRANSPARENT); 

      
        outtextxy(x + 30, 14, L"对局信息");
        setcolor(EGERGB(160, 120, 80));
        line(x + 20, 52, Board::WIN_W - 20, 52); 

        int y = 80; 
        settextcolor(EGERGB(80, 55, 35));

        outtextxy(x + 20, y, "模式：");
        outtextxy(x + 105, y, is_PVE ? "人机对战" : "双人对战");

        y += 44;
        int index = curPlayer - 1;
        Piece cur = player[index].getPiece();
        setfillcolor(cur == Piece::Black ? RGB(20, 20, 20) : RGB(255, 255, 255)); 
        setcolor(cur == Piece::Black ? RGB(20, 20, 20) : RGB(150, 150, 150)); 
        fillcircle(x + 32, y + 12, Board::CELL / 2 - 5); 
        settextcolor(EGERGB(80, 55, 35));

        outtextxy(x + 52, y, "回合：");
        outtextxy(x + 130, y, player[index].getName().c_str()); 
        y += 44;
        outtextxy(x + 20, y, "已落子：");
        xyprintf(x + 120, y, "%d", steps);
        outtextxy(x + 150, y, "手");

        y += 52;
        setcolor(RGB(160, 120, 80));
        line(x + 20, y, Board::WIN_W - 20, y);
        y += 14;
        settextcolor(EGERGB(80, 55, 35));

        outtextxy(x + 20, y, L"操作说明");
        y += 32;
        outtextxy(x + 20, y, L"[U] 悔棋");
        y += 28;
        outtextxy(x + 20, y, L"[R] 重新开始");
        y += 28;
        outtextxy(x + 20, y, L"[ESC] 返回菜单");

        if (is_PVE) {
            y += 40;
            outtextxy(x + 20, y, L"玩家执黑先手");
            y += 28;
            outtextxy(x + 20, y, L"电脑执白后手");
        }
    }
    void newGame(bool pve) {
        while (mousemsg()) {
            getmouse();
        }
        while (kbhit()) {
            getch();
        }
        board.clear();
        board.setLastMove(-1, -1);
        scene = Scene::PLAYING;
        winner = Piece::Empty;
        history.clear();
        winner = Piece::Empty;
        curPlayer = 1;

        isGameOver = false;
        is_PVE = pve;
        steps = 0;
        curPlayer = 1;
    }

    void handleClick(int x, int y) {
        if (isGameOver)
            return;  

        int col;
        int row;
        if (board.xyToRC(x, y, row, col)) {
            makeMove(row, col);
            if (is_PVE && !isGameOver && curPlayer == 2) {
                int aiRow, aiCol;
                ai.getBestMove(board, aiRow, aiCol);
                makeMove(aiRow, aiCol);
            } 
        }
    }
    void makeMove(int r, int c) {
        if (exit || isGameOver) return;
        if (!board.setPiece(r, c, static_cast<Piece>(curPlayer)))
            return ;
        history.push_back({ r, c, static_cast<Piece>(curPlayer) });
        board.setLastMove(r, c);
        if (!checkWin(r, c)) {
            curPlayer = curPlayer == 1 ? 2 : 1;
            steps++;
        }
        
    }
    bool checkWin(int r, int c) {
        Piece p = static_cast<Piece>(curPlayer);
        int dir[4][2] = { { 0, 1 }, { 1, 0 }, { 1, 1 }, { 1, -1 } };
        for (int i = 0; i < 4; i++) {
            int cnt = 1;
            std::vector<Move> temp;
            temp.push_back({ r, c, p });
            for (int j = 0;; j++) {
                int nr = r + dir[i][0] * (j + 1);
                int nc = c + dir[i][1] * (j + 1);
                if (board.is_inBoard(nr, nc) && board.getPiece(nr, nc)==p) {
                    temp.push_back({ nr, nc, p });
                    cnt++;
                } else {
                    break;
                }
            }
            for (int j = 0;; j++) {
                int nr = r - dir[i][0] * (j + 1);
                int nc = c - dir[i][1] * (j + 1);
                if (board.is_inBoard(nr, nc) && board.getPiece(nr, nc)==p) {
                    temp.push_back({ nr, nc, p });
                    cnt++;
                } else {
                    break;
                }
            }
            if (cnt >= 5) {
                drawScene();
                drawInfoPanel();
                board.drawWin(temp);
                winner = p;
                isGameOver = true;
                getch();
                return true;
            }
        }
        return false;
    }
};
