#ifndef PIECE_H
#define PIECE_H

#include <vector>
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

#endif
