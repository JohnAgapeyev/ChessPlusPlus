#include <iostream>
#include <cassert>
#include "headers/piece.h"
#include "headers/consts.h"

const std::vector<int> Piece::getVectorList() const {
    switch(type) {
        case PieceTypes::PAWN:
            if (pieceColour == Colour::WHITE) {
                return {14, 15, 16, 30};
            }
            return {-14, -15, -16, -30};
        case PieceTypes::KNIGHT:
            return {13, 29, 31, 17, -13, -29, -31, -17};
        case PieceTypes::BISHOP:
            return {14, 16, -14, -16};
        case PieceTypes::KING:
            return {15, 1, -15, -1, 14, 16, -14, -16, 2, -2};
        case PieceTypes::QUEEN:
            return {15, 1, -15, -1, 14, 16, -14, -16};
        case PieceTypes::ROOK:
            return {15, 1, -15, -1};
        case PieceTypes::UNKNOWN:
            return {15, 1, -15, -1, 14, 16, -14, -16, 13, 29, 31, 17, -13, -29, -31, -17};
        default:
            return std::vector<int>();
    }
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
