#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "move.h"
#include <vector>

class MoveGenerator {
    const std::array<std::shared_ptr<Square>, 
        OUTER_BOARD_SIZE * OUTER_BOARD_SIZE>& board;
    std::vector<Move> moveList;
    
    
public:
    MoveGenerator(
        const std::array<std::shared_ptr<Square>,
            OUTER_BOARD_SIZE * OUTER_BOARD_SIZE>& table) : board(table) {}
            
    auto getMoveList() const {return moveList;}
    
    void generateAll();
    bool validateMove(Move mv);
    bool inCheck();
    
    
};

#endif
