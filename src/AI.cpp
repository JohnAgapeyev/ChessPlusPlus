#include <algorithm>
#include <climits>
#include <tuple>
#include <cassert>
#include <set>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <atomic>
#include <cassert>
#include "headers/board.h"
#include "headers/ai.h"
#include "headers/consts.h"
#include "headers/enums.h"

std::unique_ptr<AI::cache_pointer_type> AI::boardCache = std::make_unique<AI::cache_pointer_type>();
const Move AI::emptyMove{};

const std::unordered_multimap<Piece, std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>> 
    AI::pieceSquareTables = AI::initializeMap();

AI::AI(Board& b) : gameBoard(b) {
    timeLimitThread = std::thread{[&](){
        while (isAIActive.load()) {
            if (!isTimeUp.load()) {
                std::this_thread::sleep_for(moveTimeLimit);
                isTimeUp.store(true);
            }
            static std::unique_lock<std::mutex> lock{mut};
            cv.wait(lock);
        }
    }};
}
AI::~AI() {
    isAIActive.store(false);
    {
        std::lock_guard<std::mutex> lock(mut);
        isTimeUp.store(true);
    }
    cv.notify_all();
    if (timeLimitThread.joinable()) {
        timeLimitThread.join();
    }
}

/**
 * Evaluates the current board state from the perspective of the current player
 * to move with a positive number favouring white, and a negative one favouring black.
 */
void AI::evaluate(Board& board) {
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

    auto endGamePieceCount = 0;
    auto whiteKingIndex = -1;
    auto blackKingIndex = -1;

    currScore += (whiteTotalMoves - blackTotalMoves) * MOBILITY_VAL;
    
    currScore -= reduceKnightMobilityScore(whiteMoveList, cornerIndex, board);
    currScore += reduceKnightMobilityScore(blackMoveList, cornerIndex, board);
    
    if (hasWhiteCastled) {
        currScore += CASTLE_BONUS;
    }
    if (hasBlackCastled) {
        currScore -= CASTLE_BONUS;
    }
    
    //Counting material values
    for(int i = 0; i < INNER_BOARD_SIZE; ++i) {
        for(int j = 0; j < INNER_BOARD_SIZE; ++j) {
            const auto currPiece = board.vectorTable[cornerIndex + (i * OUTER_BOARD_SIZE) + j]->getPiece();
            
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
                        } else {
                            ++endGamePieceCount;
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
                        } else {
                            ++endGamePieceCount;
                        }
                    }
                } else {
                    if (currPiece->getColour() == Colour::WHITE) {
                        //Material value
                        currScore += getPieceValue(currPiece->getType());

                        whiteKingIndex = Board::convertOuterBoardIndex(cornerIndex + (i * OUTER_BOARD_SIZE) + j, cornerIndex);
                    } else {
                        //Material value
                        currScore -= getPieceValue(currPiece->getType());

                        blackKingIndex = Board::convertOuterBoardIndex(cornerIndex + (i * OUTER_BOARD_SIZE) + j, cornerIndex);
                    }
                }
            }
        }
    }

    assert(whiteKingIndex != -1);
    assert(blackKingIndex != -1);

    if (endGamePieceCount <= 4) {
        //Endgame
        const auto& whiteKingTable = pieceSquareTables.equal_range({PieceTypes::KING, Colour::WHITE});
        const auto& blackKingTable = pieceSquareTables.equal_range({PieceTypes::KING, Colour::BLACK});

        currScore += whiteKingTable.second->second[whiteKingIndex];
        currScore += blackKingTable.second->second[blackKingIndex];
    } else {
        //Not endgame
        const auto& whiteKingTable = pieceSquareTables.equal_range({PieceTypes::KING, Colour::WHITE});
        const auto& blackKingTable = pieceSquareTables.equal_range({PieceTypes::KING, Colour::BLACK});

        currScore += whiteKingTable.first->second[whiteKingIndex];
        currScore += blackKingTable.first->second[blackKingIndex];
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
        currScore = -MATE * board.whiteInCheck;
    } else if (!blackMoveList.size()) {
        currScore = MATE * board.blackInCheck;
    } else if (board.halfMoveClock >= 100) {
        currScore = 0;
    } else if (board.repititionList[0] == board.repititionList[4] 
            && board.repititionList[4] == board.repititionList[8]) {
        currScore = 0;
    } else if (board.drawByMaterial()) {
        currScore = 0;
    }
    eval = currScore;
}

