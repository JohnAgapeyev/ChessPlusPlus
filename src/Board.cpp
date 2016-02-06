class Board {

private:
    std::array<std::array<Square, 8>, 8> boardSquares;
    
    void init() {
        
    }
    
public:
    Board() {init();}
    Board(auto board) : boardSquares(board) {}
    
}
