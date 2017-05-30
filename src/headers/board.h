#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <array>
#include <memory>
#include "hash.h"
//#include "square.h"
#include "consts.h"
//#include "move.h"
#include "enums.h"

class Square;
struct Move;

class Board {
    /*
     * Convert 15x15 board index that references square on the inner board, and
     * convert it to the relative index of the inner board.
     * Eg. If outerIndex references a2, this method returns the relative position
     * of a2 regardless of the current board shift state.
     */
    static constexpr int convertOuterBoardIndex(const int outerIndex, const int cornerIndex) {
        return (((outerIndex - cornerIndex) / OUTER_BOARD_SIZE) * INNER_BOARD_SIZE) 
            + (outerIndex % OUTER_BOARD_SIZE) - (cornerIndex % OUTER_BOARD_SIZE);
    }

    static constexpr int genOffset(const int i, const int j) {
        return 98 - (15 * i) + j;
    }
    
    class MoveGenerator {
        Board& board;
        void logMoveFailure(const int failureNum, const bool isSilent) const;
        int getMoveOffset(const Move& mv) const;
        
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
    int cornerCache = -1;
    
    void shiftVertical(const int count);
    void shiftHorizontal(const int count);
    std::string promptPromotionType() const;
    void updateCheckStatus();
    bool checkBoardValidity();
    std::string convertSquareToCoordText(const Square *sq);
    std::string convertMoveToCoordText(const Move& mv);
    int getSquareIndex(const Square *sq);
    bool checkEnPassantValidity(Square *sq, const Move& mv);
    void removeCastlingRights(const unsigned char flag);
    void disableCastling(const Move& mv);
    void performCastling(Move& mv, const int offset, const int fromSquareIndex);
    void addEnPassantTarget(const Move& mv, const int offset, const int columnNum, const int endSquareIndex);
    void captureEnPassant(const Move& mv, const int offset, const int toSquareIndex);
    void hashPieceChange(const int index, const PieceTypes type);
    void promotePawn(Move& mv, const int endSquareIndex, const bool isSilent);

public:
    Board();
    Board(const Board& b);
    
    bool operator==(const Board& second) const {return currHash == second.currHash;}
    
    void printBoardState() const;
    auto getBoard() const {return vectorTable;}
    auto getGameState() const {return currentGameState;}
    auto getCurrHash() const {return currHash;}
    std::pair<int, int> findCorner();
    int findCorner_1D();
    void shiftBoard(const int col, const int row);
    bool makeMove(std::string& input);
    bool makeMove(Move& mv);
    void unmakeMove(const Move& mv);
    std::string generateFEN();
    bool drawByMaterial();
    void setPositionByFEN(const std::string& fen);
    
    friend class std::hash<Board>;
    friend class AI;
};


#endif
