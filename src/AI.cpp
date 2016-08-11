#include "headers/ai.h"
#include "headers/board.h"
#include "headers/consts.h"
#include "headers/movegenerator.h"
#include "headers/enums.h"

AI::AI(Board& b) : board(b) {
    
}

/**
 * Evaluates the current board state from the perspective of the current player
 * to move with a positive number favouring white, and a negative one favouring black.
 */
void AI::evaluate() {
    auto currScore = 0;
    std::vector<Move> whiteMoveList;
    std::vector<Move> blackMoveList;
    
    std::vector<int> whiteRookFiles;
    std::vector<int> blackRookFiles;
    int filePawnCount[16] = {0};
    
    if (board.isWhiteTurn) {
        board.moveGen->generateAll();
        whiteMoveList = board.moveGen->getMoveList();
        board.isWhiteTurn = false;
        board.moveGen->generateAll();
        blackMoveList = board.moveGen->getMoveList();
        board.isWhiteTurn = true;
    } else {
        board.moveGen->generateAll();
        blackMoveList = board.moveGen->getMoveList();
        board.isWhiteTurn = true;
        board.moveGen->generateAll();
        whiteMoveList = board.moveGen->getMoveList();
        board.isWhiteTurn = false;
    }

    const auto whiteTotalMoves = whiteMoveList.size();
    const auto blackTotalMoves = blackMoveList.size();
    
    const auto cornerIndex = board.findCorner_1D();
    
    currScore += (whiteTotalMoves - blackTotalMoves) * MOBILITY_VAL;
    
    currScore -= reduceKnightMobilityScore(whiteMoveList, cornerIndex);
    currScore += reduceKnightMobilityScore(blackMoveList, cornerIndex);
    
    //Counting material values
    for(int i = 0; i < 8; ++i) {
        for(int j = 0; j < 8; ++j) {
            const auto& currSquare = board.vectorTable[cornerIndex + (i * 15) + j].get();
            const auto& currPiece = currSquare->getPiece();
            if (currPiece && !currSquare->checkSentinel() 
                    && currPiece->getType() != PieceTypes::KING) {
                if (currPiece->getColour() == Colour::WHITE) {
                    currScore += getPieceValue(currPiece->getType());
                    if (currPiece->getType() == PieceTypes::ROOK) {
                        if (i == 1) {
                            currScore += ROOK_SEVEN_VAL;
                        }
                        whiteRookFiles.push_back(j);
                    }
                    if (currPiece->getType() == PieceTypes::PAWN) {
                        ++filePawnCount[j];
                        if (i == 1) {
                            currScore += PAWN_SEVEN_VAL;
                        } else if (i == 2) {
                            currScore += PAWN_SIX_VAL;
                        }
                    }
                } else {
                    currScore -= getPieceValue(currPiece->getType());
                    if (currPiece->getType() == PieceTypes::ROOK) {
                        if (i == 6) {
                            currScore -= ROOK_SEVEN_VAL;
                        }
                        blackRookFiles.push_back(j);
                    }
                    if (currPiece->getType() == PieceTypes::PAWN) {
                        ++filePawnCount[j + INNER_BOARD_SIZE];
                        if (i == 6) {
                            currScore -= PAWN_SEVEN_VAL;
                        } else if (i == 5) {
                            currScore -= PAWN_SIX_VAL;
                        }
                    }
                }
            }
        }
    }
    
    for (int i = 0; i < 16; ++i) {
        if (filePawnCount[i] > 1) {
            if (i > 7) {
                currScore += DOUBLED_PAWN_PENALTY * (filePawnCount[i] - 1);
            } else {
                currScore -= DOUBLED_PAWN_PENALTY * (filePawnCount[i] - 1);
            }
        } else if (!filePawnCount[i]) {
            if (i <= 13 && filePawnCount[i + 1] && !filePawnCount[i + 2]) {
                if (i > 7) {
                    currScore += ISOLATED_PAWN_PENALTY;
                } else {
                    currScore -= ISOLATED_PAWN_PENALTY;
                }
            }
        }
        if (i <= 7) {
            if (!filePawnCount[i] || !filePawnCount[i + INNER_BOARD_SIZE]) {
                if (!filePawnCount[i] && !filePawnCount[i + INNER_BOARD_SIZE]) {
                    //Open file
                    for (const int currFile : whiteRookFiles) {
                        if (currFile == i) {
                            currScore += OPEN_FILE_VAL;
                        }
                    }
                    for (const int currFile : blackRookFiles) {
                        if (currFile == i) {
                            currScore -= OPEN_FILE_VAL;
                        }
                    }
                } else {
                    //Semi open
                    for (const int currFile : whiteRookFiles) {
                        if (currFile == i) {
                            currScore += HALF_OPEN_FILE_VAL;
                        }
                    }
                    for (const int currFile : blackRookFiles) {
                        if (currFile == i) {
                            currScore -= HALF_OPEN_FILE_VAL;
                        }
                    }
                }
            }
        }
    }
    
    eval = currScore;
}

int AI::reduceKnightMobilityScore(const std::vector<Move>& moveList, const int cornerIndex) const {
    static constexpr int pawnThreatOffsets[] = {14, 16, -14, -16};
    auto totalToRemove = 0;
    auto cornerCheckIndex = 0;
    
    for(const Move& mv : moveList) {
        if (mv.fromPiece->getType() == PieceTypes::KNIGHT) {
            
            //Calculate index of destination square without requiring linear search
            const auto cornerToSqDiff = board.moveGen->getOffsetIndex(
                mv.toSq->getOffset() - board.vectorTable[cornerIndex]->getOffset(), cornerIndex);
                
            for (int i = 0; i < 4; ++i) {
                cornerCheckIndex = board.moveGen->getOffsetIndex(pawnThreatOffsets[i], cornerToSqDiff);
                if (cornerCheckIndex < 0 || cornerCheckIndex >= OUTER_BOARD_SIZE * OUTER_BOARD_SIZE) {
                    continue;
                }
                
                const auto& knightMoveNeighbour = board.vectorTable[cornerCheckIndex]->getPiece();
                
                if (knightMoveNeighbour && knightMoveNeighbour->getType() == PieceTypes::PAWN
                        && knightMoveNeighbour->getColour() != mv.fromPiece->getColour()) {
                    totalToRemove += MOBILITY_VAL;
                }
            }
        }
    }
    return totalToRemove;
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
