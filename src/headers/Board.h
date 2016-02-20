#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <memory>

class Board {
    
    std::array<std::array<std::unique_ptr<Square>, 15>, 15> vectorTable;
    
    void setVector();
    
};

#endif
