#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <regex>
#include <unistd.h>
#include <omp.h>
#include "headers/board.h"
#include "headers/square.h"
#include "headers/piece.h"
#include "headers/chessplusplus.h"
#include "headers/consts.h"
#include "headers/tt.h"
#include "headers/ai.h"

int main(int argc, char **argv) {
    if (!omp_get_cancellation()) {
      printf("Cancellations were not enabled, enabling cancellation and rerunning program\n");
      putenv("OMP_CANCELLATION=true");
      execv(argv[0], argv);
    }
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
        if (!input.compare("benchmark")) {
            comp.benchmarkPerft();
        }
        if (!checkInputValid(input)) {
            std::cout << "Not a valid move format\n";
            continue;
        }
        if (b.makeMove(input)) {
            b.printBoardState();
            comp.search();
            b.printBoardState();
        }
    }
}

bool checkInputValid(const std::string& input) {
    // Match 4 letter inputs that are either 1-8 or alternate a-h, 1-8
    std::regex reg("([a-h][1-8]){2}|[1-8]{4}");
    return std::regex_match(input, reg);
}
