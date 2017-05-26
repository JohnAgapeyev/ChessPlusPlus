#include <vector>
#include <array>
#include <iostream>
#include <iomanip>
#include <regex>
#include <cassert>
#include <string>
#include "headers/board.h"
#include "headers/square.h"
#include "headers/consts.h"
#include "headers/enums.h"
#include "headers/move.h"

/*
 * Try to remove index from squares and instead calculate it based off the index
 * This will ensure that the offsets aren't rotated during board movement
 * 
 * Offset = 98 - (15 * i) + (j + 1)
 * IMPORTANT:
 * Positive x: (30 * ((x + 7) / 15)) - x
 * Negative x: -((30 * ((abs(x) + 7) / 15)) - abs(x))
 */
 
constexpr auto genOffset = [](const auto a, const auto b){return 98 - (15 * a) + b;};

Board::Board() {
    for (int i = 0; i < OUTER_BOARD_SIZE; ++i) {
        for (int j = 0; j < OUTER_BOARD_SIZE; ++j) {
            if (i >= 0 && i <= 7) {
                if (j >= 0 && j <= 7) {
                    vectorTable[(i * OUTER_BOARD_SIZE) + j] = INIT_BOARD[i][j];
                    vectorTable[(i * OUTER_BOARD_SIZE) + j]->setOffset(genOffset(i, j));
                } else {
                    vectorTable[(i * OUTER_BOARD_SIZE) + j] = std::make_shared<Square>(genOffset(i, j));
                }
            } else {
                vectorTable[(i * OUTER_BOARD_SIZE) + j] = std::make_shared<Square>(genOffset(i, j));
            }
        }
    }
    
    for (size_t i = 0, len = repititionList.size(); i < len; ++i) {
        repititionList[i] = i;
    }
    currHash = std::hash<Board>()(*this);
}

Board::Board(const Board& b) : moveGen(b.moveGen), vectorTable(b.vectorTable), 
        currentGameState(b.currentGameState), castleRights(b.castleRights), 
        blackInCheck(b.blackInCheck), whiteInCheck(b.whiteInCheck), 
        isWhiteTurn(b.isWhiteTurn), enPassantActive(b.enPassantActive), 
        enPassantTarget(b.enPassantTarget), halfMoveClock(b.halfMoveClock), 
        moveCounter(b.moveCounter), currHash(b.currHash), 
        repititionList(b.repititionList) {}

std::pair<int, int> Board::findCorner() {
    if (cornerCache != -1) {
        return {cornerCache / OUTER_BOARD_SIZE, cornerCache % OUTER_BOARD_SIZE};
    }
    for (size_t i = 0; i < vectorTable.size(); ++i) {
        if (!vectorTable[i]->checkSentinel()) {
            cornerCache = i;
            return {i / OUTER_BOARD_SIZE, i % OUTER_BOARD_SIZE};
        }
    }
    return {-1, -1};
}

int Board::findCorner_1D() {
    if (cornerCache != -1) {
        return cornerCache;
    }
    for (size_t i = 0; i < vectorTable.size(); ++i) {
        if (!vectorTable[i]->checkSentinel()) {
            cornerCache = i;
            return i; 
        }
    }
    return -1;
}

void Board::printBoardState() const {
#ifdef NDEBUG
    auto range = INNER_BOARD_SIZE;
    std::array<std::shared_ptr<Square>, 64> outputTable;
    std::copy_if(vectorTable.begin(), vectorTable.end(), outputTable.begin(), [](const auto& sq) {
        auto pc = sq->getPiece();
        return !(pc && pc->getColour() == Colour::UNKNOWN && pc->getType() == PieceTypes::UNKNOWN);
    });
#else
    auto range = OUTER_BOARD_SIZE;
#endif
    for (auto i = 0; i < range; ++i) {
#ifdef NDEBUG
        std::cout << "  ";
#endif
        for (int j = 0; j < range; ++j) {
#ifndef NDEBUG
            std::cout << "--------";
#else
            std::cout << "---";
#endif
        }

#ifndef NDEBUG
        std::cout << "-\n|";
        std::for_each(vectorTable.cbegin() + (i * range), vectorTable.cbegin() + ((i + 1) * range), 
                [](const auto& sq){std::cout << *sq << '|';});
#else
        std::cout << "-\n" << (INNER_BOARD_SIZE - i) << " |";
        std::for_each(outputTable.cbegin() + (i * range), outputTable.cbegin() + ((i + 1) * range), 
                [](const auto& sq){std::cout << *sq << '|';});
#endif
        std::cout << "\n";
    }
#ifdef NDEBUG
    std::cout << "  ";
#endif

    for (int k = 0; k < range; ++k) {
#ifndef NDEBUG
        std::cout << "--------";
#else
        std::cout << "---";
#endif
    }
    std::cout << "-\n";
    
#ifdef NDEBUG
    std::cout << "    ";
    for (int k = 0; k < range; ++k) {
        std::cout << std::left << std::setw(3) << static_cast<char>('a' + k);
    }
    std::cout << "\n";
#endif
}

void Board::shiftHorizontal(const int count) {
    if (!count) {
        return;
    }
    auto startCoords = findCorner();
    
    assert(!(startCoords.second + count < 0 
        || startCoords.second + count + INNER_BOARD_SIZE - 1 > OUTER_BOARD_SIZE));

    for (int i = 0, col = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0, row = 0; j < INNER_BOARD_SIZE; ++j) {
            col = (count > 0) ? INNER_BOARD_SIZE - 1 - i : i;
            row = (count > 0) ? INNER_BOARD_SIZE - 1 - j : j;
            
            std::swap(
                vectorTable[
                (startCoords.first * OUTER_BOARD_SIZE) 
                    + startCoords.second + (col * OUTER_BOARD_SIZE) + row]
                ,
                vectorTable[
                count + (startCoords.first * OUTER_BOARD_SIZE) 
                    + startCoords.second + (col * OUTER_BOARD_SIZE) + row]
            );
        }
    }
}

