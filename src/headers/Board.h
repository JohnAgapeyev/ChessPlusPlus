#ifndef BOARD_H
#define BOARD_H

#include "square.h"
#include "consts.h"
#include "move.h"
#include <vector>
#include <array>
#include <memory>


class Board {
    class MoveGenerator {
        const Board& board;
        std::vector<Move> moveList;
        
    public:
        MoveGenerator(const Board& b) : board(b) {}
        auto getMoveList() const {return moveList;}
        void generateAll();
        bool validateMove(const Move& mv);
        bool inCheck();
        Move createMove(std::string input);
    };
    
    std::array<std::shared_ptr<Square>, OUTER_BOARD_SIZE * OUTER_BOARD_SIZE> vectorTable;
    void shiftVertical(int count);
    void shiftHorizontal(int count);
    MoveGenerator moveGen;
    
public:
    Board();
    auto& getBoard() const {return vectorTable;}
    auto& getMoveGen() {return moveGen;}
    auto findCorner() const;
    void shiftBoard(int col, int row);
    void makeMove(std::string input);
};

#endif
