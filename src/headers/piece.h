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

#ifndef PIECE_H
#define PIECE_H

#include <vector>
#include "enums.h"

/**
 * Class representing a piece in the game.
 */
class Piece {
    PieceTypes type;
    Colour pieceColour;

public:
    Piece(const PieceTypes t, const Colour c) : type(t), pieceColour(c) {}
    Piece(const Piece& p) = default;
    Piece(Piece&& p) = default;
    auto getType() const {return type;}
    auto getColour() const {return pieceColour;}
    const std::vector<int>& getVectorList() const;
    int getVectorLength() const;
    void promote(const PieceTypes newType);
    
    friend bool operator==(const Piece& first, const Piece& second);
    friend std::ostream& operator<<(std::ostream& os, const Piece& piece);
    friend class std::hash<Piece>;
};

#endif
