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
#include "hash.h"

class Board {
    /*
     * Convert 15x15 board index that references square on the inner board, and
     * convert it to the relative index of the inner board.
     * Eg. If outerIndex references a2, this method returns the relative position
     * of a2 regardless of the current board shift state.
     */
    static inline constexpr int convertOuterBoardIndex(const int outerIndex, const int cornerIndex) {
        return (((outerIndex - cornerIndex) / OUTER_BOARD_SIZE) * INNER_BOARD_SIZE) 
            + (outerIndex % OUTER_BOARD_SIZE) - (cornerIndex % OUTER_BOARD_SIZE);
    }
    
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
    bool checkBoardValidity() const;
    std::string convertSquareToCoordText(const Square& sq) const;
    std::string convertMoveToCoordText(const Move& mv) const;
    
    size_t getSquareIndex(const Square *sq) const;

    bool checkEnPassantValidity(Square *sq, const Move& mv);

    
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
    bool makeMove(std::string& input);
    bool makeMove(Move& mv);
    void unmakeMove(const Move& mv);
    std::string generateFEN() const;
    bool drawByMaterial() const;
    void setPositionByFEN(const std::string& fen);
    
    friend class std::hash<Board>;
    friend class AI;
};


#endif
