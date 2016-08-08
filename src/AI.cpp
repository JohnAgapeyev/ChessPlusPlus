#include "headers/ai.h"
#include "headers/board.h"
#include "headers/movegenerator.h"
#include "headers/enums.h"

AI::AI(Board& b) : board(b) {
    
}

void AI::evaluate() {
    auto whiteScore = 0;
    auto blackScore = 0;
    
    const auto cornerIndex = board.findCorner_1D();
    
    //Counting material values
    for(int i = 0; i < 8; ++i) {
        for(int j = 0; j < 8; ++j) {
            const auto& currSquare = board.vectorTable[cornerIndex + (i * 15) + j].get();
            const auto& currPiece = currSquare->getPiece();
            if (currPiece && !currSquare->checkSentinel() 
                    && currPiece->getType() != PieceTypes::KING) {
                if (currPiece->getColour() == Colour::WHITE) {
                    whiteScore += getPieceValue(currPiece->getType());
                } else {
                    blackScore += getPieceValue(currPiece->getType());
                }
            }
        }
    }
    
    board.moveGen->generateAll();
    const auto& moveList = board.moveGen->getMoveList();
    const auto totalMoves = moveList.size();
    
    if (board.isWhiteTurn) {
        whiteScore += totalMoves * MOBILITY_VAL;
    } else {
        blackScore += totalMoves * MOBILITY_VAL;
    }
    
    eval = whiteScore - blackScore;
}

int AI::getPieceValue(const PieceTypes type) const {
    switch(type) {
        case PieceTypes::PAWN:
            return PAWN_VAL;
        case PieceTypes::KNIGHT:
            return KNIGHT_VAL;
        case PieceTypes::BISHOP:
            return BISHOP_VAL;
        case PieceTypes::ROOK:
            return ROOK_VAL;
        case PieceTypes::QUEEN:
            return QUEEN_VAL;
        default:
            return 0;
    }
}