void Board::shiftVertical(const int count) {
    if (!count) {
        return;
    }
    auto startCoords = findCorner();
    
    assert(!(startCoords.first + count < 0 
        || startCoords.first + count + INNER_BOARD_SIZE - 1 > OUTER_BOARD_SIZE));
    
    for (int i = 0, col = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0, row = 0; j < INNER_BOARD_SIZE; ++j) {
            col = (count > 0) ? INNER_BOARD_SIZE - 1 - i : i;
            row = (count > 0) ? INNER_BOARD_SIZE - 1 - j : j;
            
            std::swap(
                vectorTable[
                (startCoords.first * OUTER_BOARD_SIZE) 
                    + startCoords.second + (col * OUTER_BOARD_SIZE) + row]
                ,
                vectorTable[
                (count * OUTER_BOARD_SIZE) 
                    + (startCoords.first * OUTER_BOARD_SIZE) 
                    + startCoords.second + (col * OUTER_BOARD_SIZE) + row]
            );
        }
    }
}

void Board::shiftBoard(const int col, const int row) {
    assert(checkBoardValidity());
    const auto& startCoords = findCorner();
    const auto colDiff = ZERO_LOCATION.first - (startCoords.second + col);
    const auto rowDiff = ZERO_LOCATION.second - (startCoords.first + row);
    shiftHorizontal(colDiff);
    cornerCache = -1; //Invalidate corner cache after shifting the board
    assert(checkBoardValidity());
    shiftVertical(rowDiff);
    cornerCache = -1; //Invalidate corner cache after shifting the board
    assert(checkBoardValidity());
    for (int i = 0; i < OUTER_BOARD_SIZE; ++i) {
        for (int j = 0; j < OUTER_BOARD_SIZE; ++j) {
            vectorTable[(i* OUTER_BOARD_SIZE) + j]->setOffset(genOffset(i, j));
        }
    }
    assert(checkBoardValidity());
}

