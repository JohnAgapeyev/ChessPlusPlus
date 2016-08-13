#ifndef BOARD_H
#define BOARD_H

#include "square.h"
#include "consts.h"
#include "move.h"
#include "enums.h"
#include <vector>
#include <array>
#include <memory>
#include <algorithm>

class Board {
    class MoveGenerator;
    std::unique_ptr<MoveGenerator> moveGen;
    std::array<std::shared_ptr<Square>, OUTER_BOARD_SIZE * OUTER_BOARD_SIZE> vectorTable;
    GameState currentGameState = GameState::ACTIVE;
    unsigned char castleRights = 0x0F;
    bool blackInCheck = false;
    bool whiteInCheck = false;
    bool isWhiteTurn = true;
    bool enPassantActive = false;
    Square *enPassantTarget = nullptr;
    int halfMoveClock = 0;
    int moveCounter = 1;
    size_t currHash = 0;
    std::array<size_t, 9> repititionList;
    void shiftVertical(const int count);
    void shiftHorizontal(const int count);
    void ensureEnPassantValid() const;
    int convertOuterBoardIndex(const int outerIndex, const int cornerIndex) const;
    std::string promptPromotionType() const;
    void updateCheckStatus();
    
public:
    Board();
    Board(const Board&) : Board() {}
    ~Board();
    void printBoardState() const;
    auto getBoard() const {return vectorTable;}
    auto getGameState() const {return currentGameState;}
    auto getCurrHash() const {return currHash;}
    std::pair<int, int> findCorner() const;
    int findCorner_1D() const;
    void shiftBoard(const int col, const int row);
    void makeMove(std::string& input);
    void makeMove(Move mv);
    void unmakeMove(const Move& mv);
    std::string generateFEN() const;
    bool drawByMaterial() const;
    
    friend class std::hash<Board>;
    friend class AI;
    friend bool operator==(const Board& first, const Board& second);
};

namespace std {
    template<>
    class hash<Board> {
    public:
        size_t operator() (const Board& b) const;
    };
}


#endif
