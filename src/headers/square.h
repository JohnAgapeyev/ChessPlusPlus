#ifndef SQUARE_H
#define SQUARE_H

#include "piece.h"
#include <memory>

class Square {
private:
    std::unique_ptr<Piece> piece;
    int offset;
    
public:
    Square() : piece(nullptr), offset(404) {}
    Square(std::unique_ptr<Piece> p) : piece(std::move(p)), offset(0) {}
    Square(const Piece p) : piece(std::make_unique<Piece>(p)), offset(0) {}
    
    void setPiece(std::unique_ptr<Piece> p) {this->piece = std::move(p);}
    void setPiece(Piece p) {this->piece = std::make_unique<Piece>(p);}
    void setOffset(int offset) {this->offset = offset;}
    int getOffset() {return offset;}
};
#endif