bool Board::makeMove(std::string& input) {
    assert(checkBoardValidity());
    auto mv = moveGen.createMove(input);
    
    shiftBoard(input[0], INNER_BOARD_SIZE - 1 - input[1]);
    
    if (!moveGen.validateMove(mv, false)) {
        return false;
    }
    
    halfMoveClock++;
    
    const auto cornerIndex = findCorner_1D();
    const auto diff = mv.toSq->getOffset() - mv.fromSq->getOffset();
    
    const auto blackFromPieceHashOffset = ((mv.fromPieceColour == Colour::WHITE) ? 0 : 6);
    const auto blackToPieceHashOffset = ((mv.toPieceColour == Colour::WHITE) ? 0 : 6);
    
    if (mv.fromSq->getPiece()) {
        mv.fromPieceType = mv.fromSq->getPiece()->getType();
        mv.fromPieceColour = mv.fromSq->getPiece()->getColour();
    } else {
        mv.fromPieceType = PieceTypes::UNKNOWN;
        mv.fromPieceColour = Colour::UNKNOWN;
    }
    if (mv.toSq->getPiece()) {
        mv.toPieceType = mv.toSq->getPiece()->getType();
        mv.toPieceColour = mv.toSq->getPiece()->getColour();
        mv.captureMade = true;
    } else {
        mv.toPieceType = PieceTypes::UNKNOWN;
        mv.toPieceColour = Colour::UNKNOWN;
        mv.captureMade = false;
    }

    const auto distToFromSquare = getSquareIndex(mv.fromSq);
    const auto distToEndSquare = getSquareIndex(mv.toSq);
    
    // If en passant move is made, capture the appropriate pawn
    if (enPassantActive && mv.fromPieceType == PieceTypes::PAWN 
            && (diff % OUTER_BOARD_SIZE) && *mv.toSq == *enPassantTarget) {
                
        const int captureIndex = distToEndSquare + ((isWhiteTurn) ? OUTER_BOARD_SIZE: -OUTER_BOARD_SIZE);
        vectorTable[captureIndex]->setPiece(nullptr);
        
        //xor out the captured pawn
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(captureIndex, cornerIndex)
            + pieceLookupTable[PieceTypes::PAWN] + blackToPieceHashOffset];
    }
    if (enPassantActive) {
        const auto distToEnPassantTarget = std::distance(vectorTable.cbegin(), 
            std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
                [=](const auto& sq){return (*sq == *enPassantTarget);}));
    
        //xor out en passant file
        currHash ^= HASH_VALUES[static_cast<int>(SquareState::EN_PASSANT_FILE) 
            + (convertOuterBoardIndex(distToEnPassantTarget, cornerIndex) % INNER_BOARD_SIZE)];
    }
    
    mv.enPassantActive = enPassantActive;
    mv.enPassantTarget = enPassantTarget;
    
    enPassantActive = false;
    enPassantTarget = nullptr;
    
    // Add en Passant target if pawn double move was made.
    if (std::abs(diff) == 30 && mv.fromPieceType == PieceTypes::PAWN) {
        enPassantActive = true;
        enPassantTarget = vectorTable[distToEndSquare + (diff >> 1)].get();
        
        //xor in en passant file
        currHash ^= HASH_VALUES[static_cast<int>(SquareState::EN_PASSANT_FILE) 
            + (distToEndSquare % OUTER_BOARD_SIZE) 
            - static_cast<int>(INNER_BOARD_SIZE - 1 - input[0])];
    }
    
    const bool castleDirectionChosen = moveGen.getCastleDirectionBool(
            mv.fromPieceType, mv.fromPieceColour, diff);
    
    //Perform castling
    if (mv.fromPieceType == PieceTypes::KING && std::abs(diff) == 2 && castleDirectionChosen) {
        mv.isCastle = true;
        
        const auto isQueenSide = (diff < 0);
        
        std::swap(vectorTable[distToFromSquare + 3 - (isQueenSide * 7)], 
            vectorTable[distToFromSquare + 1 - (isQueenSide << 1)]);
        const auto temp = vectorTable[distToFromSquare + 3 - (isQueenSide * 7)]->getOffset();
        vectorTable[distToFromSquare + 3 - (isQueenSide * 7)]->setOffset(
            vectorTable[distToFromSquare + 1 - (isQueenSide << 1)]->getOffset());
        vectorTable[distToFromSquare + 1 - (isQueenSide << 1)]->setOffset(temp);
        
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(
                distToFromSquare + 3 - (isQueenSide * 7), cornerIndex)  
            + pieceLookupTable[PieceTypes::ROOK] + blackFromPieceHashOffset];
            
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(
                distToFromSquare + 1 - (isQueenSide << 1), cornerIndex)
            + pieceLookupTable[PieceTypes::ROOK] + blackFromPieceHashOffset];
            
        if (mv.fromPieceColour == Colour::WHITE) {
            removeCastlingRights(WHITE_CASTLE_FLAG);
        } else {
            removeCastlingRights(BLACK_CASTLE_FLAG);
        }
    }
    // Disable castling if the appropriate rook moves
    if (mv.fromPieceType == PieceTypes::ROOK) {
        if (mv.fromPieceColour == Colour::WHITE && (castleRights & WHITE_CASTLE_FLAG)) {
            const int backRankIndex = findCorner_1D() + (7 * OUTER_BOARD_SIZE);
            const auto& fromSquare = *mv.fromSq;
            if (*(vectorTable[backRankIndex].get()) == fromSquare) {
                removeCastlingRights(WHITE_CASTLE_QUEEN_FLAG);
            }
            if (*(vectorTable[backRankIndex + 7].get()) == fromSquare) {
                removeCastlingRights(WHITE_CASTLE_KING_FLAG);
            }
        } else if (mv.fromPieceColour == Colour::BLACK && (castleRights & BLACK_CASTLE_FLAG)) {
            const int backRankIndex = findCorner_1D();
            const auto& fromSquare = *mv.fromSq;
            if (*(vectorTable[backRankIndex].get()) == fromSquare) {
                removeCastlingRights(BLACK_CASTLE_QUEEN_FLAG);
            }
            if (*(vectorTable[backRankIndex + 7].get()) == fromSquare) {
                removeCastlingRights(BLACK_CASTLE_KING_FLAG);
            }
        }
    }
    
    if (mv.fromPieceType == PieceTypes::PAWN) {
        halfMoveClock = 0;
        const auto distFromStartToCorner = distToEndSquare - cornerIndex;
        const auto& rowPairs = (mv.fromPieceColour == Colour::WHITE) ? std::make_pair(0, 14) : std::make_pair(105, 119);
        
        if (distFromStartToCorner >= rowPairs.first && distFromStartToCorner <= rowPairs.second) {
            const auto& input = promptPromotionType();
            mv.fromSq->getPiece()->promote(static_cast<PieceTypes>(input.front()));
            mv.fromPieceType = static_cast<PieceTypes>(input.front());
            mv.promotionMade = true;
        }
    }
    // If moving to an occupied square, capture the piece
    if (mv.toSq->getPiece()) {
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToEndSquare, cornerIndex)
            + pieceLookupTable[mv.toPieceType] + blackToPieceHashOffset];
        mv.toSq->setPiece(nullptr);
        halfMoveClock = 0;
    }
    //xor out from piece at old square
    currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToFromSquare, cornerIndex)
        + pieceLookupTable[mv.fromPieceType] + blackFromPieceHashOffset];
        
    //xor in from piece at new square
    currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToEndSquare, cornerIndex)
        + pieceLookupTable[mv.fromPieceType] + blackFromPieceHashOffset];
        
    std::swap(*mv.fromSq, *mv.toSq);
    swapOffsets(mv);
    isWhiteTurn = !isWhiteTurn;
    
    mv.halfMoveClock = halfMoveClock;
    
    //xor the change in turn
    currHash ^= HASH_VALUES[static_cast<int>(SquareState::WHITE_MOVE)];
    
    if (isWhiteTurn) {
        ++moveCounter;
    }
    
    mv.moveCounter = moveCounter;
    
    updateCheckStatus();
    
    std::rotate(repititionList.begin(), repititionList.begin() + 1, repititionList.end());
    repititionList[repititionList.size() - 1] = currHash;
    
    //Opponent has no legal moves
    if (!moveGen.generateAll().size()) {
        const auto opponentCheck = (!isWhiteTurn) ? whiteInCheck : blackInCheck;
        if (opponentCheck) {
            //Checkmate
            std::cout << "CHECKMATE\n";
            currentGameState = GameState::MATE;
            return true;
        }
        //Stalemate
        std::cout << "STALEMATE\n";
        currentGameState = GameState::DRAWN;
        return true;
    }
    if (halfMoveClock >= 100) {
        //50 move rule
        std::cout << "DRAW\n";
        std::cout << "50 move rule\n";
        currentGameState = GameState::DRAWN;
        return true;
    }
    
    if (repititionList[0] == repititionList[4] && repititionList[4] == repititionList[8]) {
        //Three move Repitition
        std::cout << "DRAW\n";
        std::cout << "Three move repitition\n";
        currentGameState = GameState::DRAWN;
        return true;
    }
    
    if (drawByMaterial()) {
        //Insufficient Material
        std::cout << "DRAW\n";
        std::cout << "Insufficient Material\n";
        currentGameState = GameState::DRAWN;
        return true;
    }
    assert(checkBoardValidity());
    return true;
}

