/**
* This is part of ChessPlusPlus, a C++14 Chess AI
* Copyright (C) 2017 John Agapeyev
* 
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

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

