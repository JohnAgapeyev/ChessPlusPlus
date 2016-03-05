#include "headers/board.h"
#include "headers/square.h"
#include "headers/piece.h"
#include "headers/chessplusplus.h"
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
    const int range = 15;
#else
    const int range = 8;
#endif
    for (const auto& row : b.getBoard()) {
#ifndef DEBUG
        if (std::count_if(row.cbegin(), row.cend(), [](auto sq){
                    auto pc = sq->getPiece();
                    return (pc
                            && (pc->getColour() == Colour::UNKNOWN
                            || pc->getType() == PieceTypes::UNKNOWN)
                    );
                    }) == 15) {
            continue;
        }
#endif
        for (int i = 0; i < range; ++i) {
#ifdef DEBUG
            std::cout << "--------";
#else
            std::cout << "---";
#endif
        }
        std::cout << '-' << std::endl << "|";
        for (const auto& sq : row) {
#ifndef DEBUG
            auto pc = sq->getPiece();
            if (pc && (pc->getColour() == Colour::UNKNOWN || pc->getType() == PieceTypes::UNKNOWN)) {
                continue;
            }
#endif
            std::cout << *sq << '|';
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