int AI::reduceKnightMobilityScore(const std::vector<Move>& moveList, const int cornerIndex, const Board& board) const {
    static constexpr int pawnThreatOffsets[] = {14, 16, -14, -16};
    auto totalToRemove = 0;
    auto cornerCheckIndex = 0;
    
    for(const auto& mv : moveList) {
        if (mv.fromPieceType == PieceTypes::KNIGHT) {
            
            //Calculate index of destination square without requiring linear search
            const auto cornerToSqDiff = board.moveGen.getOffsetIndex(
                mv.toSq->getOffset() - board.vectorTable[cornerIndex]->getOffset(), cornerIndex);
                
            for (int i = 0; i < 4; ++i) {
                cornerCheckIndex = board.moveGen.getOffsetIndex(pawnThreatOffsets[i], cornerToSqDiff);
                if (cornerCheckIndex < 0 || cornerCheckIndex >= TOTAL_BOARD_SIZE) {
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
        case PieceTypes::KING:
            return KING_VAL;
        default:
            return 0;
    }
}

void AI::search() {
    auto result = iterativeDeepening();
    
    if (result.first.isCastle) {
        if (result.first.fromPieceColour == Colour::WHITE) {
            hasWhiteCastled = true;
        } else {
            hasBlackCastled = true;
        }
    }

    prev = result.first;
    gameBoard.makeMove(result.first);

    gameBoard.detectGameEnd();
}

std::pair<Move, int> AI::iterativeDeepening() {
    auto firstGuess = std::make_pair(emptyMove, 0);
    if (usingTimeLimit) {
        {
            std::lock_guard<std::mutex> lock(mut);
            isTimeUp.store(false);
        }
        cv.notify_all();
        for (int i = 0; i < DEPTH; ++i) {
            firstGuess = MTD(firstGuess.second, i, gameBoard);
            if (usingTimeLimit && isTimeUp.load()) {
                break;
            }
        }
    } else {
        for (int i = 0; i < DEPTH; ++i) {
            firstGuess = MTD(firstGuess.second, i, gameBoard);
        }
    }
    return firstGuess;
}

std::pair<Move, int> AI::MTD(const int firstGuess, const int depth, Board& board) {
    auto currGuess = std::make_pair(emptyMove, firstGuess);
    int upper = INT_MAX;
    int lower = INT_MIN;
    int beta = 0;
    
    if (usingTimeLimit) {
        while (upper > lower) {
            beta = std::max(currGuess.second, lower + 1);
            currGuess = AlphaBeta(beta - 1, beta, depth, board);
            if (currGuess.second < beta) {
                upper = currGuess.second;
            } else {
                lower = currGuess.second;
            }
            if (isTimeUp.load()) {
                break;
            }
        }
    } else {
        while (upper > lower) {
            beta = std::max(currGuess.second, lower + 1);
            currGuess = AlphaBeta(beta - 1, beta, depth, board);
            if (currGuess.second < beta) {
                upper = currGuess.second;
            } else {
                lower = currGuess.second;
            }
        }
    }
    return currGuess;
}

std::pair<Move, int> AI::AlphaBeta(int alpha, int beta, const int depth, Board& board) {
    assert(depth >= 0);
    auto rtn = std::make_pair(emptyMove, INT_MIN);
        
    if (boardCache->retrieve(board)) {
        int entryDepth;
        int entryValue;
        SearchBoundary entryType;
        std::tie(entryDepth, entryValue, entryType, rtn.first) = (*boardCache)[board];
        if (entryDepth >= depth) {
            if (entryType == SearchBoundary::EXACT) {
                return {rtn.first, entryValue};
            }
            //Update the best move based on the previous value
            if (entryType == SearchBoundary::LOWER && entryValue > alpha) {
                alpha = entryValue;
            } else if (entryType == SearchBoundary::UPPER && entryValue < beta) {
                beta = entryValue;
            }
            if (alpha >= beta) {
                //Return the best move as well
                return {rtn.first, entryValue};
            }
        }
    }
    
    if (depth == 0) {
        evaluate(board);
        rtn = std::make_pair(emptyMove, eval);
    } else if (board.isWhiteTurn) {
        int a = alpha;
        rtn.second = INT_MIN;

        if (rtn.first != emptyMove) {
            board.makeMove(rtn.first); //Make the move if it was found in the cache
            const auto& abCall = AlphaBeta(a, beta, depth - 1, board);
            
            if (abCall.second > rtn.second) {
                rtn.second = abCall.second;
            }
            a = std::max(a, rtn.second);
            board.unmakeMove(rtn.first);
        }

        auto moveList = orderMoveList(board.moveGen.generateAll(), board);
        const auto moveListSize = moveList.size();
        
        for (size_t i = 0; rtn.second < beta && i < moveListSize; ++i) {
            assert(moveList[i].fromSq && moveList[i].toSq);
            board.makeMove(moveList[i]);
            const auto& abCall = AlphaBeta(a, beta, depth - 1, board);
            
            if (abCall.second > rtn.second) {
                rtn.first = moveList[i];
                rtn.second = abCall.second;
            }
            a = std::max(a, rtn.second);
            board.unmakeMove(moveList[i]);
        }
    } else {
        int b = beta;
        rtn.second = INT_MAX;

        if (rtn.first != emptyMove) {
            board.makeMove(rtn.first); //Make the move if it was found in the cache
            const auto& abCall = AlphaBeta(alpha, b, depth - 1, board);
            
            if (abCall.second < rtn.second) {
                rtn.second = abCall.second;
            }
            b = std::min(b, rtn.second);
            board.unmakeMove(rtn.first);
        }

        auto moveList = orderMoveList(board.moveGen.generateAll(), board);
        const auto moveListSize = moveList.size();
        
        for (size_t i = 0; rtn.second > alpha && i < moveListSize; ++i) {
            assert(moveList[i].fromSq && moveList[i].toSq);
            board.makeMove(moveList[i]);
            const auto& abCall = AlphaBeta(alpha, b, depth - 1, board);
            
            if (abCall.second < rtn.second) {
                rtn.first = moveList[i];
                rtn.second = abCall.second;
            }
            b = std::min(b, rtn.second);
            board.unmakeMove(moveList[i]);
        }
    }
    
    if (rtn.second <= alpha) {
        //Store rtn as upper bound
        boardCache->add(board, std::make_tuple(depth, rtn.second, SearchBoundary::UPPER, rtn.first));
    } else if (rtn.second > alpha && rtn.second < beta) {
        //Should not happen if using null window, but if it does, store rtn as both upper and lower
        boardCache->add(board, std::make_tuple(depth, rtn.second, SearchBoundary::EXACT, rtn.first));
    } else if (rtn.second >= beta ) {
        //Store rtn as lower bound
        boardCache->add(board, std::make_tuple(depth, rtn.second, SearchBoundary::LOWER, rtn.first));
        
        if (prev != emptyMove) {
            assert(pieceLookupTable.find(prev.fromPieceType) != pieceLookupTable.end());

            //If no piece is being captured
            if (rtn.first.toSq && !rtn.first.toSq->getPiece() && prev != emptyMove) {
                counterMove[
                    (pieceLookupTable.find(prev.fromPieceType)->second * INNER_BOARD_SIZE * INNER_BOARD_SIZE) 
                    + board.convertOuterBoardIndex(board.getSquareIndex(prev.toSq), board.findCorner_1D())
                ] = rtn.first;
            }
        }
    }
    
    return rtn;
}

unsigned long long AI::perft(int depth, Board& board) {
    auto moveList = board.moveGen.generateAll();
    const auto moveListSize = moveList.size();

    assert(depth > 0);

    if (depth == 1) {
        return moveListSize;
    }

    unsigned long long nodeCount = 0;

#pragma omp parallel firstprivate(board, moveList, moveListSize)
    {
        Board b{board};
        moveList = b.moveGen.generateAll();
#pragma omp for reduction(+:nodeCount)
        for (size_t i = 0; i < moveListSize; ++i) {
            b.makeMove(moveList[i]);
            nodeCount += perft(depth - 1, b);
            b.unmakeMove(moveList[i]);
        }
    }
    return nodeCount;
}

unsigned long long AI::perftDivide(int depth, Board& board) {
    auto moveList = board.moveGen.generateAll();
    const auto moveListSize = moveList.size();
    
    if (!depth) {
        return 1;
    }
    
    unsigned long long nodeCount = 0;
    for (size_t i = 0; i < moveListSize; ++i) {
        board.makeMove(moveList[i]);
        const auto perftResult = perft(depth - 1, board);
        std::cout << board.convertMoveToCoordText(moveList[i]) << ": " << perftResult << "\n";
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

std::vector<Move> AI::orderMoveList(std::vector<Move>&& list, Board& board) {
    assert([&]()->bool{
        for (const auto& mv : list) {
            if (!mv.fromSq || !mv.toSq) {
                return false;
            }
        }
        return true;
    }());

    //Partition list with captures coming before quiet moves
    auto captureIt = std::partition(list.begin(), list.end(), 
        [](const auto& mv){return mv.toPieceType != PieceTypes::UNKNOWN;});
        
    //MVV-LVA sorting
    std::sort(list.begin(), captureIt, 
        [this](const auto& first, const auto& second) {
            if (first.toPieceType == second.toPieceType) {
                return this->getPieceValue(first.fromPieceType) < this->getPieceValue(second.fromPieceType);
            }
            return this->getPieceValue(first.toPieceType) > this->getPieceValue(second.toPieceType);
        }
    );

    if (prev != emptyMove) {
        assert(pieceLookupTable.find(prev.fromPieceType) != pieceLookupTable.end());

        for (const auto& mv : list) {
            if (mv == counterMove[(pieceLookupTable.find(prev.fromPieceType)->second * INNER_BOARD_SIZE * INNER_BOARD_SIZE) 
                    + board.convertOuterBoardIndex(board.getSquareIndex(prev.toSq), board.findCorner_1D())]) {
                list.insert(captureIt + 1, mv);
            }
        }
    }
    
    return list;
}

void AI::benchmarkPerft() {
    gameBoard.setPositionByFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");

    std::cout << "Position 1\n";
    std::cout << "Depth 1: " << ((perft(1, gameBoard) == 48) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 2: " << ((perft(2, gameBoard) == 2039) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 3: " << ((perft(3, gameBoard) == 97862) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 4: " << ((perft(4, gameBoard) == 4085603) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 5: " << ((perft(5, gameBoard) == 193690690) ? "Passed" : "Failed") << "\n";
    gameBoard.setPositionByFEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
    std::cout << "Position 2\n";
    std::cout << "Depth 1: " << ((perft(1, gameBoard) == 14) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 2: " << ((perft(2, gameBoard) == 191) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 3: " << ((perft(3, gameBoard) == 2812) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 4: " << ((perft(4, gameBoard) == 43238) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 5: " << ((perft(5, gameBoard) == 674624) ? "Passed" : "Failed") << "\n";
    gameBoard.setPositionByFEN("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    std::cout << "Position 3\n";
    std::cout << "Depth 1: " << ((perft(1, gameBoard) == 6) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 2: " << ((perft(2, gameBoard) == 264) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 3: " << ((perft(3, gameBoard) == 9467) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 4: " << ((perft(4, gameBoard) == 422333) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 5: " << ((perft(5, gameBoard) == 15833292) ? "Passed" : "Failed") << "\n";
    gameBoard.setPositionByFEN("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    std::cout << "Position 4\n";
    std::cout << "Depth 1: " << ((perft(1, gameBoard) == 44) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 2: " << ((perft(2, gameBoard) == 1486) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 3: " << ((perft(3, gameBoard) == 62379) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 4: " << ((perft(4, gameBoard) == 2103487) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 5: " << ((perft(5, gameBoard) == 89941194) ? "Passed" : "Failed") << "\n";
    gameBoard.setPositionByFEN("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    std::cout << "Position 5\n";
    std::cout << "Depth 1: " << ((perft(1, gameBoard) == 46) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 2: " << ((perft(2, gameBoard) == 2079) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 3: " << ((perft(3, gameBoard) == 89890) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 4: " << ((perft(4, gameBoard) == 3894594) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 5: " << ((perft(5, gameBoard) == 164075551) ? "Passed" : "Failed") << "\n";
}
