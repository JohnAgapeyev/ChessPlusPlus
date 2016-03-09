#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <memory>
#include "square.h"
#include "consts.h"

class Board {
    std::array<std::shared_ptr<Square>, OUTER_BOARD_SIZE * OUTER_BOARD_SIZE> vectorTable;
    void setVector();
    void shiftVertical(int count);
    void shiftHorizontal(int count);
    
public:
    Board(){setVector();};
    auto& getBoard() const {return vectorTable;};
    auto findCorner();
    void shiftBoard(int col, int row);
};

#endif
