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

#include <algorithm>
#include <utility>
#include <functional>
#include <cassert>
#include "headers/board.h"
#include "headers/hash.h"

/**
 * Default zobrist hashing implemenation.
 * Due to the incremental nature of zobrist hashing, each board
 * caches its hash value.
 * Therefore, this method is only ever called in constructors, or
 * for debug assertions to confirm the incremental updates are equivalent
 * to this full hash.
 */
size_t std::hash<Board>::operator() (Board& b) const {
    if (b.currHash) {
        return b.currHash;
    }
    
    const auto& cornerCoords = b.findCorner();
    const int cornerIndex = (cornerCoords.first * OUTER_BOARD_SIZE) + cornerCoords.second;
    size_t newHash = 0;
    
    for (int i = cornerCoords.first; i < cornerCoords.first + INNER_BOARD_SIZE; ++i) {
        for (int j = cornerCoords.second; j < cornerCoords.second + INNER_BOARD_SIZE; ++j) {
            //Black is (white hash + 6) for equivalent piece types
            if (b.vectorTable[(i * OUTER_BOARD_SIZE) + j]->getPiece()) {
                const auto currPiece = b.vectorTable[(i * OUTER_BOARD_SIZE) + j]->getPiece();
                assert(pieceLookupTable.find(currPiece->getType()) != pieceLookupTable.end());
                newHash ^= HASH_VALUES[NUM_SQUARE_STATES 
                    * b.convertOuterBoardIndex((i * OUTER_BOARD_SIZE) + j, cornerIndex)
                    + pieceLookupTable.find(currPiece->getType())->second
                    + ((currPiece->getColour() == Colour::WHITE) ? 0 : 6)];
            }
        }
    }
    if (b.isWhiteTurn) {
        newHash ^= HASH_VALUES[static_cast<unsigned int>(SquareState::WHITE_MOVE)];
    }
    
    newHash ^= HASH_VALUES[static_cast<unsigned int>(SquareState::CASTLE_RIGHTS) + b.castleRights];
    
    if (b.enPassantActive) {
         //Calculate en passant file
        const auto targetIndex = std::distance(b.vectorTable.cbegin(), 
            std::find_if(b.vectorTable.cbegin(), b.vectorTable.cend(), 
            [&b](const auto& sq){
                return *sq == *b.enPassantTarget;
            }
        ));
        const int fileNum = Board::convertOuterBoardIndex(targetIndex, cornerIndex) % INNER_BOARD_SIZE;
        newHash ^= HASH_VALUES[static_cast<unsigned int>(SquareState::EN_PASSANT_FILE) + fileNum];
    }
    return newHash;
}

size_t std::hash<Piece>::operator()(const Piece& p) const {
    return std::hash<char>()(static_cast<char>(p.type)) ^ std::hash<char>()(static_cast<char>(p.pieceColour));
}

