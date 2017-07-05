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

#ifndef CONSTS_H
#define CONSTS_H

#include <cstdint>
#include "square.h"

/**
 * Various global constants used throughout the program.
 * Mainly used for castling bit-flipping and reducing magic numbers.
 */

constexpr uint_least8_t INNER_BOARD_SIZE = 8;
constexpr uint_least8_t OUTER_BOARD_SIZE = 15;
constexpr uint_least8_t TOTAL_BOARD_SIZE = OUTER_BOARD_SIZE * OUTER_BOARD_SIZE;
constexpr uint_least8_t NUM_SQUARE_STATES = 12;

/*
 * Value meaning:
 * 64 squares in a chess board
 * 12 states for each square
 * 1 for side to move
 * 16 for castling rights
 * 9 for en passant target file including empty
 */
constexpr uint_least16_t HASH_BOARD_LENGTH = (NUM_SQUARE_STATES * 64) + 1 + 16 + 9;
constexpr std::pair<int, int> ZERO_LOCATION = std::make_pair(7, 7);
constexpr uint_least8_t ZERO_LOCATION_1D = (ZERO_LOCATION.first * OUTER_BOARD_SIZE) + ZERO_LOCATION.second;

extern const std::array<std::array<std::unique_ptr<Square>, INNER_BOARD_SIZE>, INNER_BOARD_SIZE> INIT_BOARD;
extern const std::array<uint_fast64_t, HASH_BOARD_LENGTH> HASH_VALUES;
extern const unsigned char BLACK_CASTLE_FLAG;
extern const unsigned char WHITE_CASTLE_FLAG;
extern const unsigned char WHITE_CASTLE_QUEEN_FLAG;
extern const unsigned char WHITE_CASTLE_KING_FLAG;
extern const unsigned char BLACK_CASTLE_QUEEN_FLAG;
extern const unsigned char BLACK_CASTLE_KING_FLAG;

std::array<std::array<std::unique_ptr<Square>, INNER_BOARD_SIZE>, INNER_BOARD_SIZE> fillInitBoard();
std::array<uint_fast64_t, HASH_BOARD_LENGTH> populateHashTable();

#endif
