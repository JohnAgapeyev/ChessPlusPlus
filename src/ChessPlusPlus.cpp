#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <regex>
#include "headers/board.h"
#include "headers/square.h"
#include "headers/piece.h"
#include "headers/chessplusplus.h"
#include "headers/consts.h"
#include "headers/tt.h"
#include "headers/ai.h"

int main() {
    Board b;
    b.printBoardState();
    mainLoop(b);
    return 0;
}

void mainLoop(Board& b) {
    std::string input;
    AI comp(b);
    while (b.getGameState() == GameState::ACTIVE) {
        std::cout << "Enter your move: ";
        std::getline(std::cin, input);
        std::transform(input.begin(), input.end(), input.begin(), ::tolower);

        if (!input.compare("exit")) {
            break;
        }
        if (!checkInputValid(input)) {
            std::cout << "Not a valid move format" << std::endl;
            continue;
        }
        //b.makeMove(input);
        comp.search();
        b.printBoardState();
    }
}

bool checkInputValid(const std::string& input) {
    // Match 4 letter inputs that are either 1-8 or alternate a-h, 1-8
    std::regex reg("([a-h][1-8]){2}|[1-8]{4}");
    return std::regex_match(input, reg);
}
