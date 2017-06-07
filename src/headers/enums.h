#ifndef ENUMS_H
#define ENUMS_H

#include <unordered_map>

enum class PieceTypes : unsigned char {
    UNKNOWN = ' ',
    PAWN = 'P',
    KNIGHT = 'N',
    BISHOP = 'B',
    ROOK = 'R',
    QUEEN = 'Q',
    KING = 'K'
};

enum class Colour : unsigned char {
    UNKNOWN = ' ',
    WHITE = 'W',
    BLACK = 'B'
};

enum class GameState : unsigned char {
    ACTIVE,
    DRAWN,
    MATE
};

enum class SquareState : unsigned int {
    WHITE_PAWN,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,
    BLACK_PAWN,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING,
    WHITE_MOVE = (12 * 64),
    //Add current castle rights to this to get each combination
    CASTLE_RIGHTS,
    //Start at none and add 1-8 for A-H respectively
    EN_PASSANT_FILE = (12 * 64) + 16 + 1
};

enum class SearchBoundary : unsigned char {
    UPPER,
    LOWER,
    EXACT
};

static const std::unordered_map<PieceTypes, int> pieceLookupTable = {
    {PieceTypes::PAWN, 0},
    {PieceTypes::KNIGHT, 1},
    {PieceTypes::BISHOP, 2},
    {PieceTypes::ROOK, 3},
    {PieceTypes::QUEEN, 4},
    {PieceTypes::KING, 5}
};

#endif
