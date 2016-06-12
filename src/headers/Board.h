#ifndef BOARD_H
#define BOARD_H

#include "square.h"
#include "consts.h"
#include "move.h"
#include "enums.h"
#include <vector>
#include <array>
#include <memory>

class Board {
    class MoveGenerator;
    std::unique_ptr<MoveGenerator> moveGen;
    std::array<std::shared_ptr<Square>, OUTER_BOARD_SIZE * OUTER_BOARD_SIZE> vectorTable;
    GameState currentGameState = GameState::ACTIVE;
    bool whiteCastleKing = true;
    bool whiteCastleQueen = true;
    bool blackCastleKing = true;
    bool blackCastleQueen = true;
    bool blackInCheck = false;
    bool whiteInCheck = false;
    bool isWhiteTurn = true;
    bool enPassantActive = false;
    Square *enPassantTarget = nullptr;
    int halfMoveClock = 0;
    int moveCounter = 1;
    std::array<std::string, 5> repititionList;
    void shiftVertical(const int count);
    void shiftHorizontal(const int count);
    void ensureEnPassantValid() const;
    
public:
    Board();
    ~Board();
    void printBoardState() const;
    auto getBoard() const {return vectorTable;}
    auto getGameState() const {return currentGameState;}
    std::pair<int, int> findCorner() const;
    int findCorner_1D() const;
    void shiftBoard(const int col, const int row);
    void makeMove(std::string& input);
    std::string generateFEN() const;
};

#endif
