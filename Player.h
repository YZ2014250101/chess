#pragma once
#include "Board.h"
#include <string>
class Player {
public:
    Player(std::string name, Piece p, bool ai = false):name(name),piece(p),ai(ai) {}
    Player() {}
    void setPiece(Piece p) { piece = p; }
    void setAI(bool a) { ai = a; }
    void setName(std::string n) { name = n; }
    Piece getPiece() { return piece; }
    std::string getName() { return name; }
    bool isAI() { return ai; }

private:
    std::string name;
    Piece piece;
    bool ai;
};