class Board {

private:
    static const std::array<std::array<int, 15>, 15> vectorTable = fillVectorTable(vectorTable);
    
    const std::array<std::array<Square, 8>, 8> boardSquares;
    
    //See if the following loops can be replaced with iterator
    static constexpr void fillVectorTable(auto& out) {
        static int count = -112;
        for (int i = 0; i < 15; ++i) {
            for (int j = 0; j < 15; ++j) {
                out[i][j] = count++;
            }
        }
    }
    
    void init() {
        
    }
    
public:
    Board() {init();}
    Board(auto board) : boardSquares(board) {}
    
}
