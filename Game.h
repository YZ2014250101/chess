#pragma once	
#include "Network.h"
#include"Board.h"
#include"Player.h"
#include"Ai.h"
#include <vector>
#include <cwchar>
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
            } else if (scene == Scene::NET_MENU) {
                handleNetMenuInput();
                drawNetMenu();
            } else if (scene == Scene::NET_PLAY) {
                handleNetInput(); 
                pollNetwork(); 
                drawNetScene();
            }
            delay_fps(60);
        }

    }
private:
    enum class Scene {MENU,PLAYING,NET_MENU,NET_PLAY};
    Scene scene;
    Board board;
    Player player[2];
    Ai ai;
    Piece winner;
    bool isGameOver;
    bool exit;
    bool is_PVE;
    bool lastMoveUndone;
    int steps;
    int curPlayer;
    std::vector<Move> history;

    Network net;
    bool is_netMod;
    bool is_ready   ;
    int myturn;
    int netTimer;
    wchar_t netIp[32];
    wchar_t netPort[16];
    wchar_t netMsg[128];

    void pollNetwork() {
        if (!is_ready) {
            if (net.isHost()) {
                if (net.pollAccept()) {
                    startNetGame();
                }
            } else {
                int ev = net.pollConnect();
                if (ev == NET_CONNECTED) {
                    startNetGame();
                } else if (ev == NET_DISCONNECT) {
                    wcscpy(netMsg, L"连接主机失败，请检查IP是否正确");
                    net.close();
                    is_ready = false;
                    is_netMod = false;
                    scene = Scene::NET_MENU;
                }
            }
        }
        // 处理网络消息
        int r = 0, c = 0; // 用于接收落子位置
        NetEvent ev;
        while ((ev = net.pollReceive(r, c)) != NET_NONE) {
            if (ev == NET_STEP) {
                if (!isGameOver && curPlayer != myturn) {
                    makeMove(r, c);
                } else if (ev == NET_RESTART){
                    startNetGame(); // 双方同步重置
                } else if (ev == NET_QUIT || ev == NET_DISCONNECT){
                    // 回主菜单，连接资源由 close() 释放
                    net.close();
                    is_ready = false;
                    is_netMod = false;
                    scene = Scene::MENU;
                    return;
                }
            }
        }
        
    }
    void startNetGame() {
        is_ready = true;
        steps = 0;
        curPlayer = 1;
        history.clear();
        board.clear();
        winner = Piece::Empty;
        isGameOver = false;
        netTimer = 0;
        board.setLastMove(-1, -1);

        if (myturn == 1) {
            player[0] = Player("我", Piece::Black, false);
            player[1] = Player("对方", Piece::White, false);
        } else {
            player[0] = Player("我", Piece::White, false);
            player[1] = Player("对方", Piece::Black, false);
        }
    }
    void handleNetInput() {
        while (kbhit()) {
            int key = getch();
            if (key == 27) {
                //net.sendQuit();
                //net.closeConnection();
                is_netMod = false;
                is_ready = false;
                scene = Scene::NET_MENU;
                return;
            } else if (key == 'R' || key == 'r'){
                net.sendRestart(); // 通知对方
                startNetGame(); // 本机也重置
            } else if (key == 'U' || key == 'u'){
                wcscpy(netMsg, L"网络对战不支持悔棋");
                netTimer = 90; // 提示显示约 1.5 秒
            }
        }
        while (mousemsg() && !isGameOver) {
            auto msg = getmouse();
            if (msg.is_left() && msg.is_down()) {
                if (!is_ready || curPlayer != myturn) {
                    continue; // 不是我的回合，忽略点击
                }
                int r = 0, c = 0;
                if (!board.xyToRC(msg.x, msg.y, r, c))
                    continue; // 不在交叉点附近
                if (!board.is_validMove(r, c))
                    continue; // 该位置已有棋子

                makeMove(r, c); // 本地落子（我的颜色）
                net.sendStep(r, c); 
            }
        }
    }
    void drawNetScene() {
        if (!is_ready) {
            drawNetWait();
            return;
        }
        board.draw(); 
        drawInfoPanel();
    }
    void drawNetWait() {
        setbkcolor(EGERGB(205, 170, 115));
        cleardevice();
        setbkmode(TRANSPARENT);
        setfont(64, 0, L"微软雅黑");
        settextcolor(EGERGB(80, 55, 35));
        outtextxy(290, 110, L"局域网对战");
        setfont(28, 0, L"微软雅黑");
        outtextxy(300, 320, L"等待对手加入...");
        outtextxy(300, 380, L"ESC. 退出等待");
    }
    void drawNetMenu() {
        setbkcolor(EGERGB(205, 170, 115));
        cleardevice();
        setbkmode(TRANSPARENT);
        setfont(64, 0, L"微软雅黑");
        settextcolor(EGERGB(80, 55, 35));
        outtextxy(290, 110, L"局域网联机对战");
        setfont(28, 0, L"微软雅黑");
        outtextxy(300, 320, L"H. 创建房间");
        outtextxy(300, 380, L"C. 加入房间");
        outtextxy(300, 440, L"ESC. 返回菜单");
        if (netMsg[0] != '\0') {
            setfont(20, 0, L"微软雅黑");
            settextcolor(EGERGB(255, 0, 0));
            outtextxy(300, 500, netMsg);
            if (netTimer > 0) {
                netTimer--;
                if (netTimer == 0) {
                    netMsg[0] = '\0';
                }
            }
        }
    }
    void handleNetMenuInput() {
        while (kbhit()) {
            int key = getch();
            if (key == 27) {
                scene = Scene::MENU;
            } else if (key == 'H' || key == 'h') {
                if (!net.startHost()) {
                    wcscpy(netMsg, L"创建房间失败，端口被占用？");
                    netTimer = 120;
                } else {
                    myturn = 1;
                    scene = Scene::NET_PLAY;
                    netTimer = 0;
                }
            } else if (key == 'C' || key == 'c') {
                if (inputbox_getline(L"加入房间" , L"请输入房间主机IP:(如 192.168.1.5）", netIp, 32)) {
                    char ip[32];
                    wcstombs(ip, netIp, sizeof(ip));
                    ip[sizeof(ip) - 1] = '\0';
                    if (!net.connectTo(ip)) {
                        wcscpy(netMsg, L"连接服务器失败，请检查IP是否正确");
                        netTimer = 120;
                    } else {
                        myturn = 2;
                        scene = Scene::NET_PLAY;
                        netTimer = 0;
                    }
                }
            }
        }
    }
    void handleMenuInput() {
        while (kbhit()) {
            int key = getch();
            if (key == '1') {
                player[0] = Player("玩家1", Piece::Black, false);
                player[1] = Player("玩家2", Piece::White, false);
                newGame(false);
            } else if (key == '2') {
                player[0] = Player("玩家", Piece::Black, false);
                player[1] = Player("电脑", Piece::White, true);
                newGame(true);
            } else if (key == '3') {
                enterNetMenu();
            } else if (key == 27) {
                exit = true;
            }
        }
    }
    void enterNetMenu() {
        scene = Scene::NET_MENU;
        is_netMod = true;
        is_ready = false;
        netTimer = 0;
        netMsg[0] = '\0';
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
        if (!is_PVE && steps > 0 && !isGameOver && !lastMoveUndone) {
            steps--;
            curPlayer = curPlayer == 1 ? 2 : 1;
            board.setEmpty(history.back().r, history.back().c);
            history.pop_back();
            board.setLastMove(history.back().r, history.back().c);
            lastMoveUndone = true;
        } else if (is_PVE && steps > 1 && !isGameOver && !lastMoveUndone) {
            steps -= 2;
            board.setEmpty(history.back().r, history.back().c);
            history.pop_back();
            board.setEmpty(history.back().r, history.back().c);
            history.pop_back();
            board.setLastMove(history.back().r, history.back().c);
            lastMoveUndone = true;
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
        outtextxy(300, 380, L"2. 人机模式（玩家执黑先手）");
        outtextxy(300, 440, L"3. 局域网联机对战");
        outtextxy(300, 500, L"ESC. 退出程序");

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
        lastMoveUndone = false;

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
        lastMoveUndone = false;
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
