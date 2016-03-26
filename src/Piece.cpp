
#include "headers/piece.h"
#include <algorithm>
#include <cctype>
#include <iostream>

Piece::Piece(const PieceTypes t, const Colour c) : type(t), pieceColour(c) {
    switch(t) {
        case PieceTypes::PAWN:
            if (c == Colour::WHITE) {
                vecOffsets.push_back(14);
                vecOffsets.push_back(15);
                vecOffsets.push_back(16);
                vecOffsets.push_back(30);
            } else {
                vecOffsets.push_back(-14);
                vecOffsets.push_back(-15);
                vecOffsets.push_back(-16);
                vecOffsets.push_back(-30);
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
    std::sort(vecOffsets.begin(), vecOffsets.end(), [](auto& first, auto& second){
        return std::abs(first) > std::abs(second);
        });
    //if (t == PieceTypes::PAWN) {
        //std::for_each(vecOffsets.cbegin(), vecOffsets.cend(), [](auto& elem){std::cout << elem << std::endl;});
        //std::cout << std::endl;
    //}
}
