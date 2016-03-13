#ifndef PIECE_H
#define PIECE_H

#include "enums.h"
#include <array>
#include <vector>

class Piece {
private:
    PieceTypes type;
    Colour pieceColour;
    std::vector<int> vecOffsets;

public:
    Piece(const PieceTypes t, const Colour c);
    PieceTypes getType() const {return type;}
    Colour getColour() const {return pieceColour;}
    auto getVectorList() const {return vecOffsets;}
};

#endif
