#ifndef SQUARE_H
#define SQUARE_H

#include <memory>
#include <iostream>
#include "piece.h"
#include "enums.h"

class Square {
private:
    std::unique_ptr<Piece> piece;
    int offset;
    
public:
    Square() : piece(nullptr), offset(0) {}
    Square(const int off) : piece(std::make_unique<Piece>(PieceTypes::UNKNOWN, Colour::UNKNOWN)), offset(off) {}
    Square(const Piece p, const int off = 0) : piece(std::make_unique<Piece>(p)), offset(off) {}
    Square(std::unique_ptr<Piece> p, const int off = 0) : piece(std::move(p)), offset(off) {}
    
    auto setPiece(std::unique_ptr<Piece> p) {piece = std::move(p);}
    auto setPiece(Piece p) {piece = std::make_unique<Piece>(p);}
    auto setOffset(int off) {offset = off;}
    auto getOffset() const {return offset;}
    auto getPiece() const {return piece.get();}
    
    bool checkSentinel();
    
    friend std::ostream& operator<<(std::ostream& os, const Square& square);
    friend bool operator==(const Square& first, const Square& second);
};
#endif
