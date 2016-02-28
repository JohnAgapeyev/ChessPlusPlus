#include "headers/piece.h"
#include "headers/enums.h"
#include "headers/square.h"
#include <iostream>
#include <iomanip>

std::ostream& operator<<(std::ostream& os, const Square& square) {
    auto squarePiece = square.piece.get();
    // Checks if Square is empty
    if (!squarePiece) {
        return os << "  " << std::left << std::setw(5) << square.offset;
    }
    // Checks if Square is a sentinel
    if (squarePiece->getColour() == Colour::UNKNOWN 
            || squarePiece->getType() == PieceTypes::UNKNOWN) {
        return os << "   x   ";
    }
    // Prints out the contents of the Square
    return os << std::right << std::setw(4) << square.offset << ' ' 
            << static_cast<char>(squarePiece->getColour()) 
            << static_cast<char>(squarePiece->getType());
}   