bool Board::makeMove(Move& mv) {
    assert(checkBoardValidity());
    assert(mv.fromSq && mv.toSq);

    const auto blackFromPieceHashOffset = ((mv.fromPieceColour == Colour::WHITE) ? 0 : 6);
    const auto blackToPieceHashOffset = ((mv.toPieceColour == Colour::WHITE) ? 0 : 6);

    const auto oldFromDist = getSquareIndex(mv.fromSq);
    const auto cornerCoords = findCorner_1D();

    const std::pair<int, int> shiftCoords{(oldFromDist - cornerCoords) / OUTER_BOARD_SIZE, 
        (oldFromDist - cornerCoords) % OUTER_BOARD_SIZE};

    assert(shiftCoords.first != -1);
    assert(shiftCoords.second != -1);
    shiftBoard(shiftCoords.second, shiftCoords.first);

    assert(moveGen.validateMove(mv, false));

    mv.halfMoveClock = halfMoveClock;
    mv.moveCounter = moveCounter;

    halfMoveClock++;
    
    const auto cornerIndex = findCorner_1D();
    const auto diff = mv.toSq->getOffset() - mv.fromSq->getOffset();
    
    if (mv.fromSq->getPiece()) {
        mv.fromPieceType = mv.fromSq->getPiece()->getType();
        mv.fromPieceColour = mv.fromSq->getPiece()->getColour();
    } else {
        mv.fromPieceType = PieceTypes::UNKNOWN;
        mv.fromPieceColour = Colour::UNKNOWN;
    }
    
    if (mv.toSq->getPiece()) {
        mv.toPieceType = mv.toSq->getPiece()->getType();
        mv.toPieceColour = mv.toSq->getPiece()->getColour();
        mv.captureMade = true;
    } else {
        mv.toPieceType = PieceTypes::UNKNOWN;
        mv.toPieceColour = Colour::UNKNOWN;
        mv.captureMade = false;
    }
    
    const auto distToFromSquare = getSquareIndex(mv.fromSq);
    const auto distToEndSquare = getSquareIndex(mv.toSq);

    assert(distToFromSquare != -1);
    assert(distToEndSquare != -1);
    
    const int captureIndex = distToEndSquare + ((isWhiteTurn) ? OUTER_BOARD_SIZE: -OUTER_BOARD_SIZE);
    // If en passant move is made, capture the appropriate pawn
    if (enPassantActive && mv.fromPieceType == PieceTypes::PAWN 
            && (diff % OUTER_BOARD_SIZE) && *mv.toSq == *enPassantTarget
            && vectorTable[captureIndex]->getPiece() 
            && vectorTable[captureIndex]->getPiece()->getColour() != mv.fromPieceColour) {
        vectorTable[captureIndex]->setPiece(nullptr);
        
        //xor out the captured pawn
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(captureIndex, cornerIndex)
            + pieceLookupTable[PieceTypes::PAWN] + ((isWhiteTurn) ? 6 : 0)];
    }
    if (enPassantActive) {
        const auto distToEnPassantTarget = getSquareIndex(enPassantTarget);
        assert(distToEnPassantTarget != -1);
                
        //xor out en passant file
        currHash ^= HASH_VALUES[static_cast<int>(SquareState::EN_PASSANT_FILE) 
            + (convertOuterBoardIndex(distToEnPassantTarget, cornerIndex) % INNER_BOARD_SIZE)];
    }
    
    mv.enPassantActive = enPassantActive;
    mv.enPassantTarget = enPassantTarget;
    
    enPassantActive = false;
    enPassantTarget = nullptr;
    
    // Add en Passant target if pawn double move was made.
    if (std::abs(diff) == 30 && mv.fromPieceType == PieceTypes::PAWN) {
        auto left = vectorTable[distToEndSquare - 1].get();
        auto right = vectorTable[distToEndSquare + 1].get();

        if (left->getPiece() || right->getPiece()) {
            if (left->getPiece() && right->getPiece()) {
                const auto isLeftPawn = (left->getPiece()->getType() == PieceTypes::PAWN && left->getPiece()->getColour() != mv.fromPieceColour);
                const auto isRightPawn = (right->getPiece()->getType() == PieceTypes::PAWN && right->getPiece()->getColour() != mv.fromPieceColour);
                if (isLeftPawn ^ isRightPawn) {
                    if (isLeftPawn) {
                        //Check right square for its piece type
                        if (right->getPiece()->getVectorLength() == INNER_BOARD_SIZE 
                                || (right->getPiece()->getType() == PieceTypes::KING && right->getPiece()->getColour() != mv.fromPieceColour)) {
                            //Enemy pawn and enemy king - perform check check on left square
                            if (!checkEnPassantValidity(left, mv)) {
                                goto notarget;
                            }
                        }
                    } else {
                        //Check left square for its piece type
                        if (left->getPiece()->getVectorLength() == INNER_BOARD_SIZE 
                                || (left->getPiece()->getType() == PieceTypes::KING && left->getPiece()->getColour() != mv.fromPieceColour)) {
                            //Enemy pawn and enemy king - perform check check on right square
                            if (!checkEnPassantValidity(right, mv)) {
                                goto notarget;
                            }
                        }
                    }
                }
            } else if (left->getPiece() && left->getPiece()->getType() == PieceTypes::PAWN && left->getPiece()->getColour() != mv.fromPieceColour) {
                //Perform check check on left square
                if (!checkEnPassantValidity(left, mv)) {
                    goto notarget;
                }
            } else if (right->getPiece() && right->getPiece()->getType() == PieceTypes::PAWN && right->getPiece()->getColour() != mv.fromPieceColour) {
                //Perform check check on right square
                if (!checkEnPassantValidity(right, mv)) {
                    goto notarget;
                }
            }
        }

        enPassantActive = true;
        enPassantTarget = vectorTable[distToEndSquare + (diff / 2)].get();
        
        //xor in en passant file
        currHash ^= HASH_VALUES[static_cast<int>(SquareState::EN_PASSANT_FILE) 
            + (distToEndSquare % OUTER_BOARD_SIZE) - (INNER_BOARD_SIZE - 1 - shiftCoords.second)];
    }

