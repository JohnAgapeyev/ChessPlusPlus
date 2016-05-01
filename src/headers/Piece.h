#ifndef PIECE_H
#define PIECE_H

#include "enums.h"
#include <array>
#include <vector>

class Piece {
private:
    PieceTypes type;
    Colour pieceColour;

public:
    Piece(const PieceTypes t, const Colour c) : type(t), pieceColour(c) {}
    auto getType() const {return type;}
    auto getColour() const {return pieceColour;}
    std::vector<int> getVectorList() const;
};

#endif
