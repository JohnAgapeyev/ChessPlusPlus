#ifndef SQUARE_H
#define SQUARE_H

#include "piece.h"

class Square {
private:
    Piece piece;
    int offset;
    
public:
    Square() : offset(404) {}
    Square(Piece piece) : piece(piece), offset(0) {}
    
    void setOffset(int offset) {offset = offset;}
    int getOffset() {return offset;}
};
#endif
