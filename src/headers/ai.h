#ifndef AI_H
#define AI_H

#include "board.h"
#include <climits>

class AI {
    static constexpr auto MATE = SHRT_MAX;
    static constexpr auto DRAW = 0;
    
    static constexpr auto PAWN_VAL = 100;
    static constexpr auto KNIGHT_VAL = 300;
    static constexpr auto BISHOP_VAL = 300;
    static constexpr auto ROOK_VAL = 500;
    static constexpr auto QUEEN_VAL = 900;
    
    static constexpr auto MOBILITY_VAL = 1;
    
    static constexpr auto DOUBLED_PAWN_PENALTY = 40;
    static constexpr auto ISOLATED_PAWN_PENALTY = 50;
    static constexpr auto BACKWARD_PAWN_PENALTY = 30;
    static constexpr auto PAWN_SIX_VAL = 70;
    static constexpr auto PAWN_SEVEN_VAL = 100;
    
    static constexpr auto OPEN_FILE_VAL = 20;
    static constexpr auto HALF_OPEN_FILE_VAL = 10;
    static constexpr auto ROOK_SEVEN_VAL = 10;
    
    const int DEPTH = 5;
    
    bool isWhitePlayer = false;
    
    class MoveGenerator;
    Board& board;
    int eval = 0;
    
    
    int reduceKnightMobilityScore(const std::vector<Move>& moveList, const int cornerIndex) const;
    int iterativeDeepening();
    int MTD(const int guess, const int depth);
    int AlphaBeta(const int alpha, const int beta, const int depth);
    
public:
    AI(Board& b);
    void evaluate();
    void search();
    auto getEval() const {return static_cast<double>(eval) / 100;}
    int getPieceValue(const PieceTypes type) const;
};

#endif
