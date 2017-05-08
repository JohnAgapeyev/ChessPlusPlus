#include <iostream>
#include <iomanip>
#include "headers/piece.h"
#include "headers/enums.h"
#include "headers/square.h"

std::ostream& operator<<(std::ostream& os, const Square& square) {
    const auto squarePiece = square.piece.get();
    // Checks if Square is empty
    if (!squarePiece) {
#ifndef NDEBUG
        return os << "  " << std::left << std::setw(5) << square.offset;
#else
        return os << "  ";
#endif
    }
    // Prints out the contents of the Square
#ifndef NDEBUG
    return os << std::right << std::setw(4) << square.offset << *squarePiece;
#else
    return os << *squarePiece;
#endif
}

