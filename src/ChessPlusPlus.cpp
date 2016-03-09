#include "headers/board.h"
#include "headers/square.h"
#include "headers/piece.h"
#include "headers/chessplusplus.h"
#include "headers/consts.h"
#include <iostream>
#include <algorithm>

int main() {
    Board b;
    printBoardState(b);
    for (int i = 0; i < 10; ++i) {
        std::cout << std::endl;
    }
    b.shiftBoard(1, 1);
    printBoardState(b);
    return 0;
}

void printBoardState(const Board b) {
#ifdef DEBUG
    auto range = OUTER_BOARD_SIZE;
#else
    auto range = INNER_BOARD_SIZE;
#endif
    auto table = b.getBoard();
    for (int i = 0; i < range; ++i) {
        for (int i = 0; i < range; ++i) {
#ifdef DEBUG
            std::cout << "--------";
#else
            std::cout << "---";
#endif
        }
        std::cout << '-' << std::endl << "|";
        for (auto j = 0; j < 15; ++j) {
#ifndef DEBUG
            if (table[(i * OUTER_BOARD_SIZE) + j]->checkSentinel()) {
                continue;
            }
#endif
            if (!table[(i * OUTER_BOARD_SIZE) + j]) {
                std::cout << '0' << '|';
            } else {
                std::cout << *table[(i * OUTER_BOARD_SIZE) + j] << '|';
            }
        }
        std::cout << std::endl;
    }
    for (int i = 0; i < range; ++i) {
#ifdef DEBUG
        std::cout << "--------";
#else
        std::cout << "---";
#endif
    }
    std::cout << '-' << std::endl;
}
