#ifndef CHESSPLUSPLUS_H
#define CHESSPLUSPLUS_H

/**
 * These are methods entirely based aorund the text interface and UX design.
 */

bool checkMoveInputValid(const std::string& input);
std::string getUserInput();
int getUserInt();
void printHelpText();
void printInGameHelp();
void printInGameAIHelp();
void setupGame();
bool configureAI(bool& usingTimeLimit, int& timeLimit, int& plyCount);

void playHumanGame();
void playMixedGame(bool isAIWhite, bool usingTimeLimit, int timeLimit, int plyCount);
void playAIGame(bool usingTimeLimit, int timeLimit, int plyCount);


#endif
