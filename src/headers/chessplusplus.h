/**
* This is part of ChessPlusPlus, a C++14 Chess AI
* Copyright (C) 2017 John Agapeyev
* 
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

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
