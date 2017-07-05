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

#ifndef SQUARE_H
#define SQUARE_H

#include <memory>
#include <iostream>
#include "piece.h"
#include "enums.h"

/**
 * Class representing a square on the board.
 */
class Square {
    std::unique_ptr<Piece> piece;
    int offset;
    
public:
    Square() = default;
    Square(const Square& sq) : piece((sq.piece) ? std::make_unique<Piece>(*sq.piece) : nullptr), offset(sq.offset) {}
    Square(Square&& sq) = default;
    Square(const int off) : piece(std::make_unique<Piece>(PieceTypes::UNKNOWN, Colour::UNKNOWN)), offset(off) {}
    Square(const Piece p, const int off = 0) : piece(std::make_unique<Piece>(p)), offset(off) {}
    Square(std::unique_ptr<Piece> p, const int off = 0) : piece(std::move(p)), offset(off) {}
    
    void setPiece(std::unique_ptr<Piece> p) {piece = std::move(p);}
    void setPiece(const Piece& p) {piece = std::make_unique<Piece>(p);}
    void setOffset(const int off) {offset = off;}
    auto getOffset() const {return offset;}
    auto getPiece() const {return piece.get();}
    auto releasePiece() {return piece.release();}
    
    bool checkSentinel() const {
        return (getPiece() && getPiece()->getColour() == Colour::UNKNOWN 
            && getPiece()->getType() == PieceTypes::UNKNOWN);
    }

    bool operator==(const Square& second) {
        return ((offset == second.offset) && !piece && !second.piece) 
            || ((offset == second.offset) && piece && second.piece && *piece == *second.piece);
    }

    bool operator!=(const Square& second) {
        return !(*this == second);
    }

    Square& operator=(const Square& sq) {
        piece = (sq.piece) ? std::make_unique<Piece>(*sq.piece) : nullptr;
        offset = sq.offset;
        return *this;
    }

    Square& operator=(Square&& sq) {
        piece = std::move(sq.piece);
        offset = sq.offset;
        return *this;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Square& square);
};

#endif
