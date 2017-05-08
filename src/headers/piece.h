#ifndef PIECE_H
#define PIECE_H

#include <array>
#include <vector>
#include <iostream>
#include "enums.h"

class Piece {
    PieceTypes type;
    Colour pieceColour;

public:
    Piece(const PieceTypes t, const Colour c) : type(t), pieceColour(c) {}
    auto getType() const {return type;}
    auto getColour() const {return pieceColour;}
    const std::vector<int> getVectorList() const;
    int getVectorLength() const;
    void promote(const PieceTypes newType);
    
    friend bool operator==(const Piece& first, const Piece& second);
    friend std::ostream& operator<<(std::ostream& os, const Piece& piece);
    friend class std::hash<Piece>;
};

namespace std {
    template<>
    class hash<Piece> {
    public:
        size_t operator() (const Piece& p) const;
    };
}

#endif
