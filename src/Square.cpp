#include <iostream>
#include <iomanip>
#include "headers/piece.h"
#include "headers/enums.h"
#include "headers/square.h"

bool Square::checkSentinel() {
    return (this->getPiece() && this->getPiece()->getColour() == Colour::UNKNOWN 
        && this->getPiece()->getType() == PieceTypes::UNKNOWN);
}

std::ostream& operator<<(std::ostream& os, const Square& square) {
    const auto squarePiece = square.piece.get();
    // Checks if Square is empty
    if (!squarePiece) {
#ifdef DEBUG
        return os << "  " << std::left << std::setw(5) << square.offset;
#else
        return os << "  ";
#endif
    }
    // Prints out the contents of the Square
#ifdef DEBUG
    return os << std::right << std::setw(4) << square.offset << *squarePiece;
#else
    return os << *squarePiece;
#endif
}

bool operator==(const Square& first, const Square& second) {
    return (first.piece == second.piece && first.offset == second.offset);
}
