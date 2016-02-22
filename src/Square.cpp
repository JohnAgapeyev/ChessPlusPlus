#include "headers/piece.h"
#include "headers/enums.h"
#include "headers/consts.h"

auto& operator<<(std::ostream& os, const Square& sq) {
    auto squarePiece = sq.piece.get();
    return os << sq.offset << ' ' << static_cast<char>(squarePiece->getColour()) << static_cast<char>(squarePiece->getType());
}   
