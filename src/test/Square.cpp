class Square {

private:
    bool occupied;
    Piece piece;
    
public:
    Square() : piece(nullptr), occupied(false) {}
    Square(Piece piece) : this.piece(piece), occupied(true) {}
}
