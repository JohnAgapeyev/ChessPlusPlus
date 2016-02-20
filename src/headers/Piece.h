#ifndef PIECE_H
#define PIECE_H

#include "enums.h"

class Piece {
private:
    PieceTypes type;
    Colour pieceColour;

public:
    PieceTypes getType() {return type;}
    Colour getColour() {return pieceColour;}
};

#endif
