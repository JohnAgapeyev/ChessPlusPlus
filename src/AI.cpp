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
#include <omp.h>
#include "headers/board.h"
#include "headers/ai.h"
#include "headers/consts.h"
#include "headers/enums.h"

std::unique_ptr<AI::cache_pointer_type> AI::boardCache = std::make_unique<AI::cache_pointer_type>();
const Move AI::emptyMove{};

const std::unordered_multimap<Piece, std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>> 
    AI::pieceSquareTables = AI::initializeMap();

AI::AI(Board *b) : gameBoard(b) {
    timeLimitThread = std::thread{[&](){
        while (isAIActive.load()) {
            if (!isTimeUp.load()) {
                std::this_thread::sleep_for(moveTimeLimit);
                isTimeUp.store(true);
            }
            std::unique_lock<std::mutex> lock{mut};
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
int AI::evaluate(Board& board) {
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
    return currScore;
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
    prev = std::get<0>(result);
    previousToSquareIndex = gameBoard->convertOuterBoardIndex(gameBoard->getSquareIndex(std::get<0>(result).toSq), gameBoard->findCorner_1D());
    gameBoard->makeMove(std::get<0>(result));
    gameBoard->detectGameEnd();
}

std::tuple<Move, int, int, int, int> AI::iterativeDeepening() {
    auto firstGuess = std::make_tuple(emptyMove, 0, -1, -1, -1);
    int evalGuess;
    std::atomic_int maxDepth{0};
    {
        std::lock_guard<std::mutex> lock(mut);
        isTimeUp.store(false);
    }
    cv.notify_all();
#pragma omp parallel default(none) private(evalGuess) firstprivate(gameBoard) shared(firstGuess, maxDepth, isTimeUp)
    {
        Board b{*gameBoard};
#pragma omp for schedule(guided)
        for (int i = 1; i <= DEPTH + (99 * usingTimeLimit); ++i) {
#pragma omp atomic read
            evalGuess = std::get<1>(firstGuess);
            auto searchResult = MTD(evalGuess, i, b);
            if (usingTimeLimit && isTimeUp.load()) {
#pragma omp cancel for
            }
            if (i > maxDepth.load()) {
                maxDepth.store(i);
#pragma omp critical
                firstGuess = searchResult;
            }
#pragma omp cancellation point for
        }
    }
    translateMovePointers(*gameBoard, std::get<0>(firstGuess), std::forward_as_tuple(std::get<2>(firstGuess), std::get<3>(firstGuess), std::get<4>(firstGuess)));
    return firstGuess;
}

std::tuple<Move, int, int, int, int> AI::MTD(const int firstGuess, const int depth, Board& board) {
    auto currGuess = std::make_tuple(emptyMove, firstGuess, -1, -1, -1);
    int upper = INT_MAX;
    int lower = INT_MIN;
    int beta = 0;
    
    while (upper > lower) {
        beta = std::max(std::get<1>(currGuess), lower + 1);
        currGuess = AlphaBeta(beta - 1, beta, depth, board);
        if (std::get<1>(currGuess) < beta) {
            upper = std::get<1>(currGuess);
        } else {
            lower = std::get<1>(currGuess);
        }
    }
    return currGuess;
}

std::tuple<Move, int, int, int, int> AI::AlphaBeta(int alpha, int beta, const int depth, Board& board) {
    assert(depth >= 0);
    auto rtn = std::make_tuple(emptyMove, INT_MIN, -1, -1, -1);

    if (boardCache->retrieve(board)) {
        int entryDepth;
        int entryValue;
        SearchBoundary entryType;
        std::tuple<int, int, int> moveConversionOffsets;
        int fromOffset;
        int toOffset;
        int targetOffset;
        std::tie(entryDepth, entryValue, entryType, std::get<0>(rtn), fromOffset, toOffset, targetOffset) = (*boardCache)[board];
        if (entryDepth >= depth) {
            if (entryType == SearchBoundary::EXACT) {
                return std::make_tuple(std::get<0>(rtn), entryValue, fromOffset, toOffset, targetOffset);
            }
            //Update the best move based on the previous value
            if (entryType == SearchBoundary::LOWER && entryValue > alpha) {
                alpha = entryValue;
            } else if (entryType == SearchBoundary::UPPER && entryValue < beta) {
                beta = entryValue;
            }
            if (alpha >= beta) {
                //Return the best move as well
                return std::make_tuple(std::get<0>(rtn), entryValue, fromOffset, toOffset, targetOffset);
            }
        }
        translateMovePointers(board, std::get<0>(rtn), std::forward_as_tuple(fromOffset, toOffset, targetOffset));

        //If cache entry is invalid due to hash collision, ignore it
        if (std::get<0>(rtn) != emptyMove && !board.moveGen.validateMove(std::get<0>(rtn), true)) {
            rtn = std::make_tuple(emptyMove, INT_MIN, -1, -1, -1);
        }
    }
    
    if (depth == 0) {
        rtn = std::make_tuple(emptyMove, evaluate(board), -1, -1, -1);
    } else if (board.isWhiteTurn) {
        int a = alpha;
        std::get<1>(rtn) = INT_MIN;

        if (std::get<0>(rtn) != emptyMove) {
            board.makeMove(std::get<0>(rtn)); //Make the move if it was found in the cache
            const auto abCall = AlphaBeta(a, beta, depth - 1, board);
            
            if (std::get<1>(abCall) > std::get<1>(rtn)) {
                const auto cornerIndex = board.findCorner_1D();

                std::get<1>(rtn) = std::get<1>(abCall);
                std::get<2>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(std::get<0>(rtn).fromSq), cornerIndex);
                std::get<3>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(std::get<0>(rtn).toSq), cornerIndex);
                std::get<4>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(std::get<0>(rtn).enPassantTarget), cornerIndex);
            }
            a = std::max(a, std::get<1>(rtn));
            board.unmakeMove(std::get<0>(rtn));
        }

        auto moveList = orderMoveList(board.moveGen.generateAll(), board);
        const auto moveListSize = moveList.size();

        if (moveListSize == 0) {
            rtn = std::make_tuple(emptyMove, evaluate(board), -1, -1, -1);
            return rtn;
        }

        for (size_t i = 0; std::get<1>(rtn) < beta && i < moveListSize; ++i) {
            board.makeMove(moveList[i]);
            const auto abCall = AlphaBeta(a, beta, depth - 1, board);

            if (std::get<1>(abCall) > std::get<1>(rtn)) {
                const auto cornerIndex = board.findCorner_1D();

                std::get<0>(rtn) = moveList[i];
                std::get<1>(rtn) = std::get<1>(abCall);
                std::get<2>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(moveList[i].fromSq), cornerIndex);
                std::get<3>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(moveList[i].toSq), cornerIndex);
                std::get<4>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(moveList[i].enPassantTarget), cornerIndex);

                if (usingTimeLimit && isTimeUp.load()) {
                    board.unmakeMove(std::get<0>(rtn));
                    break;
                }
            }

            a = std::max(a, std::get<1>(rtn));
            board.unmakeMove(moveList[i]);
        }
        if (moveListSize > 0 && std::get<0>(rtn) == emptyMove) {
            const auto cornerIndex = board.findCorner_1D();
            std::get<0>(rtn) = moveList[0];
            std::get<1>(rtn) = a;
            std::get<2>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(moveList[0].fromSq), cornerIndex);
            std::get<3>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(moveList[0].toSq), cornerIndex);
            std::get<4>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(moveList[0].enPassantTarget), cornerIndex);
        }
    } else {
        int b = beta;
        std::get<1>(rtn) = INT_MAX;

        if (std::get<0>(rtn) != emptyMove) {
            board.makeMove(std::get<0>(rtn)); //Make the move if it was found in the cache
            const auto abCall = AlphaBeta(alpha, b, depth - 1, board);
            
            if (std::get<1>(abCall) < std::get<1>(rtn)) {
                const auto cornerIndex = board.findCorner_1D();

                std::get<1>(rtn) = std::get<1>(abCall);
                std::get<2>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(std::get<0>(rtn).fromSq), cornerIndex);
                std::get<3>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(std::get<0>(rtn).toSq), cornerIndex);
                std::get<4>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(std::get<0>(rtn).enPassantTarget), cornerIndex);
            }
            b = std::min(b, std::get<1>(rtn));
            board.unmakeMove(std::get<0>(rtn));
        }

        auto moveList = orderMoveList(board.moveGen.generateAll(), board);
        const auto moveListSize = moveList.size();

        if (moveListSize == 0) {
            rtn = std::make_tuple(emptyMove, evaluate(board), -1, -1, -1);
            return rtn;
        }

        for (size_t i = 0; std::get<1>(rtn) > alpha && i < moveListSize; ++i) {
            board.makeMove(moveList[i]);
            const auto abCall = AlphaBeta(alpha, b, depth - 1, board);

            if (std::get<1>(abCall) < std::get<1>(rtn)) {
                const auto cornerIndex = board.findCorner_1D();

                std::get<0>(rtn) = moveList[i];
                std::get<1>(rtn) = std::get<1>(abCall);
                std::get<2>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(moveList[i].fromSq), cornerIndex);
                std::get<3>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(moveList[i].toSq), cornerIndex);
                std::get<4>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(moveList[i].enPassantTarget), cornerIndex);

                if (usingTimeLimit && isTimeUp.load()) {
                    board.unmakeMove(std::get<0>(rtn));
                    break;
                }
            }
            b = std::min(b, std::get<1>(rtn));
            board.unmakeMove(moveList[i]);
        }
        if (moveListSize > 0 && std::get<0>(rtn) == emptyMove) {
            const auto cornerIndex = board.findCorner_1D();
            std::get<0>(rtn) = moveList[0];
            std::get<1>(rtn) = b;
            std::get<2>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(moveList[0].fromSq), cornerIndex);
            std::get<3>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(moveList[0].toSq), cornerIndex);
            std::get<4>(rtn) = board.convertOuterBoardIndex(board.getSquareIndex(moveList[0].enPassantTarget), cornerIndex);
        }
    }

    const auto cornerIndex = board.findCorner_1D();
    
    if (std::get<1>(rtn) <= alpha) {
        //Store rtn as upper bound
        boardCache->add(board, std::make_tuple(depth, std::get<1>(rtn), SearchBoundary::UPPER, 
                    std::get<0>(rtn), 
                        board.convertOuterBoardIndex(board.getSquareIndex(std::get<0>(rtn).fromSq), cornerIndex), 
                        board.convertOuterBoardIndex(board.getSquareIndex(std::get<0>(rtn).toSq), cornerIndex), 
                        board.convertOuterBoardIndex(board.getSquareIndex(std::get<0>(rtn).enPassantTarget), cornerIndex)
                    ));
    } else if (std::get<1>(rtn) > alpha && std::get<1>(rtn) < beta) {
        //Should not happen if using null window, but if it does, store rtn as both upper and lower
        boardCache->add(board, std::make_tuple(depth, std::get<1>(rtn), SearchBoundary::EXACT, 
                    std::get<0>(rtn), 
                        board.convertOuterBoardIndex(board.getSquareIndex(std::get<0>(rtn).fromSq), cornerIndex), 
                        board.convertOuterBoardIndex(board.getSquareIndex(std::get<0>(rtn).toSq), cornerIndex), 
                        board.convertOuterBoardIndex(board.getSquareIndex(std::get<0>(rtn).enPassantTarget), cornerIndex)
                    ));
    } else if (std::get<1>(rtn) >= beta ) {
        //Store rtn as lower bound
        boardCache->add(board, std::make_tuple(depth, std::get<1>(rtn), SearchBoundary::LOWER, 
                    std::get<0>(rtn), 
                        board.convertOuterBoardIndex(board.getSquareIndex(std::get<0>(rtn).fromSq), cornerIndex), 
                        board.convertOuterBoardIndex(board.getSquareIndex(std::get<0>(rtn).toSq), cornerIndex), 
                        board.convertOuterBoardIndex(board.getSquareIndex(std::get<0>(rtn).enPassantTarget), cornerIndex)
                    ));
        if (prev != emptyMove) {
            //If no piece is being captured
            if (std::get<0>(rtn).toSq && !std::get<0>(rtn).toSq->getPiece() && prev != emptyMove) {
                counterMove[
                    (pieceLookupTable.find(prev.fromPieceType)->second * INNER_BOARD_SIZE * INNER_BOARD_SIZE) 
                    + previousToSquareIndex
                ] = std::get<0>(rtn);
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
                    + previousToSquareIndex]) {
                list.insert(captureIt + 1, mv);
            }
        }
    }
    
    return list;
}

