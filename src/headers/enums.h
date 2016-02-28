#ifndef ENUMS_H
#define ENUMS_H

enum class PieceTypes {
    UNKNOWN = '\0',
    PAWN = 'P',
    KNIGHT = 'N',
    BISHOP = 'B',
    ROOK = 'R',
    QUEEN = 'Q',
    KING = 'K'
};

enum class Colour {
    UNKNOWN = '\0',
    WHITE = 'W',
    BLACK = 'B'
};


#endif
