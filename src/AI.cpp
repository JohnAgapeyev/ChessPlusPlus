#include "headers/ai.h"
#include "headers/board.h"
#include "headers/consts.h"
#include "headers/movegenerator.h"
#include "headers/enums.h"
#include <climits>
#include <algorithm>
#include <tuple>
#include <utility>

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
    for(int i = 0; i < INNER_BOARD_SIZE; ++i) {
        for(int j = 0; j < INNER_BOARD_SIZE; ++j) {
            const auto& currSquare = board.vectorTable[cornerIndex + (i * OUTER_BOARD_SIZE) + j].get();
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
    //const auto result = iterativeDeepening();
    //board.makeMove(board.moveGen->generateAll()[result.first]);
    for (int i = 0; i <= 6; ++i) {
        std::cout << "Perft at depth " << i << ": " << perft(i) << std::endl;
    }
}

std::pair<int, int> AI::iterativeDeepening() {
    auto firstGuess = std::make_pair(INT_MIN, 0);
    for (int i = 0; i < DEPTH; ++i) {
        firstGuess = MTD(firstGuess.second, i);
    }
    return firstGuess;
}

std::pair<int, int> AI::MTD(const int firstGuess, const int depth) {
    auto currGuess = std::make_pair(INT_MIN, firstGuess);
    int upper = INT_MAX;
    int lower = INT_MIN;
    int beta = 0;
    
    while (upper > lower) {
        beta = std::max(currGuess.second, lower + 1);
        currGuess = AlphaBeta(beta - 1, beta, depth);
        if (currGuess.second < beta) {
            upper = currGuess.second;
        } else {
            lower = currGuess.second;
        }
    }
    return currGuess;
}

std::pair<int, int> AI::AlphaBeta(int alpha, int beta, const int depth) {
    auto rtn = std::make_pair(INT_MIN, 0);
        
    const auto moveList = board.moveGen->generateAll();
    const auto moveListSize = moveList.size();
    
    if (boardCache.retrieve(board.currHash)) {
        int entryDepth;
        int entryValue;
        SearchBoundary entryType;
        std::tie(entryDepth, entryValue, entryType, rtn.first) = boardCache[board.currHash];
        if (entryDepth >= depth) {
            if (entryType == SearchBoundary::EXACT) {
                return std::make_pair(rtn.first, entryValue);
            }
            //Update the best move based on the previous value, not sure how yet
            if (entryType == SearchBoundary::LOWER && entryValue > alpha) {
                alpha = entryValue;
                
            } else if (entryType == SearchBoundary::UPPER && entryValue < beta) {
                beta = entryValue;
                
            }
            if (alpha >= beta) {
                //Return the best move as well
                return std::make_pair(rtn.first, entryValue);
            }
        }
    }
    
    if (depth == 0) {
        evaluate();
        rtn.second = eval;
    } else if (isWhitePlayer == board.isWhiteTurn) {
        rtn.first = 0;
        rtn.second = INT_MIN;
        int a = alpha;
        
        auto cmp = std::make_pair(0, 0);
        
        for (size_t i = 0; rtn.second < beta && i < moveListSize; ++i) {
            board.makeMove(moveList[i]);
            
            
            cmp = AlphaBeta(a, beta, depth - 1);
            if (cmp.second > rtn.second) {
                rtn.first = i;
                rtn.second = cmp.second;
            }
            a = std::max(a, rtn.second);
            board.unmakeMove(moveList[i]);
        }
    } else {
        rtn.first = 0;
        rtn.second = INT_MAX;
        int b = beta;
        
        auto cmp = std::make_pair(0, 0);
        
        for (size_t i = 0; rtn.second > alpha && i < moveListSize; ++i) {
            board.makeMove(moveList[i]);
            
            
            cmp = AlphaBeta(alpha, b, depth - 1);
            if (cmp.second < rtn.second) {
                rtn.first = i;
                rtn.second = cmp.second;
            }
            b = std::min(b, rtn.second);
            board.unmakeMove(moveList[i]);
        }
    }
    
    if (rtn.second <= alpha) {
        //Store rtn as upper bound
        boardCache.add(board.currHash, std::make_tuple(depth, rtn.second, SearchBoundary::UPPER, rtn.first));
    } else if (rtn.second > alpha && rtn.second < beta) {
        //Should not happen if using null window, but if it does, store rtn as both upper and lower
        boardCache.add(board.currHash, std::make_tuple(depth, rtn.second, SearchBoundary::EXACT, rtn.first));
    } else if (rtn.second >= beta ) {
        //Store rtn as lower bound
        boardCache.add(board.currHash, std::make_tuple(depth, rtn.second, SearchBoundary::LOWER, rtn.first));
    }
    
    return rtn;
}

unsigned long long AI::perft(int depth) {
    unsigned long long nodeCount = 0;
    const auto& moveList = board.moveGen->generateAll();
    const auto& moveListSize = moveList.size();
    
    if (depth == 1) {
        return moveListSize;
    }
    
    for (size_t i = 0; i < moveListSize; ++i) {
        board.makeMove(moveList[i]);
        nodeCount += perft(depth - 1);
        board.unmakeMove(moveList[i]);
    }
    return nodeCount;
}
