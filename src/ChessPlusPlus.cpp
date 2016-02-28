#include <iostream>
#include "headers/board.h"
#include "headers/square.h"
#include "headers/piece.h"
#include "headers/chessplusplus.h"

int main() {
    Board b;
    b.setVector();
    printBoardStateDebug(b);
    return 0;
}

void printBoardStateDebug(Board b) {
    for (const auto& row : b.getBoard()) {
        for (int i = 0; i < 15; ++i) {
            std::cout << "------";
        }
        std::cout << '-' << std::endl;
        std::cout << "|";
        for (const auto& sq : row) {
            std::cout << *sq << "|";
        }
        std::cout << std::endl;
    }
    for (int i = 0; i < 15; ++i) {
        std::cout << "------";
    }
    std::cout << '-' << std::endl;
}
