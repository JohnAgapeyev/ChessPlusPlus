#include "headers/square.h"
#include "headers/board.h"
#include <vector>
#include <array>
#include <iostream>
#include <memory>

void Board::setVector() {
    int count = -112;
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            vectorTable[i][j] = std::unique_ptr<Square>(new Square());
            vectorTable[i][j]->setOffset(count++);
        }
    }
}
