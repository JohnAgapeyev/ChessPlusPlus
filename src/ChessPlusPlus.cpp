#include "headers/board.h"
#include "headers/square.h"
#include "headers/piece.h"
#include "headers/chessplusplus.h"
#include "headers/consts.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <regex>

int main() {
    Board b;
    printBoardState(b);
    for (int i = 0; i < 10; ++i) {
        std::cout << std::endl;
    }
    b.shiftBoard(1, 1);
    printBoardState(b);
    for (int i = 0; i < 10; ++i) {
        std::cout << std::endl;
    }
    mainLoop();
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

void mainLoop() {
    std::string input;
    for (;;) {
        std::cout << "Enter your move: ";
        std::getline(std::cin, input);
        for (auto& ch : input) {
            ch = std::tolower(ch);
        }
        // Input equals exit
        if (!input.compare("exit")) {
            break;
        }
        if (!checkMoveValid(input)) {
            std::cout << "Not a valid move" << std::endl;
            continue;
        }
    }
}

bool checkMoveValid(std::string input) {
    // Match 4 letter inputs that are either 1-8 or alternate a-h, 1-8
    std::regex reg("([a-h][1-8]){2}|[1-8]{4}");
    return std::regex_match(input, reg);
}
