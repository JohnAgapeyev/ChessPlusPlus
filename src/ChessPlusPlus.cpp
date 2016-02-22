#include <iostream>
#include "headers/board.h"
#include "headers/square.h"
#include "headers/piece.h"

int main() {
    std::cout << "Hello World!" << std::endl;
    Board b;
    for (const auto& row : b.getBoard()) {
        for (const auto& sq : row) {
            std::cout << sq.get() << std::endl;
        }
    }
    return 0;
}
