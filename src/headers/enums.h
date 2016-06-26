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
    EMPTY,
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
    WHITE_MOVE = (13 * 64),
    WHITE_CASTLE_KING,
    WHITE_CASTLE_QUEEN,
    BLACK_CASTLE_KING,
    BLACK_CASTLE_QUEEN,
    //Start at A and add 1-7 for B-H respectively
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
    {PieceTypes::PAWN, 1},
    {PieceTypes::KNIGHT, 2},
    {PieceTypes::BISHOP, 3},
    {PieceTypes::ROOK, 4},
    {PieceTypes::QUEEN, 5},
    {PieceTypes::KING, 6}
};

#endif
