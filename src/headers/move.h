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

#ifndef MOVE_H
#define MOVE_H

#include <iostream>
#include "square.h"
#include "piece.h"
#include "enums.h"

struct Move {
    Move();
    Move(const Move& mv) = default;
    Move(Move&& mv) = default;

    Move& operator=(const Move& mv) = default;
    Move& operator=(Move&& mv) = default;
    
    Square *fromSq;
    Square *toSq;
    PieceTypes fromPieceType;
    Colour fromPieceColour;
    bool captureMade;
    PieceTypes toPieceType;
    Colour toPieceColour;
    PieceTypes promotionType;
    bool promotionMade;
    bool isCastle;
    unsigned char castleRights;
    bool enPassantActive;
    Square *enPassantTarget;
    int halfMoveClock;
    int moveCounter;
};

void swapOffsets(const Move& mv);

bool operator==(const Move& first, const Move& second);
bool operator!=(const Move& first, const Move& second);
std::ostream& operator<<(std::ostream& os, const Move& mv);

#endif
