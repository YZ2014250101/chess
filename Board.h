#pragma once
#include<graphics.h>
#include <vector>
#include <cstdlib>
enum class Piece : int {
    Empty = 0,
    Black = 1,
    White = 2
};
typedef struct Move {
    int r, c;
    Piece color;
    bool operator==(const Move& other) const {
        return r == other.r && c == other.c && color == other.color;
    }
} Move;
class Board {
public:
    static constexpr int WIN_W = 860;
    static constexpr int WIN_H = 700;
    static constexpr int SIZE = 15;
    static constexpr int CELL = 40;
    static constexpr int MARGIN = 50;
    static constexpr int BOARD_PX = CELL * (SIZE - 1);
    static constexpr int PANEL_X = MARGIN + BOARD_PX + 30;

    Board() {
        clear();
        setLastMove(-1, -1);
    }
    void draw() {
        setbkcolor(EGERGB(205, 170, 115));
        cleardevice();
        setcolor(EGERGB(80, 55, 35));
        drawGrid();
        drawPieces();
        drawLastMove();
    }
    void drawLastMove() {
        if (lastMoveR != -1 && lastMoveC != -1) {
            int x, y;
            rcToXY(lastMoveR, lastMoveC, x, y);
            setcolor(EGERGB(255, 0, 0));
            setlinewidth(2);
            circle(x, y, CELL/2 - 2);
            setlinewidth(1);
        }
    }
    void drawWin(const std::vector<Move>& winMoves) {
        if (winMoves.front().color == Piece::Black) {
            setfillcolor(BLACK);
        } else {
            setfillcolor(WHITE);
        }
        fillcircle(MARGIN + winMoves.front().c * CELL, MARGIN + winMoves.front().r * CELL, CELL / 2 - 2);
        setlinewidth(3);
        for (const auto& move : winMoves) {
            if (move == winMoves.front()) {
                setcolor(EGERGB(255, 0, 0));
                int x, y;
                rcToXY(move.r, move.c, x, y);
                circle(x, y, CELL / 2 - 2);
                continue;
            }
            setcolor(EGERGB(0, 255, 0));
            int x, y;
            rcToXY(move.r, move.c, x, y);
            circle(x, y, CELL / 2 - 2);
        }
        line(winMoves.front().c * CELL + MARGIN, winMoves.front().r * CELL + MARGIN, winMoves.back().c * CELL + MARGIN, winMoves.back().r * CELL + MARGIN);
        setlinewidth(1);     
    }
    void setEmpty(int r, int c) {
        if (is_inBoard(r, c)) {
            board[r][c] = Piece::Empty;
        }
    }
    Piece getPiece(int r, int c) {
        if (is_inBoard(r, c)) {
            return board[r][c];
        }
    }
    bool setPiece(int r, int c, Piece p) {
        if (is_validMove(r, c) &&is_inBoard(r, c)) {
            board[r][c] = p;
            return true;
        }
        return false;
    }
    bool is_validMove(int r, int c) {
        return board[r][c] == Piece::Empty;
    }
    bool is_inBoard(int r, int c) {
        return r >= 0 && r < SIZE && c >= 0 && c < SIZE;
    }
    bool xyToRC(int x,int y,int& r, int& c) {
        int col = (x - MARGIN + CELL / 2) / CELL;
        int row = (y - MARGIN + CELL / 2) / CELL;
        if (col < 0 || col >= SIZE || row < 0 || row >= SIZE) {
            return false;
        }
        if (abs(x - MARGIN - col * CELL - CELL / 2) <= 3 || abs(y - MARGIN - row * CELL - CELL / 2) <= 3) {
            return false;
        }
        r = row;
        c = col;
        return true;
    }
    bool rcToXY(int r, int c, int& x, int& y) {
        if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) {
            return false;
        }
        x = MARGIN + c * CELL;
        y = MARGIN + r * CELL;
        return true;
    }

    void drawPieces() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (board[i][j] == Piece::Black) {
                    setfillcolor(BLACK);
                    fillcircle(MARGIN + j * CELL, MARGIN + i * CELL, CELL / 2 - 2);
                } else if (board[i][j] == Piece::White) {
                    setfillcolor(WHITE);
                    fillcircle(MARGIN + j * CELL, MARGIN + i * CELL, CELL / 2 - 2);
                }
            }
        }
    }
    void drawGrid() {
        for (int i = 0; i < SIZE; i++) {
            line(MARGIN, MARGIN + i * CELL, MARGIN + BOARD_PX, MARGIN + i * CELL);
            line(MARGIN + i * CELL, MARGIN, MARGIN + i * CELL, MARGIN + BOARD_PX);
        }
    }
    void setLastMove(int r, int c) {
        lastMoveR = r;
        lastMoveC = c;
    }

    void clear() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = Piece::Empty;
            }
        }
    }

private:
    Piece board[SIZE][SIZE];
    int lastMoveR, lastMoveC;
};