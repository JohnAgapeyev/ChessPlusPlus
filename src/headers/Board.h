#ifndef BOARD_H
#define BOARD_H

#include <array>

class Board {
    
private:
    //static const auto initialState;
    static const std::array<std::array<int, 15>, 15> vectorTable;
    static constexpr auto fillVectorTable();
    
};

#endif
