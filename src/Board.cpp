#include "headers/square.h"
#include "headers/board.h"
#include "headers/consts.h"
#include <vector>
#include <array>
#include <iostream>
#include <memory>

void Board::setVector() {
    int count = -112;
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            if (i >= 0 && i <= 7) {
                if (j >= 0 && j <= 7) {
                    vectorTable[i][j] = INIT_BOARD[i][j];
                    vectorTable[i][j]->setOffset(count++);
                } else {
                    vectorTable[i][j] = std::make_shared<Square>(count++);
                }
            } else {
                vectorTable[i][j] = std::make_shared<Square>(count++);
            }
        }
    }
}
