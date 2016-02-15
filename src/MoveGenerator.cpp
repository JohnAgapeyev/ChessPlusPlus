#include "headers/enums.h"
#include "headers/move.h"
#include "headers/square.h"
#include <vector>
#include <array>

class MoveGenerator {
    
public:
    //std::vector<Move> generate(const PieceTypes& piece, 
    void generate(const PieceTypes& piece, 
        std::array<std::array<int, 15>, 15>& vectorTable, 
        std::array<std::array<Square, 8>, 8>& board) {
        return;
    }
    
};
