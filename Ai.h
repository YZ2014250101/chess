#pragma once
#include"Board.h"
class Ai {
public:
    Ai() { }
    bool getBestMove(Board& board, int& r, int& c) {
        int bestScore = -1;
        bool foundMove = false;
        for (int row = 0; row < Board::SIZE; ++row) {
            for (int col = 0; col < Board::SIZE; ++col) {   
                if (!board.is_validMove(row,col))
                    continue; 
                int attackScore = evaluateMove(board, row, col, aiPiece);
                int defenseScore = evaluateMove(board, row, col, opponentPiece) * 0.9;
                int total = attackScore + defenseScore;
                if (total > bestScore) {
                    bestScore = total;
                    r = row;
                    c = col;
                    foundMove = true;
                }
            }
        }
        return foundMove;
    }

private:
    Piece aiPiece = Piece::White;
    Piece opponentPiece = Piece::Black;
    int evaluateMove(Board& board, int r, int c, Piece p) {
        if (!board.is_validMove(r,c))
            return -1;

        int dir[4][2] = { { 0, 1 }, { 1, 0 }, { 1, 1 }, { 1, -1 } };
        int score = 0;
        for (int i = 0; i < 4; i++) {
            int cnt = 1;
            int openEnds = 0;
            for (int j = 0;; j++) {
                int nr = r + dir[i][0] * (j + 1);
                int nc = c + dir[i][1] * (j + 1);
                if (board.is_inBoard(nr, nc) && board.getPiece(nr, nc) == p) {
                    cnt++;
                } else if (board.is_inBoard(nr, nc) && board.getPiece(nr, nc) == Piece::Empty) {
                    openEnds++;
                    break;
                } else {
                    break;
                }
            }
            for (int j = 0;; j++) {
                int nr = r - dir[i][0] * (j + 1);
                int nc = c - dir[i][1] * (j + 1);
                if (board.is_inBoard(nr, nc) && board.getPiece(nr, nc) == p) {
                    cnt++;
                } else if (board.is_inBoard(nr, nc) && board.getPiece(nr, nc) == Piece::Empty) {
                    openEnds++;
                    break;
                } else {
                    break;
                }
            }
            if (cnt >= 5)
                score += 1000000;
            if (cnt == 4)
                score += openEnds == 2 ? 100000 : (openEnds == 1 ? 30000 : 0);
            if (cnt == 3)
                score += openEnds == 2 ? 20000 : (openEnds == 1 ? 5000 : 0);
            if (cnt == 2)
                score += openEnds == 2 ? 3000 : (openEnds == 1 ? 500 : 0);
            if (cnt == 1)
                score += openEnds == 2 ? 200 : 50;
        }
        int center = Board::SIZE / 2;
        score += (center - abs(r - center)) + (center - abs(c - center));
        return score;
    }
};
