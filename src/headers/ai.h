#ifndef AI_H
#define AI_H

#include "board.h"

class AI {
    static constexpr auto PAWN_VAL = 100;
    static constexpr auto KNIGHT_VAL = 300;
    static constexpr auto BISHOP_VAL = 300;
    static constexpr auto ROOK_VAL = 500;
    static constexpr auto QUEEN_VAL = 900;
    
    static constexpr auto MOBILITY_VAL = 1;
    
    class MoveGenerator;
    Board& board;
    int eval = 0;
    
    
    int reduceKnightMobilityScore(const std::vector<Move>& moveList, const int cornerIndex) const;
    
public:
    AI(Board& b);
    void evaluate();
    auto getEval() const {return static_cast<double>(eval) / 100;}
    int getPieceValue(const PieceTypes type) const;
};

#endif