notarget:
    
    const bool castleDirectionChosen = moveGen.getCastleDirectionBool(mv.fromPieceType, mv.fromPieceColour, diff);
    //Perform castling
    if (mv.fromPieceType == PieceTypes::KING && std::abs(diff) == 2 && castleDirectionChosen) {
        mv.isCastle = true;
        
        const auto isQueenSide = (diff < 0);
        
        std::swap(vectorTable[distToFromSquare + 3 - (isQueenSide * 7)], 
            vectorTable[distToFromSquare + 1 - (isQueenSide << 1)]);
        const auto temp = vectorTable[distToFromSquare + 3 - (isQueenSide * 7)]->getOffset();
        vectorTable[distToFromSquare + 3 - (isQueenSide * 7)]->setOffset(
            vectorTable[distToFromSquare + 1 - (isQueenSide << 1)]->getOffset());
        vectorTable[distToFromSquare + 1 - (isQueenSide << 1)]->setOffset(temp);
        
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToFromSquare + 3 - (isQueenSide * 7), cornerIndex)
            + pieceLookupTable[PieceTypes::ROOK] + blackFromPieceHashOffset];
            
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToFromSquare + 1 - (isQueenSide << 1), cornerIndex)
            + pieceLookupTable[PieceTypes::ROOK] + blackFromPieceHashOffset];
            
        if (mv.fromPieceColour == Colour::WHITE) {
            removeCastlingRights(WHITE_CASTLE_FLAG);
        } else {
            removeCastlingRights(BLACK_CASTLE_FLAG);
        }
    }
    // Disable castling if the appropriate rook moves
    if (mv.fromPieceType == PieceTypes::ROOK) {
        if (mv.fromPieceColour == Colour::WHITE && (castleRights & WHITE_CASTLE_FLAG)) {
            const int backRankIndex = findCorner_1D() + (7 * OUTER_BOARD_SIZE);
            if (*(vectorTable[backRankIndex].get()) == *mv.fromSq) {
                removeCastlingRights(WHITE_CASTLE_QUEEN_FLAG);
            }
            if (*(vectorTable[backRankIndex + 7].get()) == *mv.fromSq) {
                removeCastlingRights(WHITE_CASTLE_KING_FLAG);
            }
        } else if (mv.fromPieceColour == Colour::BLACK && (castleRights & BLACK_CASTLE_FLAG)) {
            const int backRankIndex = findCorner_1D();
            if (*(vectorTable[backRankIndex].get()) == *mv.fromSq) {
                removeCastlingRights(BLACK_CASTLE_QUEEN_FLAG);
            }
            if (*(vectorTable[backRankIndex + 7].get()) == *mv.fromSq) {
                removeCastlingRights(BLACK_CASTLE_KING_FLAG);
            }
        }
    } else if (mv.fromPieceType == PieceTypes::KING) {
        if (mv.fromPieceColour == Colour::WHITE && (castleRights & WHITE_CASTLE_FLAG)) {
            removeCastlingRights(WHITE_CASTLE_FLAG);
        } else if (mv.fromPieceColour == Colour::BLACK && (castleRights & BLACK_CASTLE_FLAG)) {
            removeCastlingRights(BLACK_CASTLE_FLAG);
        }
    }
    //Promote pawns on the end ranks
    if (mv.fromPieceType == PieceTypes::PAWN) {
        halfMoveClock = 0;
        const auto distFromStartToCorner = distToEndSquare - cornerIndex;
        const auto& rowPairs = (mv.fromPieceColour == Colour::WHITE) ? std::make_pair(0, 14) : std::make_pair(105, 119);
        
        if (distFromStartToCorner >= rowPairs.first && distFromStartToCorner <= rowPairs.second) {
            mv.fromSq->getPiece()->promote(mv.promotionType);
            mv.fromPieceType = static_cast<PieceTypes>(mv.promotionType);
            mv.promotionMade = true;
        }
    }
    
    // If moving to an occupied square, capture the piece
    if (mv.toSq->getPiece()) {
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToEndSquare, cornerIndex)
            + pieceLookupTable[mv.toPieceType] + blackToPieceHashOffset];
            
        mv.toSq->setPiece(nullptr);
        halfMoveClock = 0;
    }
    //xor out from piece at old square
    currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToFromSquare, cornerIndex)
        + pieceLookupTable[mv.fromPieceType] + blackFromPieceHashOffset];
        
    //xor in from piece at new square
    currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToEndSquare, cornerIndex)
        + pieceLookupTable[mv.fromPieceType] + blackFromPieceHashOffset];
        
    std::swap(*mv.fromSq, *mv.toSq);
    swapOffsets(mv);
    isWhiteTurn = !isWhiteTurn;
    
    
    //xor the change in turn
    currHash ^= HASH_VALUES[static_cast<int>(SquareState::WHITE_MOVE)];
    
    if (isWhiteTurn) {
        moveCounter++;
    }
    
    
    updateCheckStatus();
    
    std::rotate(repititionList.begin(), repititionList.begin() + 1, repititionList.end());
    repititionList[repititionList.size() - 1] = currHash;
    
    assert(checkBoardValidity());
    return true;
}

