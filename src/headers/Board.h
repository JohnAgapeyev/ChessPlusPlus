#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <memory>
#include "square.h"

class Board {
    
    std::array<std::array<std::shared_ptr<Square>, 15>, 15> vectorTable;
    
    //void setVector();
    
public:
    auto& getBoard(){return vectorTable;};
    void setVector();
};

#endif
