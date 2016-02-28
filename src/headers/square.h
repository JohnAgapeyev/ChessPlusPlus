#ifndef SQUARE_H
#define SQUARE_H

#include "piece.h"
#include "enums.h"
#include <memory>
#include <iostream>

class Square {
private:
    std::unique_ptr<Piece> piece;
    int offset;
    
public:
    Square() : piece(nullptr), offset(0) {}
    Square(const int off) : piece(std::make_unique<Piece>(PieceTypes::UNKNOWN, Colour::UNKNOWN)), offset(off) {}
    Square(std::unique_ptr<Piece> p) : piece(std::move(p)), offset(0) {}
    Square(const Piece p) : piece(std::make_unique<Piece>(p)), offset(0) {}
    Square(std::unique_ptr<Piece> p, const int off) : piece(std::move(p)), offset(off) {}
    Square(const Piece p, const int off) : piece(std::make_unique<Piece>(p)), offset(off) {}
    
    void setPiece(std::unique_ptr<Piece> p) {this->piece = std::move(p);}
    void setPiece(Piece p) {this->piece = std::make_unique<Piece>(p);}
    void setOffset(int offset) {this->offset = offset;}
    int getOffset() {return offset;}
    
    friend std::ostream& operator<<(std::ostream& os, const Square& square);
};
#endif
