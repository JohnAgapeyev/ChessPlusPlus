#include <iostream>
#include <string>
#include <algorithm>
#include <regex>
#include <cstdlib>
#include <unistd.h>
#include <omp.h>
#include "headers/board.h"
#include "headers/chessplusplus.h"
#include "headers/ai.h"

int main(int argc, char **argv) {
    if (!omp_get_cancellation()) {
        putenv((char *)"OMP_CANCELLATION=true");
        execv(argv[0], argv);
    }
    std::string input;

    std::cout << "Welcome to ChessPlusPlus!\n" << "This program is a Chess AI written in C++14 and OpenMP.\n" 
        << "For more information on this program, go to www.github.com/JohnAgapeyev/ChessPlusPlus\n\n";
        
    for (;;) {
        std::cout << "Enter your command, or enter help to see a list of possible commands: ";
        input = getUserInput(); 
        if (!input.compare("exit")) {
            break;
        }
        if (!input.compare("benchmark")) {
            Board b;
            AI comp(&b);
            comp.benchmarkPerft();
            continue;
        }
        if (!input.compare("help")) {
            printHelpText();
            continue;
        }
        if (!input.compare("game")) {
            setupGame();
            //continue;
        }
    }
    return 0;
}

bool checkMoveInputValid(const std::string& input) {
    // Match 4 letter inputs that are either 1-8 or alternate a-h, 1-8
    std::regex reg("([a-h][1-8]){2}|[1-8]{4}");
    return std::regex_match(input, reg);
}

std::string getUserInput() {
    std::string input;
    std::getline(std::cin, input);
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);
    return input;
}

int getUserInt() {
    int output;
    for (;;) {
        if (std::cin >> output && (output > 0 || output == -1)) {
            break;
        }
        std::cout << "Please enter either a valid number corresponding to your desired choice, or -1 to return to the main menu\n";
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    return output;
}

void printHelpText() {
    std::cout << "There are 4 commands available for use in this program:\n" 
        << "game - Configures and launches a chess game against either another local player, or the AI\n"
        << "benchmark - For testing the speed and correctness of the chess engine behind the AI using a set of known test positions\n"
        << "help - To display this help message\n"
        << "exit - To exit the program\n";
}

void setupGame() {
    std::string input;
    for (;;) {
        std::cout << "Enter your game type:\n1 - Human vs Human; 2 - Human vs AI; 3 - AI vs AI\n";
        input = getUserInput(); 
        if (!input.compare("1")) {
            //Human v human game
            playHumanGame();
            return;
        } else if (!input.compare("2")) {
            //Human v AI
            bool isAIWhite = false;
            bool usingTimeLimit = false;
            int timeLimit = -2;
            int plyCount = -2;
            if (!configureAI(usingTimeLimit, timeLimit, plyCount)) {
                return;
            }
            for (;;) {
                input = getUserInput(); 
                std::cout << "Should the AI play as [w]hite or as [b]lack?\n";
                input = getUserInput(); 
                if (!input.compare("w")) {
                    isAIWhite = true;
                    break;
                } else if (!input.compare("b")) {
                    isAIWhite = false;
                    break;
                } else if (!input.compare("cancel")) {
                    return;
                } else {
                    std::cout << "Please enter either w or b corresponding to the desired AI starting colour, or cancel to return to the main menu\n";
                }
            }
            playMixedGame(isAIWhite, usingTimeLimit, timeLimit, plyCount);
            return;
        } else if (!input.compare("3")) {
            //AI v AI
            bool usingTimeLimit = false;
            int timeLimit = -2;
            int plyCount = -2;
            if (!configureAI(usingTimeLimit, timeLimit, plyCount)) {
                return;
            }
            playAIGame(usingTimeLimit, timeLimit, plyCount);
            return;
        } else if (!input.compare("cancel")) {
            return;
        } else {
            std::cout << "Please enter either a 1, 2, or 3 corresponding to your desired game choice, or cancel to cancel the game setup\n";
        }
    }
}

bool configureAI(bool& usingTimeLimit, int& timeLimit, int& plyCount) {
    std::string input;
    for (;;) {
        std::cout << "Should the AI(s) use a move time limit [y/n]? ";
        input = getUserInput(); 
        if (!input.compare("y")) {
            //Use limit
            usingTimeLimit = true;
            break;
        } else if (!input.compare("n")) {
            //Infinite time
            usingTimeLimit = false;
            break;
        } else if (!input.compare("cancel")) {
            return false;
        } else {
            std::cout << "Please enter either y or n corresponding to your desired choice, or cancel to return to the main menu\n";
        }
    }
    if (usingTimeLimit) {
        std::cout << "How long in seconds should the AI's per-move time limit be? (Minimum 1 second): ";
        timeLimit = getUserInt();
    }
    if (timeLimit == -1) {
        return false;
    }
    std::cout << "How many plys should the AI search? (Minimum 1 ply): ";
    plyCount = getUserInt();
    if (plyCount == -1) {
        return false;
    }
    return true;
}

void playHumanGame() {
    Board b;
    b.printBoardState();
    std::string input;
    while (b.getGameState() == GameState::ACTIVE) {
        std::cout << "Enter your move: ";
        input = getUserInput();
        if (!input.compare("exit")) {
            break;
        }
        if (!checkMoveInputValid(input)) {
            std::cout << "Not a valid move format\n";
            continue;
        }
        if (b.makeMove(input)) {
            b.printBoardState();
        }
    }
}

void playMixedGame(bool isAIWhite, bool usingTimeLimit, int timeLimit, int plyCount) {
    Board b;
    AI comp(&b);
    if (usingTimeLimit) {
        comp.setMoveTimeLimit(timeLimit);
        comp.setDepth(plyCount);
    } else {
        comp.setInfiniteMode(true);
        comp.setDepth(plyCount);
    }
    b.printBoardState();
    std::string input;
    while (b.getGameState() == GameState::ACTIVE) {
        if (isAIWhite) {
            //AI is white
            comp.search();
            b.printBoardState();
            for (;;) {
                std::cout << "Enter your move: ";
                input = getUserInput();
                if (!input.compare("exit")) {
                    return;
                }
                if (checkMoveInputValid(input) && b.makeMove(input)) {
                    b.printBoardState();
                    break;
                }
                std::cout << "Not a valid move format\n";
            }
        } else {
            //AI is black
            std::cout << "Enter your move: ";
            input = getUserInput();
            if (!input.compare("exit")) {
                return;
            }
            if (!checkMoveInputValid(input)) {
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
}

void playAIGame(bool usingTimeLimit, int timeLimit, int plyCount) {
    Board b;
    AI comp1(&b);
    AI comp2(&b);
    if (usingTimeLimit) {
        comp1.setMoveTimeLimit(timeLimit);
        comp1.setDepth(plyCount);
        comp2.setMoveTimeLimit(timeLimit);
        comp2.setDepth(plyCount);
    } else {
        comp1.setInfiniteMode(true);
        comp1.setDepth(plyCount);
        comp2.setInfiniteMode(true);
        comp2.setDepth(plyCount);
    }
    b.printBoardState();
    while (b.getGameState() == GameState::ACTIVE) {
        comp1.search();
        b.printBoardState();
        comp2.search();
        b.printBoardState();
    }
}

