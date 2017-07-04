#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <array>
#include <memory>
#include "hash.h"
#include "consts.h"
#include "enums.h"
#include "move.h"

class Square;

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
        Board *board;
        std::vector<Move> moveList;
        std::vector<std::tuple<int, int, Piece*>> pieceCoords;
        void logMoveFailure(const int failureNum, const bool isSilent) const;
        int getMoveOffset(const Move& mv) const;
        
    public:
        MoveGenerator(Board *b) : board(b) {moveList.reserve(100); pieceCoords.reserve(16);}
        MoveGenerator(const MoveGenerator& m) = default;
        MoveGenerator(MoveGenerator&& m) = default;
        MoveGenerator& operator=(const MoveGenerator& m) = default;
        MoveGenerator& operator=(MoveGenerator&& m) = default;

        bool operator==(const MoveGenerator& second) const {return *board == *second.board && moveList == second.moveList && pieceCoords == second.pieceCoords;}

        static constexpr int getOffsetIndex(const int offset, const int startIndex = 0, const int vectorLen = 1) {
            const auto absOffset = std::abs(offset);
            const auto offsetVal = (vectorLen * ((30 * ((absOffset + INNER_BOARD_SIZE - 1) / OUTER_BOARD_SIZE)) - absOffset));
            return startIndex + offsetVal - ((offset > 0) * offsetVal * 2);
        }

        std::vector<Move> generateAll();
        bool validateMove(const Move& mv, const bool isSilent);
        bool inCheck(const Move& mv) const;
        bool inCheck(const int squareIndex) const;
        Move createMove(std::string& input) const;
        bool getCastleDirectionBool(const PieceTypes type, const Colour pieceColour, const int offset) const;
    };
    MoveGenerator moveGen{this};
    
    std::array<std::unique_ptr<Square>, TOTAL_BOARD_SIZE> vectorTable;
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
    void hashPieceChange(const int index, const PieceTypes type, const Colour colour);
    void hashTurnChange();
    void hashEnPassantFile(const int fileNum);
    void hashCastleRights();
    void promotePawn(Move& mv, const int endSquareIndex, const bool isSilent);
    void detectGameEnd();

public:
    Board();
    Board(const Board& b);
    Board(Board&& b) = default;
    
    Board& operator=(const Board& b);
    Board& operator=(Board&& b) = default;
    bool operator==(const Board& second) const {return currHash == second.currHash;}
    
    void printBoardState() const;
    auto& getBoard() const {return vectorTable;}
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
    void setGameState(const GameState state) {currentGameState = state;}
    auto getWhiteTurn() const {return isWhiteTurn;}
    
    friend class std::hash<Board>;
    friend class AI;
};


#endif
