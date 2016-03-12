#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <memory>
#include "square.h"
#include "consts.h"
#include "movegenerator.h"

class Board {
    std::array<std::shared_ptr<Square>, OUTER_BOARD_SIZE * OUTER_BOARD_SIZE> vectorTable;
    MoveGenerator moveGen {vectorTable};
    void shiftVertical(int count);
    void shiftHorizontal(int count);
    
public:
    Board();
    auto& getBoard() const {return vectorTable;};
    auto findCorner() const;
    void shiftBoard(int col, int row);
};

#endif
