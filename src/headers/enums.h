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

#ifndef ENUMS_H
#define ENUMS_H

#include <unordered_map>

/**
 * Various enums used throughout the program
 * to represent values such as piece types and colour.
 * These enums also represent values for use in board hashing.
 */

enum class PieceTypes : unsigned char {
    UNKNOWN = ' ',
    PAWN = 'P',
    KNIGHT = 'N',
    BISHOP = 'B',
    ROOK = 'R',
    QUEEN = 'Q',
    KING = 'K'
};

enum class Colour : unsigned char {
    UNKNOWN = ' ',
    WHITE = 'W',
    BLACK = 'B'
};

enum class GameState : unsigned char {
    ACTIVE,
    DRAWN,
    MATE
};

enum class SquareState : unsigned int {
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
    //Add current castle rights to this to get each combination
    CASTLE_RIGHTS,
    //Start at none and add 1-8 for A-H respectively
    EN_PASSANT_FILE = (12 * 64) + 16 + 1
};

enum class SearchBoundary : unsigned char {
    UPPER,
    LOWER,
    EXACT
};

extern const std::unordered_map<PieceTypes, int> pieceLookupTable;

#endif
