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

#include <array>
#include <memory>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <unordered_map>
#include "headers/square.h"
#include "headers/enums.h"
#include "headers/consts.h"

const std::array<std::array<std::unique_ptr<Square>, INNER_BOARD_SIZE>, INNER_BOARD_SIZE> INIT_BOARD = fillInitBoard();
const std::array<uint_fast64_t, HASH_BOARD_LENGTH> HASH_VALUES = populateHashTable();
const unsigned char BLACK_CASTLE_FLAG = 0b1100;
const unsigned char WHITE_CASTLE_FLAG = 0b0011;
const unsigned char WHITE_CASTLE_QUEEN_FLAG = 0b0010;
const unsigned char WHITE_CASTLE_KING_FLAG = 0b0001;
const unsigned char BLACK_CASTLE_QUEEN_FLAG = 0b1000;
const unsigned char BLACK_CASTLE_KING_FLAG = 0b0100;

/**
 * This method populates the global initial board state.
 * I had originally planned early on in development for the board to be reset back to this internal state using
 * a reset method in the board class.
 * This never really became feasible, and as such this init board is merely used to initially construct a board.
 */
std::array<std::array<std::unique_ptr<Square>, INNER_BOARD_SIZE>, INNER_BOARD_SIZE> fillInitBoard() {
    const auto& pickColour = [](const auto T){return (T < 5) ? Colour::BLACK : Colour::WHITE;};
    std::array<std::array<std::unique_ptr<Square>, INNER_BOARD_SIZE>, INNER_BOARD_SIZE> result;
    for (int i = 0; i < INNER_BOARD_SIZE; ++i) {
        std::array<std::unique_ptr<Square>, INNER_BOARD_SIZE> row;
        for (int j = 0; j < INNER_BOARD_SIZE; ++j) {
            switch(i) {
                case 0:
                case 7:
                    switch(j) {
                        case 0:
                        case 7:
                            row[j] = std::make_unique<Square>(Piece(PieceTypes::ROOK, pickColour(i)));
                            break;
                        case 1:
                        case 6:
                            row[j] = std::make_unique<Square>(Piece(PieceTypes::KNIGHT, pickColour(i)));
                            break;
                        case 2:
                        case 5:
                            row[j] = std::make_unique<Square>(Piece(PieceTypes::BISHOP, pickColour(i)));
                            break;
                        case 3:
                            row[j] = std::make_unique<Square>(Piece(PieceTypes::QUEEN, pickColour(i)));
                            break;
                        case 4:
                            row[j] = std::make_unique<Square>(Piece(PieceTypes::KING, pickColour(i)));
                            break;
                    }
                    break;
                case 1:
                case 6:
                    row[j] = std::make_unique<Square>(Piece(PieceTypes::PAWN, pickColour(i)));
                    break;
                default:
                    row[j] = std::make_unique<Square>();
                    break;
            }
        }
        result[i] = std::move(row);
    }
    return result;
}

/**
 * Populates the hash table values used by zobrist hashing.
 */
std::array<uint_fast64_t, HASH_BOARD_LENGTH> populateHashTable() {
    std::array<uint_fast64_t, HASH_BOARD_LENGTH> result;
    std::mt19937_64 gen(std::random_device{}());
    std::uniform_int_distribution<uint_fast64_t> uni;
    for (int i = 0; i < HASH_BOARD_LENGTH; ++i) {
        result[i] = uni(gen);
    }
    return result;
}
