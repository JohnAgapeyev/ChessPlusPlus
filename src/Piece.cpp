#include <algorithm>
#include <cctype>
#include <iostream>
#include <vector>
#include <cassert>
#include "headers/piece.h"
#include "headers/consts.h"

std::vector<int> Piece::getVectorList() const {
    std::vector<int> rtn;
    switch(this->type) {
        case PieceTypes::PAWN:
            if (this->pieceColour == Colour::WHITE) {
                rtn = {14, 15, 16, 30};
            } else {
                rtn = {-14, -15, -16, -30};
            }
            break;
        case PieceTypes::KNIGHT:
            rtn = {13, 29, 31, 17, -13, -29, -31, -17};
            break;
        case PieceTypes::BISHOP:
            rtn = {14, 16, -14, -16};
            break;
        case PieceTypes::KING:
            rtn = {15, 1, -15, -1, 14, 16, -14, -16, 2, -2};
            break;
        case PieceTypes::QUEEN:
            rtn = {15, 1, -15, -1, 14, 16, -14, -16};
            break;
        case PieceTypes::ROOK:
            rtn = {15, 1, -15, -1};
            break;
        case PieceTypes::UNKNOWN:
            rtn = {15, 1, -15, -1, 14, 16, -14, -16, 13, 29, 31, 17, -13, -29, -31, -17};
            break;
        default:
            return std::vector<int>();
    }
    
    std::sort(rtn.begin(), rtn.end(), [](const auto first, const auto second){
        return std::abs(first) > std::abs(second);
    });
    
    return rtn;
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

int Piece::getVectorLength() const {
    switch (this->type) {
        case PieceTypes::KING:
        case PieceTypes::PAWN:
        case PieceTypes::KNIGHT:
            return 2;
        default:
            return INNER_BOARD_SIZE;
    }
}

void Piece::promote(PieceTypes newType) {
    assert(type == PieceTypes::PAWN);
    type = newType;
}
