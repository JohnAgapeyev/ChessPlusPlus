#ifndef PIECE_H
#define PIECE_H

#include "enums.h"

class Piece {
private:
    PieceTypes type;
    Colour pieceColour;

public:
    Piece() {}
    Piece(const PieceTypes t, const Colour c) : type(t), pieceColour(c) {}
    PieceTypes getType() {return type;}
    Colour getColour() {return pieceColour;}
};

#endif