void Board::unmakeMove(const Move& mv) {
    assert(checkBoardValidity());
    
    const auto distToFromSquare = getSquareIndex(mv.fromSq);
    const auto distToEndSquare = getSquareIndex(mv.toSq);

    assert(distToFromSquare != -1 && distToEndSquare != -1);
                
    const auto blackFromPieceHashOffset = ((mv.fromPieceColour == Colour::WHITE) ? 0 : 6);
    const auto blackToPieceHashOffset = ((mv.toPieceColour == Colour::WHITE) ? 0 : 6);
            
    auto distToOldTarget = 0;
    
    if (enPassantTarget) {
        distToOldTarget = getSquareIndex(enPassantTarget);
        assert(distToOldTarget != -1);
    }
    
    const auto cornerIndex = findCorner_1D();
    const auto diff = mv.toSq->getOffset() - mv.fromSq->getOffset();
    
    //xor out from piece at old square
    currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToFromSquare, cornerIndex)
        + pieceLookupTable[mv.fromPieceType] + blackFromPieceHashOffset];

    if (mv.promotionMade) {
        //xor in from piece at new square
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToEndSquare, cornerIndex)
            + pieceLookupTable[PieceTypes::PAWN] + blackFromPieceHashOffset];

        mv.toSq->setPiece({PieceTypes::PAWN, mv.fromPieceColour});
    } else {
        //xor in from piece at new square
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToEndSquare, cornerIndex)
            + pieceLookupTable[mv.fromPieceType] + blackFromPieceHashOffset];
    }
        
    std::swap(*mv.fromSq, *mv.toSq);
    swapOffsets(mv);
    isWhiteTurn = !isWhiteTurn;
    
    //xor the change in turn
    currHash ^= HASH_VALUES[static_cast<int>(SquareState::WHITE_MOVE)];
    
    if (mv.captureMade) {
        mv.toSq->setPiece({mv.toPieceType, mv.toPieceColour});
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToEndSquare, cornerIndex)
            + pieceLookupTable[mv.toPieceType] + blackToPieceHashOffset];
    }
    
    if (enPassantActive) {
        //Hash out the current file
        currHash ^= HASH_VALUES[static_cast<int>(SquareState::EN_PASSANT_FILE) 
            + (convertOuterBoardIndex(distToOldTarget, cornerIndex) % INNER_BOARD_SIZE)];
    }
    
    if (mv.enPassantTarget) {
        //Hash in the previous file num
        const auto distToCurrTarget = getSquareIndex(mv.enPassantTarget);
        assert(distToCurrTarget != -1);
        const auto fileNum = convertOuterBoardIndex(distToCurrTarget, cornerIndex) % INNER_BOARD_SIZE;
        
        currHash ^= HASH_VALUES[static_cast<int>(SquareState::EN_PASSANT_FILE) + fileNum];
    }
    
    //En passant Capture was made.
    if (mv.enPassantActive 
            && mv.enPassantTarget 
            && mv.fromPieceType == PieceTypes::PAWN 
            && (diff % OUTER_BOARD_SIZE) 
            && !mv.toSq->getPiece()
            && mv.toSq->getOffset() == mv.enPassantTarget->getOffset()) {
                
        //Inner board index to en passant capture square
        const auto enPassantCaptureIndex = distToFromSquare + diff 
            - OUTER_BOARD_SIZE + (30 * (diff < 0));
        
        //Calculate the square containing the captured pawn and put it back
        const auto capturedColour = (mv.fromPieceColour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
            vectorTable[enPassantCaptureIndex]->setPiece({PieceTypes::PAWN, capturedColour});
        
        //Hashing in the captured pawn
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES 
            * convertOuterBoardIndex(enPassantCaptureIndex, cornerIndex) + pieceLookupTable[PieceTypes::PAWN] 
            + ((capturedColour == Colour::WHITE) ? 0 : 6)];
    }
    
    enPassantActive = mv.enPassantActive;
    enPassantTarget = mv.enPassantTarget;
    
    if (mv.isCastle || mv.fromPieceType == PieceTypes::ROOK || mv.fromPieceType == PieceTypes::KING) {
        currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
        castleRights = mv.castleRights;
        currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
    }
    
    /*
     * This section originally was an if else depending on whether the castling
     * was done long or short. Since the only difference between the two was the 
     * values used to offset from distToEndSquare, I decided to remove the branch
     * and redudant code and use the boolean value to change the offset when necessary.
     */
    if (mv.isCastle) {
        const auto isQueenSide = (mv.toSq->getOffset() - mv.fromSq->getOffset() < 0);
        
        std::swap(vectorTable[distToEndSquare - 1 + (isQueenSide << 1)], 
            vectorTable[distToEndSquare + 1 - (isQueenSide * 3)]);
            
        const auto temp = vectorTable[distToEndSquare - 1 + (isQueenSide << 1)]->getOffset();
        
        vectorTable[distToEndSquare - 1 + (isQueenSide << 1)]->setOffset(
            vectorTable[distToEndSquare + 1 - (isQueenSide * 3)]->getOffset());
            
        vectorTable[distToEndSquare + 1 - (isQueenSide * 3)]->setOffset(temp);

        currHash ^= HASH_VALUES[NUM_SQUARE_STATES 
            * convertOuterBoardIndex(distToFromSquare + 3 - (isQueenSide * 7), cornerIndex)
            + pieceLookupTable[PieceTypes::ROOK] + blackFromPieceHashOffset];
            
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES 
            * convertOuterBoardIndex(distToFromSquare + 1 - (isQueenSide << 1), cornerIndex)
            + pieceLookupTable[PieceTypes::ROOK] + blackFromPieceHashOffset];
    }

    std::rotate(repititionList.rbegin(), repititionList.rbegin() + 1, repititionList.rend());
    repititionList[0] = currHash;
    
    updateCheckStatus();

    halfMoveClock = mv.halfMoveClock;
    moveCounter = mv.moveCounter;
    
    assert(checkBoardValidity());
}

std::string Board::generateFEN() {
    std::string output;
    int emptySquareCounter = 0;
    int rowCount = 1;
    bool rowContainsGameBoard = false;
    for (size_t i = 0; i < vectorTable.size(); ++i) {
        const auto& sq = vectorTable[i].get();
        if (!sq->getPiece()) {
            emptySquareCounter++;
            rowContainsGameBoard = true;
        } else if (!sq->checkSentinel()) {
            rowContainsGameBoard = true;
            const auto& squarePiece = sq->getPiece();
            const auto& charCaseFunction = (squarePiece->getColour() == Colour::WHITE) ? toupper : tolower;
            if (emptySquareCounter) {
                output += std::to_string(emptySquareCounter);
            }
            output += charCaseFunction(static_cast<char>(squarePiece->getType()));
            emptySquareCounter = 0;
        }
        if (i % OUTER_BOARD_SIZE == 14) {
            if (rowContainsGameBoard) {
                if (emptySquareCounter) {
                    output += std::to_string(emptySquareCounter);
                }
                if (rowCount < INNER_BOARD_SIZE) {
                    output += '/';
                }
                emptySquareCounter = 0;
                rowCount++;
            }
            rowContainsGameBoard = false;
        }
    }
    output += ' ';
    output += (isWhiteTurn) ? 'w' : 'b';
    output += ' ';
    if (castleRights & WHITE_CASTLE_KING_FLAG) {
        output += 'K';
    }
    if (castleRights & WHITE_CASTLE_QUEEN_FLAG) {
        output += 'Q';
    }
    if (castleRights & BLACK_CASTLE_KING_FLAG) {
        output += 'k';
    }
    if (castleRights & BLACK_CASTLE_QUEEN_FLAG) {
        output += 'q';
    } 
    if (!castleRights) {
        output += '-';
    }
    output += ' ';
    if (enPassantActive) {
        const auto distToCurrSquare = getSquareIndex(enPassantTarget);
        assert(distToCurrSquare != -1);
        const auto cornerDist = findCorner_1D();
        const auto distToTarget = distToCurrSquare - cornerDist;
        output += static_cast<char>(97 + (distToTarget % OUTER_BOARD_SIZE));
        output += std::to_string(INNER_BOARD_SIZE - (distToTarget / OUTER_BOARD_SIZE));
    } else {
        output += '-';
    }
    output += ' ';
    output += std::to_string(halfMoveClock);
    output += ' ';
    output += std::to_string(moveCounter);
    return output;
}

