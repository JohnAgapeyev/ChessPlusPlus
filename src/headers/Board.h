#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <memory>
#include "square.h"

class Board {
    std::array<std::array<std::shared_ptr<Square>, 15>, 15> vectorTable;
    void setVector();
    
public:
    Board(){setVector();};
    auto& getBoard() const {return vectorTable;};
    auto findCorner();
    void shiftBoard(int col, int row);
    void shiftVertical(int count);
    void shiftHorizontal(int count);
    
};

#endif
