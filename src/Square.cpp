#include "headers/piece.h"
#include "headers/enums.h"
#include "headers/square.h"
#include <iostream>
#include <iomanip>

bool Square::checkSentinel() {
    auto pc = this->getPiece();
    return (pc && pc->getColour() == Colour::UNKNOWN && pc->getType() == PieceTypes::UNKNOWN);
}

std::ostream& operator<<(std::ostream& os, const Square& square) {
    auto squarePiece = square.piece.get();
    // Checks if Square is empty
    if (!squarePiece) {
#ifdef DEBUG
        return os << "  " << std::left << std::setw(5) << square.offset;
#else
        return os << "  ";
#endif
    }
    // Checks if Square is a sentinel
    if (squarePiece->getColour() == Colour::UNKNOWN 
            || squarePiece->getType() == PieceTypes::UNKNOWN) {
#ifdef DEBUG
        return os << std::right << std::setw(4) << square.offset << " x ";
        //return os << "   x   ";
#else
        return os << "";
#endif
    }
    // Prints out the contents of the Square
#ifdef DEBUG
    return os << std::right << std::setw(4) << square.offset << ' ' 
            << static_cast<char>(squarePiece->getColour()) 
            << static_cast<char>(squarePiece->getType());
#else
    return os << static_cast<char>(squarePiece->getColour()) 
            << static_cast<char>(squarePiece->getType());
#endif
}   
