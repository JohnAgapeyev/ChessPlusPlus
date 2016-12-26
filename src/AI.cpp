#include <climits>
#include <algorithm>
#include <tuple>
#include <utility>
#include <cassert>
#include "headers/ai.h"
#include "headers/board.h"
#include "headers/consts.h"
#include "headers/enums.h"

const std::unordered_multimap<Piece, 
    std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>> 
    AI::pieceSquareTables = AI::initializeMap();

/**
 * Evaluates the current board state from the perspective of the current player
 * to move with a positive number favouring white, and a negative one favouring black.
 */
void AI::evaluate() {
    int currScore = 0;
    std::vector<Move> whiteMoveList;
    std::vector<Move> blackMoveList;
    
    std::vector<int> whiteRookFiles;
    std::vector<int> blackRookFiles;
    
    //Array to store pawn counts on a per-file basis with white being 0-7, black 8-15
    std::array<int, 16> filePawnCount{{0}};
    
    if (board.isWhiteTurn) {
        whiteMoveList = board.moveGen.generateAll();
        board.isWhiteTurn = false;
        blackMoveList = board.moveGen.generateAll();
        board.isWhiteTurn = true;
    } else {
        blackMoveList = board.moveGen.generateAll();
        board.isWhiteTurn = true;
        whiteMoveList = board.moveGen.generateAll();
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
            
            if (currPiece) {
                if (currPiece->getType() != PieceTypes::KING) {
                    //Non-king piece square table lookup
                    const auto& elem = pieceSquareTables.find(*currPiece);
                    
                    if (currPiece->getColour() == Colour::WHITE) {
                        //Material value
                        currScore += getPieceValue(currPiece->getType());
                        //Piece square lookup
                        currScore += elem->second[Board::convertOuterBoardIndex(cornerIndex + (i * OUTER_BOARD_SIZE) + j, cornerIndex)];
                        
                        //Check for rook on the seventh and store the file
                        if (currPiece->getType() == PieceTypes::ROOK) {
                            if (i == 1) {
                                currScore += ROOK_SEVEN_VAL;
                            }
                            whiteRookFiles.push_back(j);
                        }
                        //Check for pawn on the sixth and seventh, and store the file
                        if (currPiece->getType() == PieceTypes::PAWN) {
                            ++filePawnCount[j];
                            if (i == 1) {
                                currScore += PAWN_SEVEN_VAL;
                            } else if (i == 2) {
                                currScore += PAWN_SIX_VAL;
                            }
                        }
                    } else {
                        //Material value
                        currScore -= getPieceValue(currPiece->getType());
                        //Piece square lookup
                        currScore -= elem->second[Board::convertOuterBoardIndex(cornerIndex + (i * OUTER_BOARD_SIZE) + j, cornerIndex)];
                        
                        //Check for rook on the seventh and store the file
                        if (currPiece->getType() == PieceTypes::ROOK) {
                            if (i == 6) {
                                currScore -= ROOK_SEVEN_VAL;
                            }
                            blackRookFiles.push_back(j);
                        }
                        //Check for pawn on the sixth and seventh, and store the file
                        if (currPiece->getType() == PieceTypes::PAWN) {
                            ++filePawnCount[j + INNER_BOARD_SIZE];
                            if (i == 6) {
                                currScore -= PAWN_SEVEN_VAL;
                            } else if (i == 5) {
                                currScore -= PAWN_SIX_VAL;
                            }
                        }
                    }
                } else {
                    //Handle piece square table access for king
                    const auto& elemRange = pieceSquareTables.equal_range(*currPiece);
                    //Material value
                    currScore += getPieceValue(currPiece->getType());
                    //Piece square lookup
                    currScore += elemRange.first->second[Board::convertOuterBoardIndex(cornerIndex + (i * OUTER_BOARD_SIZE) + j, cornerIndex)];
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
            const auto cornerToSqDiff = board.moveGen.getOffsetIndex(
                mv.toSq->getOffset() - board.vectorTable[cornerIndex]->getOffset(), cornerIndex);
                
            for (int i = 0; i < 4; ++i) {
                cornerCheckIndex = board.moveGen.getOffsetIndex(pawnThreatOffsets[i], cornerToSqDiff);
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

std::pair<Move, int> AI::iterativeDeepening() {
    auto firstGuess = std::make_pair(Move(), 0);
    for (int i = 0; i < DEPTH; ++i) {
        firstGuess = MTD(firstGuess.second, i);
    }
    return firstGuess;
}

std::pair<Move, int> AI::MTD(const int firstGuess, const int depth) {
    auto currGuess = std::make_pair(Move(), firstGuess);
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

std::pair<Move, int> AI::AlphaBeta(int alpha, int beta, const int depth) {
    assert(depth >= 0);
    auto rtn = std::make_pair(Move(), INT_MIN);
        
    const auto& moveList = board.moveGen.generateAll();
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
        rtn = std::make_pair(Move(), eval);
    } else if (isWhitePlayer == board.isWhiteTurn) {
        rtn.first = Move();
        rtn.second = INT_MIN;
        int a = alpha;
        
        for (size_t i = 0; rtn.second < beta && i < moveListSize; ++i) {
            board.makeMove(moveList[i]);
            const auto& abCall = AlphaBeta(a, beta, depth - 1);
            
            if (abCall.second >= rtn.second) {
                rtn.first = moveList[i];
                rtn.second = abCall.second;
            }
            a = std::max(a, rtn.second);
            board.unmakeMove(moveList[i]);
        }
    } else {
        rtn.first = Move();
        rtn.second = INT_MAX;
        int b = beta;
        
        for (size_t i = 0; rtn.second > alpha && i < moveListSize; ++i) {
            board.makeMove(moveList[i]);
            const auto& abCall = AlphaBeta(alpha, b, depth - 1);
            
            if (abCall.second <= rtn.second) {
                rtn.first = moveList[i];
                rtn.second = abCall.second;
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
    const auto& moveList = board.moveGen.generateAll();
    const auto& moveListSize = moveList.size();
    
    switch(depth) {
        case 0:
            return 1;
        case 1:
            return moveListSize;
    }
    
    for (size_t i = 0; i < moveListSize; ++i) {
        board.makeMove(moveList[i]);
        nodeCount += perft(depth - 1);
        board.unmakeMove(moveList[i]);
    }
    return nodeCount;
}

unsigned long long AI::perftDivide(int depth) {
    unsigned long long nodeCount = 0;
    const auto& moveList = board.moveGen.generateAll();
    const auto& moveListSize = moveList.size();
    
    if (!depth) {
        return 1;
    }
    
    for (size_t i = 0; i < moveListSize; ++i) {
        std::cout << board.convertMoveToCoordText(moveList[i]) << "\t";
        board.makeMove(moveList[i]);
        const auto perftResult = perft(depth - 1);
        std::cout << perftResult << "\n";
        nodeCount += perftResult;
        board.unmakeMove(moveList[i]);
    }
    return nodeCount;
}

std::unordered_multimap<Piece, std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>> AI::initializeMap() {
    std::unordered_multimap<Piece, std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>> tempMap;
    
    tempMap.emplace(std::make_pair(Piece(PieceTypes::PAWN, Colour::WHITE), 
        std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>{{
              0,  0,  0,  0,  0,  0,  0,  0,
             50, 50, 50, 50, 50, 50, 50, 50,
             10, 10, 20, 30, 30, 20, 10, 10,
              5,  5, 10, 25, 25, 10,  5,  5,
              0,  0,  0, 20, 20,  0,  0,  0,
              5, -5, 10,  0,  0,-10, -5,  5,
              5, 10, 10,-20,-20, 10, 10, 50,
              0,  0,  0,  0,  0,  0,  0,  0
        }}));
    tempMap.emplace(std::make_pair(Piece(PieceTypes::BISHOP, Colour::WHITE), 
        std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>{{
            -20,-10,-10,-10,-10,-10,-10,-20,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -10,  0,  5, 10, 10,  5,  0,-10,
            -10,  5,  5, 10, 10,  5,  5,-10,
            -10,  0, 10, 10, 10, 10,  0,-10,
            -10, 10, 10, 10, 10, 10, 10,-10,
            -10,  5,  0,  0,  0,  0,  5,-10,
            -20,-10,-10,-10,-10,-10,-10,-20
        }}));
    tempMap.emplace(std::make_pair(Piece(PieceTypes::KNIGHT, Colour::WHITE), 
        std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>{{
            -50,-40,-30,-30,-30,-30,-40,-50,
            -40,-20,  0,  0,  0,  0,-20,-40,
            -30,  0, 10, 15, 15, 10,  0,-30,
            -30,  5, 15, 20, 20, 15,  5,-30,
            -30,  0, 15, 20, 20, 15,  0,-30,
            -30,  5, 10, 15, 15, 10,  5,-30,
            -40,-20,  0,  5,  5,  0,-20, 50,
            -50,-40,-30,-30,-30,-30,-40,-50
        }}));
    tempMap.emplace(std::make_pair(Piece(PieceTypes::ROOK, Colour::WHITE), 
        std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>{{
              0,  0,  0,  0,  0,  0,  0,  0,
              5, 10, 10, 10, 10, 10, 10,  5,
             -5,  0,  0,  0,  0,  0,  0, -5,
             -5,  0,  0,  0,  0,  0,  0, -5,
             -5,  0,  0,  0,  0,  0,  0, -5,
             -5,  0,  0,  0,  0,  0,  0, -5,
             -5,  0,  0,  0,  0,  0,  0, -5,
              0,  0,  0,  5,  5,  0,  0,  0
        }}));
    tempMap.emplace(std::make_pair(Piece(PieceTypes::KING, Colour::WHITE), 
        std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>{{
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -20,-30,-30,-40,-40,-30,-30,-20,
            -10,-20,-20,-20,-20,-20,-20,-10,
             20, 20,  0,  0,  0,  0, 20, 20,
             20, 30, 10,  0,  0, 10, 30, 20
        }}));
    tempMap.emplace(std::make_pair(Piece(PieceTypes::KING, Colour::WHITE), 
        std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>{{
            -50,-40,-30,-20,-20,-30,-40,-50,
            -30,-20,-10,  0,  0,-10,-20,-30,
            -30,-10, 20, 30, 30, 20,-10,-30,
            -30,-10, 30, 40, 40, 30,-10,-30,
            -30,-10, 30, 40, 40, 30,-10,-30,
            -30,-10, 20, 30, 30, 20,-10,-30,
            -30,-30,  0,  0,  0,  0,-30,-30,
            -50,-30,-30,-30,-30,-30,-30,-50
        }}));
    tempMap.emplace(std::make_pair(Piece(PieceTypes::QUEEN, Colour::WHITE), 
        std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>{{
             -20,-10,-10, -5, -5,-10,-10,-20,
             -10,  0,  0,  0,  0,  0,  0,-10,
             -10,  0,  5,  5,  5,  5,  0,-10,
              -5,  0,  5,  5,  5,  5,  0, -5,
               0,  0,  5,  5,  5,  5,  0, -5,
             -10,  5,  5,  5,  5,  5,  0,-10,
             -10,  0,  5,  0,  0,  0,  0,-10,
             -20,-10,-10, -5, -5,-10,-10,-20
        }}));
        
    for (size_t i = 0; i < 6; ++i) {
        PieceTypes type;
        switch(i) {
            case 0:
                type = PieceTypes::PAWN;
                break;
            case 1:
                type = PieceTypes::BISHOP;
                break;
            case 2:
                type = PieceTypes::KNIGHT;
                break;
            case 3:
                type = PieceTypes::ROOK;
                break;
            case 4:
                type = PieceTypes::KING;
                break;
            case 5:
                type = PieceTypes::QUEEN;
                break;
        }
        //Reverse piece square tables for black pieces
        auto previousValue = tempMap.find(Piece(type, Colour::WHITE))->second;
        std::reverse_copy(std::begin(previousValue), std::end(previousValue), std::begin(previousValue));
        tempMap.emplace(std::make_pair(Piece(type, Colour::BLACK), previousValue));
    }
    return tempMap;
}
