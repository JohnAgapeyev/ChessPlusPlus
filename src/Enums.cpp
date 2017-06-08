#include <unordered_map>
#include "headers/enums.h"

const std::unordered_map<PieceTypes, int> pieceLookupTable = {
    {PieceTypes::PAWN, 0},
    {PieceTypes::KNIGHT, 1},
    {PieceTypes::BISHOP, 2},
    {PieceTypes::ROOK, 3},
    {PieceTypes::QUEEN, 4},
    {PieceTypes::KING, 5}
};