void AI::benchmarkPerft() {
    gameBoard->setPositionByFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");

    std::cout << "Position 1\n";
    std::cout << "Depth 1: " << ((perft(1, *gameBoard) == 48) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 2: " << ((perft(2, *gameBoard) == 2039) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 3: " << ((perft(3, *gameBoard) == 97862) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 4: " << ((perft(4, *gameBoard) == 4085603) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 5: " << ((perft(5, *gameBoard) == 193690690) ? "Passed" : "Failed") << "\n";
    gameBoard->setPositionByFEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
    std::cout << "Position 2\n";
    std::cout << "Depth 1: " << ((perft(1, *gameBoard) == 14) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 2: " << ((perft(2, *gameBoard) == 191) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 3: " << ((perft(3, *gameBoard) == 2812) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 4: " << ((perft(4, *gameBoard) == 43238) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 5: " << ((perft(5, *gameBoard) == 674624) ? "Passed" : "Failed") << "\n";
    gameBoard->setPositionByFEN("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    std::cout << "Position 3\n";
    std::cout << "Depth 1: " << ((perft(1, *gameBoard) == 6) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 2: " << ((perft(2, *gameBoard) == 264) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 3: " << ((perft(3, *gameBoard) == 9467) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 4: " << ((perft(4, *gameBoard) == 422333) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 5: " << ((perft(5, *gameBoard) == 15833292) ? "Passed" : "Failed") << "\n";
    gameBoard->setPositionByFEN("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    std::cout << "Position 4\n";
    std::cout << "Depth 1: " << ((perft(1, *gameBoard) == 44) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 2: " << ((perft(2, *gameBoard) == 1486) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 3: " << ((perft(3, *gameBoard) == 62379) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 4: " << ((perft(4, *gameBoard) == 2103487) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 5: " << ((perft(5, *gameBoard) == 89941194) ? "Passed" : "Failed") << "\n";
    gameBoard->setPositionByFEN("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    std::cout << "Position 5\n";
    std::cout << "Depth 1: " << ((perft(1, *gameBoard) == 46) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 2: " << ((perft(2, *gameBoard) == 2079) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 3: " << ((perft(3, *gameBoard) == 89890) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 4: " << ((perft(4, *gameBoard) == 3894594) ? "Passed" : "Failed") << "\n";
    std::cout << "Depth 5: " << ((perft(5, *gameBoard) == 164075551) ? "Passed" : "Failed") << "\n";
}

void AI::translateMovePointers(Board& b, Move& mv, std::tuple<int, int, int> conversionOffsets) {
    const auto cornerIndex = b.findCorner_1D();

    mv.fromSq = (mv.fromSq) ? b.vectorTable[cornerIndex + ((std::get<0>(conversionOffsets) / INNER_BOARD_SIZE) 
            * OUTER_BOARD_SIZE) + (std::get<0>(conversionOffsets) % INNER_BOARD_SIZE)].get() : nullptr;
    mv.toSq = (mv.toSq) ? b.vectorTable[cornerIndex + ((std::get<1>(conversionOffsets) / INNER_BOARD_SIZE) 
            * OUTER_BOARD_SIZE) + (std::get<1>(conversionOffsets) % INNER_BOARD_SIZE)].get() : nullptr;
    mv.enPassantTarget = (mv.enPassantTarget) ? b.vectorTable[cornerIndex + ((std::get<2>(conversionOffsets) / INNER_BOARD_SIZE) 
            * OUTER_BOARD_SIZE) + (std::get<2>(conversionOffsets) % INNER_BOARD_SIZE)].get() : nullptr;
}
