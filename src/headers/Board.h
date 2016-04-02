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
        Move createMove(std::string& input);
    };
    MoveGenerator moveGen;
    std::array<std::shared_ptr<Square>, OUTER_BOARD_SIZE * OUTER_BOARD_SIZE> vectorTable;
    bool isWhiteTurn = true;
    bool enPassantActive = false;
    Square *enPassantTarget = nullptr;
    void shiftVertical(const int count);
    void shiftHorizontal(const int count);
    bool ensureEnPassantValid() const;
    
public:
    Board();
    void printBoardState() const;
    auto getBoard() const {return vectorTable;}
    auto getMoveGen() const {return moveGen;}
    const std::pair<int, int> findCorner() const;
    void shiftBoard(const int col, const int row);
    void makeMove(std::string& input);
};

#endif
