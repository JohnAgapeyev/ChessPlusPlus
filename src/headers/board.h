#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <array>
#include <memory>
#include <algorithm>
#include "square.h"
#include "consts.h"
#include "move.h"
#include "enums.h"

class Board {
    static constexpr int convertOuterBoardIndex(const int outerIndex, const int cornerIndex);
    
    class MoveGenerator {
        Board& board;
        void logMoveFailure(const int failureNum, const bool isSilent) const;
        
    public:
        MoveGenerator(Board& b) : board(b) {}
        std::vector<Move> generateAll();
        bool validateMove(const Move& mv, const bool isSilent);
        bool inCheck(const Move& mv) const;
        bool inCheck(const int squareIndex) const;
        Move createMove(std::string& input) const;
        int getOffsetIndex(const int offset, const int startIndex = 0, const int vectorLen = 1) const;
        bool getCastleDirectionBool(const PieceTypes type, const Colour pieceColour, const int offset) const;
    };
    MoveGenerator moveGen{*this};
    
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
    std::string promptPromotionType() const;
    void updateCheckStatus();
    bool checkBoardValidity();
    
public:
    Board();
    Board(const Board& b);
    
    bool operator==(const Board& second) const {return currHash == second.currHash;}
    
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
    void setPositionByFEN(const std::string& fen);
    
    friend class std::hash<Board>;
    friend class AI;
};

namespace std {
    template<>
    class hash<Board> {
    public:
        size_t operator() (const Board& b) const;
    };
}


#endif
