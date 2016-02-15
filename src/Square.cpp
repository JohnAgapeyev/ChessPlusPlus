#include "headers/piece.h"
#include "headers/enums.h"

class Square {

private:
    bool occupied;
    Piece piece;
    
public:
    Square() : piece(nullptr), occupied(false) {}
    Square(Piece piece) : this.piece(piece), occupied(true) {}
};
