
#include "headers/piece.h"
#include <algorithm>

Piece::Piece(const PieceTypes t, const Colour c) : type(t), pieceColour(c) {
    switch(t) {
        case PieceTypes::PAWN:
            if (c == Colour::WHITE) {
                vecOffsets.push_back(14);
                vecOffsets.push_back(15);
                vecOffsets.push_back(16);
            } else {
                vecOffsets.push_back(-14);
                vecOffsets.push_back(-15);
                vecOffsets.push_back(-16);
            }
            break;
        case PieceTypes::KNIGHT:
            vecOffsets.push_back(13);
            vecOffsets.push_back(29);
            vecOffsets.push_back(31);
            vecOffsets.push_back(17);
            vecOffsets.push_back(-13);
            vecOffsets.push_back(-29);
            vecOffsets.push_back(-31);
            vecOffsets.push_back(-17);
            break;
        case PieceTypes::BISHOP:
            vecOffsets.push_back(14);
            vecOffsets.push_back(16);
            vecOffsets.push_back(-14);
            vecOffsets.push_back(-16);
            break;
        case PieceTypes::ROOK:
            vecOffsets.push_back(15);
            vecOffsets.push_back(1);
            vecOffsets.push_back(-15);
            vecOffsets.push_back(-1);
            break;
        case PieceTypes::QUEEN:
        case PieceTypes::KING:
            vecOffsets.push_back(15);
            vecOffsets.push_back(1);
            vecOffsets.push_back(-15);
            vecOffsets.push_back(-1);
            vecOffsets.push_back(14);
            vecOffsets.push_back(16);
            vecOffsets.push_back(-14);
            vecOffsets.push_back(-16);
            break;
        default:
            vecOffsets.push_back(0);
            break;
    }
    std::sort(vecOffsets.begin(), vecOffsets.end());
}
