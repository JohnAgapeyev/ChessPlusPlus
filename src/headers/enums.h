#ifndef ENUMS_H
#define ENUMS_H

#include <unordered_map>

enum class PieceTypes {
    UNKNOWN = ' ',
    PAWN = 'P',
    KNIGHT = 'N',
    BISHOP = 'B',
    ROOK = 'R',
    QUEEN = 'Q',
    KING = 'K'
};

enum class Colour {
    UNKNOWN = ' ',
    WHITE = 'W',
    BLACK = 'B'
};

enum class GameState {
    ACTIVE,
    DRAWN,
    MATE
};

enum class SquareState {
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
    WHITE_CASTLE_KING,
    WHITE_CASTLE_QUEEN,
    BLACK_CASTLE_KING,
    BLACK_CASTLE_QUEEN,
    //Start at none and add 1-8 for A-H respectively
    EN_PASSANT_FILE
};

namespace std {
    template<>
    class hash<PieceTypes> {
    public:
        size_t operator()(PieceTypes p) const {
            return std::hash<int>()(static_cast<int>(p));
        }
    };
}

static std::unordered_map<PieceTypes, int> pieceLookupTable = {
    {PieceTypes::PAWN, 0},
    {PieceTypes::KNIGHT, 1},
    {PieceTypes::BISHOP, 2},
    {PieceTypes::ROOK, 3},
    {PieceTypes::QUEEN, 4},
    {PieceTypes::KING, 5}
};

#endif