bool Board::drawByMaterial() {
    const int cornerIndex = findCorner_1D();
    int minorCount = 0;
    bool whiteBishopFound = false;
    bool blackBishopFound = false;
    std::pair<int, int> whiteBishopCoords = {0,0};
    std::pair<int, int> blackBishopCoords = {0,0};
    for (int i = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0; j < INNER_BOARD_SIZE; ++j) {
            const auto& currPiece = vectorTable[cornerIndex + (i * OUTER_BOARD_SIZE) + j]->getPiece();
            if (currPiece) {
                const PieceTypes type = currPiece->getType();
                if (type == PieceTypes::PAWN || type == PieceTypes::ROOK || type == PieceTypes::QUEEN) {
                    return false;
                }
                if (type == PieceTypes::KNIGHT || type == PieceTypes::BISHOP) {
                    minorCount++;
                    if (type == PieceTypes::BISHOP) {
                        if (currPiece->getColour() == Colour::WHITE) {
                            whiteBishopFound = true;
                            whiteBishopCoords = {i, j};
                        } else {
                            blackBishopFound = true;
                            blackBishopCoords = {i, j};
                        }
                    }
                }
            }
        }
    }
    if (minorCount < 2) {
        return true;
    }
    if (minorCount > 2) {
        return false;
    }
    if (whiteBishopFound && blackBishopFound) {
        //perform bishop colour check, returns 0 if white, 1 if black and checks if they are equal
        if (((whiteBishopCoords.first & 1) ^ (whiteBishopCoords.second & 1)) 
                == ((blackBishopCoords.first & 1) ^ (blackBishopCoords.second & 1))) {
            return true;
        }
    }
    return false;
}

std::string Board::promptPromotionType() const {
    std::string input;
    std::regex reg("[NBRQ]");
    
    for (;;) {
        std::cout << "Pawn promotion detected\n";
        std::cout << "Knight = [n/N], Bishop = [b/B], Rook = [r/R], Queen = [q/Q]\n";
        std::cout << "Enter choice for promotion:\n";
        std::getline(std::cin, input);
        std::transform(input.begin(), input.end(), input.begin(), ::toupper);
        if (input.length() == 1 && std::regex_match(input, reg)) {
            break;
        }
        std::cout << "Invalid input\n";
    }
    return input;
}

void Board::updateCheckStatus() {
    const auto corner = findCorner_1D();
    int idx = -1;
    for (int i = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0; j < INNER_BOARD_SIZE; ++j) {
            const auto& piece = vectorTable[corner + (i * OUTER_BOARD_SIZE) + j]->getPiece();
            if (piece && piece->getType() == PieceTypes::KING && piece->getColour() == Colour::BLACK) {
                idx = (i * OUTER_BOARD_SIZE) + j;
                break;
            }
        }
    }
    assert(idx != -1);
    blackInCheck = moveGen.inCheck(corner + idx);

    idx = -1;
    for (int i = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0; j < INNER_BOARD_SIZE; ++j) {
            const auto& piece = vectorTable[corner + (i * OUTER_BOARD_SIZE) + j]->getPiece();
            if (piece && piece->getType() == PieceTypes::KING && piece->getColour() == Colour::WHITE) {
                idx = (i * OUTER_BOARD_SIZE) + j;
                break;
            }
        }
    }
    assert(idx != -1);
    whiteInCheck = moveGen.inCheck(corner + idx);
}

