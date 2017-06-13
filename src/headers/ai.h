#ifndef AI_H
#define AI_H

#include <climits>
#include "move.h"
#include "board.h"
#include "tt.h"
#include "consts.h"

class AI {
    static constexpr auto MATE = SHRT_MAX;
    static constexpr auto DRAW = 0;
    
    static constexpr auto PAWN_VAL = 100;
    static constexpr auto KNIGHT_VAL = 300;
    static constexpr auto BISHOP_VAL = 300;
    static constexpr auto ROOK_VAL = 500;
    static constexpr auto QUEEN_VAL = 900;
    static constexpr auto KING_VAL = 3000;
    
    static constexpr auto MOBILITY_VAL = 1;
    
    static constexpr auto DOUBLED_PAWN_PENALTY = 40;
    static constexpr auto ISOLATED_PAWN_PENALTY = 50;
    static constexpr auto BACKWARD_PAWN_PENALTY = 30;
    static constexpr auto PASSED_PAWN_VAL = 30;
    static constexpr auto PAWN_SIX_VAL = 70;
    static constexpr auto PAWN_SEVEN_VAL = 100;
    
    static constexpr auto OPEN_FILE_VAL = 20;
    static constexpr auto HALF_OPEN_FILE_VAL = 10;
    static constexpr auto ROOK_SEVEN_VAL = 10;
    
    static constexpr auto CASTLE_BONUS = 30;
    
    static std::unordered_multimap<Piece, std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>> initializeMap();
    
    static const std::unordered_multimap<Piece, std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>> pieceSquareTables;
    
    typedef Cache<Board, std::tuple<int, int, SearchBoundary, Move>, 1024 * 1024 * 512> cache_pointer_type;
    static std::unique_ptr<cache_pointer_type> boardCache;
    
    std::array<Move, 6 * INNER_BOARD_SIZE * INNER_BOARD_SIZE> counterMove; 
    
    const int DEPTH = 6;
    
    bool isWhitePlayer = false;
    
    bool hasBlackCastled = false;
    bool hasWhiteCastled = false;
    
    Board& board;
    
    int eval = 0;
    
    Move prev = Move();
    
    int reduceKnightMobilityScore(const std::vector<Move>& moveList, const int cornerIndex) const;
    std::pair<Move, int> iterativeDeepening();
    std::pair<Move, int> MTD(const int guess, const int depth);
    std::pair<Move, int> AlphaBeta(const int alpha, const int beta, const int depth);
    int getPieceValue(const PieceTypes type) const;
    unsigned long long perft(int depth);
    unsigned long long perftDivide(int depth);
    std::vector<Move> orderMoveList(std::vector<Move>&& list);
    
public:
    AI(Board& b) : board(b) {}
    void evaluate();
    void search();
    auto getEval() const {return static_cast<double>(eval) / 100;}
    void benchmarkPerft();
};

#endif
