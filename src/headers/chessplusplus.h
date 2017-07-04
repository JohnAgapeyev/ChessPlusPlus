#ifndef CHESSPLUSPLUS_H
#define CHESSPLUSPLUS_H

bool checkMoveInputValid(const std::string& input);
std::string getUserInput();
int getUserInt();
void printHelpText();
void setupGame();
bool configureAI(bool& usingTimeLimit, int& timeLimit, int& plyCount);

void playHumanGame();
void playMixedGame(bool isAIWhite, bool usingTimeLimit, int timeLimit, int plyCount);
void playAIGame(bool usingTimeLimit, int timeLimit, int plyCount);


#endif
