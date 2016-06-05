#ifndef PIECE_H
#define PIECE_H

#include "enums.h"
#include <array>
#include <vector>
#include <iostream>

class Piece {
private:
    PieceTypes type;
    Colour pieceColour;

public:
    Piece(const PieceTypes t, const Colour c) : type(t), pieceColour(c) {}
    auto getType() const {return type;}
    auto getColour() const {return pieceColour;}
    std::vector<int> getVectorList() const;
    int getVectorLength() const;
    void promote(PieceTypes newType);
    
    friend std::ostream& operator<<(std::ostream& os, const Piece& piece);
};

#endif
