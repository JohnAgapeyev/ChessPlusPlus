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
#include <cassert>
#include "headers/piece.h"
#include "headers/consts.h"

const std::vector<int>& Piece::getVectorList() const {
    static const std::vector<int> whitePawn{14, 15, 16, 30};
    static const std::vector<int> blackPawn{-14, -15, -16, -30};
    static const std::vector<int> knight{13, 29, 31, 17, -13, -29, -31, -17};
    static const std::vector<int> bishop{14, 16, -14, -16};
    static const std::vector<int> king{15, 1, -15, -1, 14, 16, -14, -16, 2, -2};
    static const std::vector<int> queen{15, 1, -15, -1, 14, 16, -14, -16};
    static const std::vector<int> rook{15, 1, -15, -1};
    static const std::vector<int> unknown{15, 1, -15, -1, 14, 16, -14, -16, 13, 29, 31, 17, -13, -29, -31, -17};
    switch(type) {
        case PieceTypes::PAWN:
            if (pieceColour == Colour::WHITE) {
                return whitePawn;
            }
            return blackPawn;
        case PieceTypes::KNIGHT:
            return knight;
        case PieceTypes::BISHOP:
            return bishop;
        case PieceTypes::KING:
            return king;
        case PieceTypes::QUEEN:
            return queen;
        case PieceTypes::ROOK:
            return rook;
        case PieceTypes::UNKNOWN:
            return unknown;
    }
    __builtin_unreachable(); //Will never have a piece without the above type
}

std::ostream& operator<<(std::ostream& os, const Piece& piece) {
    if (piece.pieceColour == Colour::UNKNOWN || piece.type == PieceTypes::UNKNOWN) {
#ifndef NDEBUG
        return os << " x ";
#else
        return os << "";
#endif
    }
#ifndef NDEBUG
    os << ' ';
#endif
    return os << static_cast<char>(piece.pieceColour) << static_cast<char>(piece.type);
}

bool operator==(const Piece& first, const Piece& second) {
    return first.type == second.type && first.pieceColour == second.pieceColour;
}

int Piece::getVectorLength() const {
    switch (type) {
        case PieceTypes::KING:
        case PieceTypes::PAWN:
        case PieceTypes::KNIGHT:
            return 2;
        default:
            return INNER_BOARD_SIZE;
    }
}

void Piece::promote(const PieceTypes newType) {
    assert(type == PieceTypes::PAWN);
    type = newType;
}
