#include "headers/piece.h"
#include "headers/enums.h"
#include "headers/square.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, const Square& square) {
    auto squarePiece = square.piece.get();
    // Checks if Square is empty
    if (!squarePiece) {
        return os << "  " << square.offset << "  ";
    }
    // Checks if Square is a sentinel
    if (squarePiece->getColour() == Colour::UNKNOWN 
            || squarePiece->getType() == PieceTypes::UNKNOWN) {
        return os << "Empty";
    }
    // Prints out the contents of the Square
    return os << ' ' << square.offset << ' ' 
            << static_cast<char>(squarePiece->getColour()) 
            << static_cast<char>(squarePiece->getType());
}   
