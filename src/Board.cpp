#include "headers/enums.h"
#include "headers/piece.h"
#include "headers/square.h"
#include "headers/board.h"
#include <vector>
#include <array>
#include <iostream>
#include <memory>

const std::array<std::array<std::unique_ptr<Square>, 15>, 15> Board::vectorTable = [](){
    int count = -112;
    std::array<std::array<std::unique_ptr<Square>, 15>, 15> output{0};
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            output[i][j] = std::unique_ptr<Square>(new Square());
            output[i][j]->setOffset(count++);
        }
    }
    return output;
};
