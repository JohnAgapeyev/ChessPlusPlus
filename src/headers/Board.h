#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <memory>

class Board {
    static const std::array<std::array<std::unique_ptr<Square>, 15>, 15> vectorTable;
    
};

#endif
