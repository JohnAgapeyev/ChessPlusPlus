#ifndef BOARD_H
#define BOARD_H

#include "square.h"
#include "consts.h"
#include "move.h"
#include <vector>
#include <array>
#include <memory>

class Board {
    class MoveGenerator;
    std::unique_ptr<MoveGenerator> moveGen;
    std::array<std::shared_ptr<Square>, OUTER_BOARD_SIZE * OUTER_BOARD_SIZE> vectorTable;
    bool isWhiteTurn = true;
    bool enPassantActive = false;
    Square *enPassantTarget = nullptr;
    void shiftVertical(const int count);
    void shiftHorizontal(const int count);
    bool ensureEnPassantValid() const;
    
public:
    Board();
    ~Board();
    void printBoardState() const;
    auto getBoard() const {return vectorTable;}
    auto getMoveGen() const {return moveGen.get();}
    const std::pair<int, int> findCorner() const;
    void shiftBoard(const int col, const int row);
    void makeMove(std::string& input);
};

#endif
