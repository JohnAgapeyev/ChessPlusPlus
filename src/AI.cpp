#include "headers/ai.h"
#include "headers/board.h"
#include "headers/consts.h"
#include "headers/movegenerator.h"
#include "headers/enums.h"
#include <climits>
#include <utility>
#include <algorithm>

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
        whiteMoveList = board.moveGen->generateAll();
        board.isWhiteTurn = false;
        blackMoveList = board.moveGen->generateAll();
        board.isWhiteTurn = true;
    } else {
        blackMoveList = board.moveGen->generateAll();
        board.isWhiteTurn = true;
        whiteMoveList = board.moveGen->generateAll();
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
    
    //End game eval conditions
    if (!whiteMoveList.size()) {
        eval = -MATE * board.whiteInCheck;
    } else if (!blackMoveList.size()) {
        eval = MATE * board.blackInCheck;
    } else if (board.halfMoveClock >= 100) {
        eval = 0;
    } else if (board.repititionList[0] == board.repititionList[4] 
            && board.repititionList[4] == board.repititionList[8]) {
        eval = 0;
    } else if (board.drawByMaterial()) {
        eval = 0;
    }
    
    eval = currScore;
}

int AI::reduceKnightMobilityScore(const std::vector<Move>& moveList, const int cornerIndex) const {
    static constexpr int pawnThreatOffsets[] = {14, 16, -14, -16};
    auto totalToRemove = 0;
    auto cornerCheckIndex = 0;
    
    for(const Move& mv : moveList) {
        if (mv.fromPieceType == PieceTypes::KNIGHT) {
            
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
                        && knightMoveNeighbour->getColour() != mv.fromPieceColour) {
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

void AI::search() {
    std::cout << "Search result: " << iterativeDeepening() << std::endl;
}

int AI::iterativeDeepening() {
    int firstGuess = 0;
    for (int i = 0; i < DEPTH; ++i) {
        firstGuess = MTD(firstGuess, i);
    }
    return firstGuess;
}

int AI::MTD(const int firstGuess, const int depth) {
    int currGuess = firstGuess;
    int upper = INT_MAX;
    int lower = INT_MIN;
    int beta = 0;
    
    while (upper > lower) {
        beta = currGuess + (currGuess == lower);
        currGuess = AlphaBeta(beta - 1, beta, depth);
        if (currGuess < beta) {
            upper = currGuess;
        } else {
            lower = currGuess;
        }
    }
    return currGuess;
}

int AI::AlphaBeta(int alpha, int beta, const int depth) {
    int rtn = 0;
    
    if (boardCache.retrieve(board)) {
        const std::pair<int, int>& cachePair = boardCache[board];
        if (cachePair.first >= beta) {
            return cachePair.first;
        }
        if (cachePair.second <= alpha) {
            return cachePair.second;
        }
        alpha = std::max(alpha, cachePair.first);
        beta = std::min(beta, cachePair.second);
    }
    
    if (depth == 0) {
        evaluate();
        rtn = eval;
    } else if (isWhitePlayer == board.isWhiteTurn) {
        rtn = INT_MIN;
        int a = alpha;
        const auto moveList = board.moveGen->generateAll();
        const auto moveListSize = moveList.size();
        
        for (size_t i = 0; rtn < beta && i < moveListSize; ++i) {
            board.makeMove(moveList[i]);
            rtn = std::max(rtn, AlphaBeta(a, beta, depth - 1));
            a = std::max(a, rtn);
            board.unmakeMove(moveList[i]);
        }
    } else {
        rtn = INT_MAX;
        int b = beta;
        const auto moveList = board.moveGen->generateAll();
        const auto moveListSize = moveList.size();
        
        for (size_t i = 0; rtn > alpha && i < moveListSize; ++i) {
            board.makeMove(moveList[i]);
            rtn = std::min(rtn, AlphaBeta(alpha, b, depth - 1));
            b = std::min(b, rtn);
            board.unmakeMove(moveList[i]);
        }
    }
    
    if (rtn <= alpha) {
        //Store rtn as upper bound
        boardCache.add(board, std::make_pair(alpha, rtn));
    }
    if (rtn > alpha && rtn < beta) {
        //Should not happen if using null window, but if it does, store rtn as both upper and lower
        boardCache.add(board, std::make_pair(rtn, rtn));
    }
    if (rtn >= beta ) {
        //Store rtn as lower bound
        boardCache.add(board, std::make_pair(rtn, beta));
    }
    
    return rtn;
}
