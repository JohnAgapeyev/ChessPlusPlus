#incldue <enums.h>

template<Pieces P>
class Piece {
    
private:
    Colour pieceColour;
    P type;
    
public:
    Piece() {}
    Piece(P type, Colour col) : this.type(type), pieceColour(col) {}
    
}