//Currently no validation for this method as it is being built primarily for perft position testing
void Board::setPositionByFEN(const std::string& fen) {
    const auto cornerDist = findCorner_1D();
    std::stringstream fenStream(fen);
    std::array<std::string, 6> fenSections;
    for (int i = 0; fenStream.good() && i < 6; ++i) {
        fenStream >> fenSections[i];
    }
    
    //Append a special character to detect the end of the string
    fenSections[0].push_back('#');
    
    for (int i = 0, currStrPos = 0, currSquareIdx = 0; i < INNER_BOARD_SIZE; ++i) {
        while (fenSections[0][currStrPos] != '/' && fenSections[0][currStrPos] != '#') {
            if (std::isdigit(fenSections[0][currStrPos])) {
                const auto jumpLength = fenSections[0][currStrPos] - '0';
                for (int j = 0; j < jumpLength; ++j) {
                    vectorTable[cornerDist + (OUTER_BOARD_SIZE * i) + currSquareIdx + j]->setPiece(nullptr);
                }
                currSquareIdx += jumpLength;
                ++currStrPos;
                continue;
            }
            
            vectorTable[cornerDist + (OUTER_BOARD_SIZE * i) + currSquareIdx]->setPiece(
                {static_cast<PieceTypes>(std::toupper(fenSections[0][currStrPos])), 
                    (std::isupper(fenSections[0][currStrPos])) ? Colour::WHITE : Colour::BLACK});
                    
            ++currStrPos;
            ++currSquareIdx;
        }
        ++currStrPos;
        currSquareIdx = 0;
    }
    
    isWhiteTurn = fenSections[1] == "w";
    
    castleRights = 0x00;
    
    //Castling rights are not empty
    if (fenSections[2].find_first_of('-') == std::string::npos) {
        if (fenSections[2].find_first_of('K') != std::string::npos) {
            castleRights += WHITE_CASTLE_KING_FLAG;
        }
        if (fenSections[2].find_first_of('Q') != std::string::npos) {
            castleRights += WHITE_CASTLE_QUEEN_FLAG;
        }
        if (fenSections[2].find_first_of('k') != std::string::npos) {
            castleRights += BLACK_CASTLE_KING_FLAG;
        }
        if (fenSections[2].find_first_of('q') != std::string::npos) {
            castleRights += BLACK_CASTLE_QUEEN_FLAG;
        }
    }
    
    if (fenSections[3].find_first_of('-') != std::string::npos) {
        enPassantActive = false;
        enPassantTarget = nullptr;
    } else {
        // Convert the letter to a digit
        fenSections[3][0] -= 49;
        // Change input to zero-indexed value
        fenSections[3][1] -= 1;
        // Change chars to digits
        fenSections[3][0] -= '0';
        fenSections[3][1] -= '0';
        
        enPassantActive = true;
        enPassantTarget = vectorTable[((INNER_BOARD_SIZE - 1 
            - fenSections[3][1]) * OUTER_BOARD_SIZE) + cornerDist 
            + fenSections[3][0]].get();
    }
    
    if (!fenSections[4].empty() && std::all_of(fenSections[4].begin(), fenSections[4].end(), ::isdigit)) {
        halfMoveClock = std::atoi(fenSections[4].c_str());
    }
    if (!fenSections[5].empty()&& std::all_of(fenSections[5].begin(), fenSections[5].end(), ::isdigit)) {
        moveCounter = std::atoi(fenSections[5].c_str());
    }
    
    //Resetting the board hash based on the new position
    currHash = 0;
    currHash = std::hash<Board>()(*this);
}

//Testing method used to assert board state
bool Board::checkBoardValidity() {
    int cornerCoords;
    if ((cornerCoords = findCorner_1D()) == -1) {
        std::cerr << "Could not find corner\n";
        return false;
    }
    for (int i = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0; j < INNER_BOARD_SIZE; ++j) {
            if (!vectorTable[cornerCoords + (i * OUTER_BOARD_SIZE) + j]) {
                std::cerr << "Board square is null\n";
                printBoardState();
                return false;
            }
            if (vectorTable[cornerCoords + (i * OUTER_BOARD_SIZE) + j]->checkSentinel()) {
                std::cerr << "Square is sentinel when it shouldn't be\n";
                printBoardState();
                return false;
            }
        }
    }
    if (enPassantActive && !enPassantTarget) {
        std::cerr << "En passant target should never be null when en passant is active\n";
        return false;
    }
    
    if (std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
            [](const auto& sq){
                const auto& piece = sq->getPiece();
                return piece && piece->getType() == PieceTypes::KING 
                    && piece->getColour() == Colour::WHITE;
            }) == vectorTable.cend()) {
        std::cerr << "Could not find white king\n";
        return false;
    }
    
    if (std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
            [](const auto& sq){
                const auto& piece = sq->getPiece();
                return piece && piece->getType() == PieceTypes::KING 
                    && piece->getColour() == Colour::BLACK;
            }) == vectorTable.cend()) {
        std::cerr << "Could not find black king\n";
        return false;
    }
    
    Board temp(*this);
    temp.currHash = 0;
    temp.currHash = std::hash<Board>()(temp);
    
    if (currHash != temp.currHash) {
        std::cerr << "Calculated hash does not match current board hash\n";
        return false;
    }
    return true;
}

std::string Board::convertSquareToCoordText(const Square *sq) {
    const auto cornerCoords = findCorner_1D();
    const auto squareDist = getSquareIndex(sq);
    assert(squareDist != -1);
    const auto innerDist = convertOuterBoardIndex(squareDist, cornerCoords);
    
    return static_cast<char>('a' + (innerDist % INNER_BOARD_SIZE)) 
        + std::to_string(INNER_BOARD_SIZE - (innerDist / INNER_BOARD_SIZE));
}

std::string Board::convertMoveToCoordText(const Move& mv) {
    return convertSquareToCoordText(mv.fromSq) + convertSquareToCoordText(mv.toSq);
}

int Board::getSquareIndex(const Square *sq) {
    if (!sq) {
        return -1;
    }

    const auto cornerIndex = findCorner_1D();

    for (int i = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0; j < INNER_BOARD_SIZE; ++j) {
            if (*vectorTable[cornerIndex + (i * OUTER_BOARD_SIZE) + j] == *sq) {
                return cornerIndex + (i * OUTER_BOARD_SIZE) + j;
            }
        }
    }

    return -1;
}

bool Board::checkEnPassantValidity(Square *sq, const Move& mv) {
    //Enemy pawn and enemy king - perform check check on left square
    Piece temp = *sq->getPiece();
    sq->setPiece(nullptr);

    const auto cornerIndex = findCorner_1D();

    //Get opposite colour king position
    int idx = -1;
    for (int i = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0; j < INNER_BOARD_SIZE; ++j) {
            const auto piece = vectorTable[cornerIndex + (i * OUTER_BOARD_SIZE) + j]->getPiece();
            if (piece && piece->getType() == PieceTypes::KING && piece->getColour() != mv.fromPieceColour) {
                idx = (i * OUTER_BOARD_SIZE) + j;
                break;
            }
        }
    }
    assert(idx != -1);

    const auto result = moveGen.inCheck(cornerIndex + idx);
    sq->setPiece(std::move(temp));

    if (result) {
        return false;
    }
    return true;
}

void Board::removeCastlingRights(const unsigned char flag) {
    currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
    castleRights &= ~flag;
    currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
}
