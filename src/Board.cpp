#include "headers/square.h"
#include "headers/board.h"
#include "headers/consts.h"
#include "headers/enums.h"
#include "headers/move.h"
#include "headers/movegenerator.h"
#include <vector>
#include <array>
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <stdexcept>
#include <regex>
#include <cctype>
#include <cstring>
#include <string>

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
    moveGen = std::make_unique<MoveGenerator>(*this);
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
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
    repititionList.fill(std::hash<Board>()(*this));
    currHash = repititionList[0];
}

Board::Board(const Board& b) : Board() {
    currentGameState = b.currentGameState;
    castleRights = b.castleRights;
    blackInCheck = b.blackInCheck;
    whiteInCheck = b.whiteInCheck;
    isWhiteTurn = b.isWhiteTurn;
    enPassantActive = b.enPassantActive;
    enPassantTarget = b.enPassantTarget;
    halfMoveClock = b.halfMoveClock;
    moveCounter = b.moveCounter;
    currHash = b.currHash;
}

/*
 * Board contains a unique_ptr<MoveGenerator> where MoveGenerator
 * is an inner class defined outside of board's header. This results in
 * MoveGenerator being defined as an incomplete type at compile time.
 * Unique_ptr requires that the type it holds is 100% complete at compile time.
 * Therefore, I need to manually define a destructor since unique_ptr can't create
 * one due to it holding an incomplete type. Since it's a unique ptr, I don't 
 * have to manually delete any memory, so my destructor just calls 
 * the default one expicitly.
 */
Board::~Board() = default;

std::pair<int, int> Board::findCorner() const {
    auto result = std::find_if(
    vectorTable.cbegin(), vectorTable.cend(), 
    [](const auto& sq) {
        auto pc = sq->getPiece();
        return (!pc || (pc->getColour() != Colour::UNKNOWN && pc->getType() != PieceTypes::UNKNOWN));
    });
    
    if (result != vectorTable.cend()) {
        auto dist = std::distance(vectorTable.cbegin(), result);
        return std::make_pair(dist / OUTER_BOARD_SIZE, dist % OUTER_BOARD_SIZE);
    }
    return std::make_pair(-1, -1);
}

int Board::findCorner_1D() const {
    auto result = std::find_if(
    vectorTable.cbegin(), vectorTable.cend(), 
    [](const auto& sq) {
        auto pc = sq->getPiece();
        return (!pc || (pc->getColour() != Colour::UNKNOWN && pc->getType() != PieceTypes::UNKNOWN));
    });
    
    if (result != vectorTable.cend()) {
        return std::distance(vectorTable.cbegin(), result);
    }
    return -1;
}

void Board::printBoardState() const {
#ifndef DEBUG
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
        for (int j = 0; j < range; ++j) {
#ifdef DEBUG
            std::cout << "--------";
#else
            std::cout << "---";
#endif
        }
        std::cout << "-\n|";
#ifdef DEBUG
        std::for_each(vectorTable.cbegin() + (i * range), vectorTable.cbegin() + ((i + 1) * range), 
                [](const auto& sq){std::cout << *sq << '|';});
#else
        std::for_each(outputTable.cbegin() + (i * range), outputTable.cbegin() + ((i + 1) * range), 
                [](const auto& sq){std::cout << *sq << '|';});
#endif
        std::cout << std::endl;
    }

    for (int k = 0; k < range; ++k) {
#ifdef DEBUG
        std::cout << "--------";
#else
        std::cout << "---";
#endif
    }
    std::cout << "-\n";
}

void Board::shiftHorizontal(const int count) {
    if (!count) {
        return;
    }
    auto startCoords = findCorner();
    if (startCoords.second + count < 0 
        || startCoords.second + count + INNER_BOARD_SIZE - 1 > OUTER_BOARD_SIZE) {
        std::cerr << "Invalid horizontal board shift; No movement performed" << std::endl;
        return;
    }
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
    if (startCoords.first + count < 0 
        || startCoords.first + count + INNER_BOARD_SIZE - 1 > OUTER_BOARD_SIZE) {
        std::cerr << "Invalid vertical board shift; No movement performed" << std::endl;
        return;
    }
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
    const auto& startCoords = findCorner();
    const auto colDiff = ZERO_LOCATION.first - (startCoords.second + col);
    const auto rowDiff = ZERO_LOCATION.second - (startCoords.first + row);
    shiftHorizontal(colDiff);
    shiftVertical(rowDiff);
    for (int i = 0; i < OUTER_BOARD_SIZE; ++i) {
        for (int j = 0; j < OUTER_BOARD_SIZE; ++j) {
            vectorTable[(i* OUTER_BOARD_SIZE) + j]->setOffset(genOffset(i, j));
        }
    }
}

void Board::makeMove(std::string& input) {
    auto mv = moveGen->createMove(input);
    
    shiftBoard(input[0], INNER_BOARD_SIZE - 1 - input[1]);
    
    if (!moveGen->validateMove(mv, false)) {
        return;
    }
    ensureEnPassantValid();
    
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
    
    const auto distToFromSquare = std::distance(vectorTable.cbegin(), 
        std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
            [&mv](const auto& sq){return (*sq == *mv.fromSq);}));
                
    const auto distToEndSquare = std::distance(vectorTable.cbegin(), 
        std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
            [&mv](const auto& sq){return (*sq == *mv.toSq);}));
    
    // If en passant move is made, capture the appropriate pawn
    if (enPassantActive && mv.fromPieceType == PieceTypes::PAWN 
            && (diff % 15) && *mv.toSq == *enPassantTarget) {
                
        const int captureIndex = distToEndSquare + ((isWhiteTurn) ? 15: -15);
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
            + (convertOuterBoardIndex(distToEnPassantTarget, cornerIndex) % 8)];
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
    
    if (enPassantTarget) {
        mv.enPassantFileNum = (distToEndSquare % OUTER_BOARD_SIZE) 
            - static_cast<int>(INNER_BOARD_SIZE - 1 - input[0]);
    } else {
        mv.enPassantFileNum = 0;
    }
    
    const bool castleDirectionChosen = moveGen->getCastleDirectionBool(
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
        
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToFromSquare + 3 - (isQueenSide * 7), cornerIndex)
            + pieceLookupTable[PieceTypes::ROOK] + blackFromPieceHashOffset];
            
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToFromSquare + 1 - (isQueenSide << 1), cornerIndex)
            + pieceLookupTable[PieceTypes::ROOK] + blackFromPieceHashOffset];
            
        if (mv.fromPieceColour == Colour::WHITE) {
            currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
            castleRights &= ~WHITE_CASTLE_FLAG;
            currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
        } else {
            currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
            castleRights &= ~BLACK_CASTLE_FLAG;
            currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
        }
    }
    // Disable castling if the appropriate rook moves
    if (mv.fromPieceType == PieceTypes::ROOK) {
        if (mv.fromPieceColour == Colour::WHITE && (castleRights & WHITE_CASTLE_FLAG)) {
            const int backRankIndex = findCorner_1D() + (7 * OUTER_BOARD_SIZE);
            const auto& fromSquare = *mv.fromSq;
            if (*(vectorTable[backRankIndex].get()) == fromSquare) {
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
                castleRights &= ~WHITE_CASTLE_QUEEN_FLAG;
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
            }
            if (*(vectorTable[backRankIndex + 7].get()) == fromSquare) {
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
                castleRights &= ~WHITE_CASTLE_KING_FLAG;
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
            }
        } else if (mv.fromPieceColour == Colour::BLACK && (castleRights & BLACK_CASTLE_FLAG)) {
            const int backRankIndex = findCorner_1D();
            const auto& fromSquare = *mv.fromSq;
            if (*(vectorTable[backRankIndex].get()) == fromSquare) {
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
                castleRights &= ~BLACK_CASTLE_QUEEN_FLAG;
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
            }
            if (*(vectorTable[backRankIndex + 7].get()) == fromSquare) {
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
                castleRights &= ~BLACK_CASTLE_KING_FLAG;
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
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
        moveCounter++;
    }
    
    moveGen->generateAll();
    
    // For testing purposes, display list of opponents legal moves
    //for (const auto& mo : moveGen->getMoveList()) {
        //if (mo.fromPiece && mo.fromPiece->getType()== PieceTypes::PAWN 
                //&& mo.promotionType != mo.fromPiece->getType()) {
            //std::cout << *mo.fromSq << ", " << *mo.toSq << " Promoting to: " << static_cast<char>(mo.promotionType) << std::endl;
        //} else {
            //std::cout << *mo.fromSq << ", " << *mo.toSq << std::endl;
        //}
    //}
    //std::cout << moveGen->getMoveList().size() << std::endl;
    
    std::rotate(repititionList.begin(), repititionList.begin() + 1, repititionList.end());
    repititionList[repititionList.size() - 1] = currHash;
    
    //Opponent has no legal moves
    if (!moveGen->generateAll().size()) {
        const auto opponentCheck = (!isWhiteTurn) ? whiteInCheck : blackInCheck;
        if (opponentCheck) {
            //Checkmate
            std::cout << "CHECKMATE" << std::endl;
            currentGameState = GameState::MATE;
            return;
        }
        //Stalemate
        std::cout << "STALEMATE" << std::endl;
        currentGameState = GameState::DRAWN;
        return;
    }
    if (halfMoveClock >= 100) {
        //50 move rule
        std::cout << "DRAW" << std::endl;
        std::cout << "50 move rule" << std::endl;
        currentGameState = GameState::DRAWN;
        return;
    }
    
    if (repititionList[0] == repititionList[4] && repititionList[4] == repititionList[8]) {
        //Three move Repitition
        std::cout << "DRAW" << std::endl;
        std::cout << "Three move repitition" << std::endl;
        currentGameState = GameState::DRAWN;
        return;
    }
    
    if (drawByMaterial()) {
        //Insufficient Material
        std::cout << "DRAW" << std::endl;
        std::cout << "Insufficient Material" << std::endl;
        currentGameState = GameState::DRAWN;
        return;
    }
}

void Board::makeMove(Move mv) {
    const auto& cornerCoords = findCorner_1D();
    const auto blackFromPieceHashOffset = ((mv.fromPieceColour == Colour::WHITE) ? 0 : 6);
    const auto blackToPieceHashOffset = ((mv.toPieceColour == Colour::WHITE) ? 0 : 6);
    int row = -1; 
    int col = -1;
    
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (*vectorTable[cornerCoords + (i * OUTER_BOARD_SIZE) + j] == *mv.fromSq) {
                row = i;
                col = j;
                goto end;
            }
        }
    }
    end:
    
    shiftBoard(col, row);
    
    if (!moveGen->validateMove(mv, true)) {
        return;
    }
    
    ensureEnPassantValid();
    
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
    
    const auto distToFromSquare = std::distance(vectorTable.cbegin(), 
        std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
            [&mv](const auto& sq){return (*sq == *mv.fromSq);}));
                
    const auto distToEndSquare = std::distance(vectorTable.cbegin(), 
        std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
            [&mv](const auto& sq){return (*sq == *mv.toSq);}));
    
    const int captureIndex = distToEndSquare + ((isWhiteTurn) ? 15: -15);
    // If en passant move is made, capture the appropriate pawn
    if (enPassantActive && mv.fromPieceType == PieceTypes::PAWN 
            && (diff % 15) && *mv.toSq == *enPassantTarget
            && vectorTable[captureIndex]->getPiece() 
            && vectorTable[captureIndex]->getPiece()->getColour() != mv.fromPieceColour) {
        vectorTable[captureIndex]->setPiece(nullptr);
        
        //xor out the captured pawn
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(captureIndex, cornerIndex)
            + pieceLookupTable[PieceTypes::PAWN] + ((isWhiteTurn) ? 6 : 0)];
    }
    if (enPassantActive) {
        const auto distToEnPassantTarget = std::distance(vectorTable.cbegin(), 
            std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
                [=](const auto& sq){return (*sq == *enPassantTarget);}));
    
        //xor out en passant file
        currHash ^= HASH_VALUES[static_cast<int>(SquareState::EN_PASSANT_FILE) 
            + (convertOuterBoardIndex(distToEnPassantTarget, cornerIndex) % 8)];
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
            + (distToEndSquare % OUTER_BOARD_SIZE) - (INNER_BOARD_SIZE - 1 - col)];
    }
    
    if (enPassantTarget) {
        mv.enPassantFileNum = (distToEndSquare % OUTER_BOARD_SIZE) - (INNER_BOARD_SIZE - 1 - col);
    } else {
        mv.enPassantFileNum = 0;
    }
    
    const bool castleDirectionChosen = moveGen->getCastleDirectionBool(mv.fromPieceType, mv.fromPieceColour, diff);
    
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
            currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
            castleRights &= ~WHITE_CASTLE_FLAG;
            currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
        } else {
            currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
            castleRights &= ~BLACK_CASTLE_FLAG;
            currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
        }
    }
    
    // Disable castling if the appropriate rook moves
    if (mv.fromPieceType == PieceTypes::ROOK) {
        if (mv.fromPieceColour == Colour::WHITE && (castleRights & WHITE_CASTLE_FLAG)) {
            const int backRankIndex = findCorner_1D() + (7 * OUTER_BOARD_SIZE);
            const auto& fromSquare = *mv.fromSq;
            if (*(vectorTable[backRankIndex].get()) == fromSquare) {
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
                castleRights &= ~WHITE_CASTLE_QUEEN_FLAG;
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
            }
            if (*(vectorTable[backRankIndex + 7].get()) == fromSquare) {
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
                castleRights &= ~WHITE_CASTLE_KING_FLAG;
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
            }
        } else if (mv.fromPieceColour == Colour::BLACK && (castleRights & BLACK_CASTLE_FLAG)) {
            const int backRankIndex = findCorner_1D();
            const auto& fromSquare = *mv.fromSq;
            if (*(vectorTable[backRankIndex].get()) == fromSquare) {
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
                castleRights &= ~BLACK_CASTLE_QUEEN_FLAG;
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
            }
            if (*(vectorTable[backRankIndex + 7].get()) == fromSquare) {
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
                castleRights &= ~BLACK_CASTLE_KING_FLAG;
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
            }
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
        moveCounter++;
    }
    
    std::rotate(repititionList.begin(), repititionList.begin() + 1, repititionList.end());
    repititionList[repititionList.size() - 1] = currHash;
}

void Board::unmakeMove(const Move& mv) {
    const auto distToFromSquare = std::distance(vectorTable.cbegin(), 
        std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
            [&mv](const auto& sq){return (*sq == *mv.fromSq);}));
                
    const auto distToEndSquare = std::distance(vectorTable.cbegin(), 
        std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
            [&mv](const auto& sq){return (*sq == *mv.toSq);}));
            
    const auto blackFromPieceHashOffset = ((mv.fromPieceColour == Colour::WHITE) ? 0 : 6);
    const auto blackToPieceHashOffset = ((mv.toPieceColour == Colour::WHITE) ? 0 : 6);
            
    auto distToOldTarget = 0;
    
    if (enPassantTarget) {
        distToOldTarget = std::distance(vectorTable.cbegin(), 
            std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
                [this](const auto& sq){return (*sq == *enPassantTarget);}));
    }
    
    const auto cornerIndex = findCorner_1D();
    const auto diff = mv.toSq->getOffset() - mv.fromSq->getOffset();
    
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
    
    if (mv.captureMade) {
        mv.toSq->setPiece({mv.toPieceType, mv.toPieceColour});
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToEndSquare, cornerIndex)
            + pieceLookupTable[mv.toPieceType] + blackToPieceHashOffset];
    }
    
    const auto innerBoardIndex = convertOuterBoardIndex(distToFromSquare 
        + diff - 15 + (30 * (diff < 0)), cornerIndex);
        
    if (enPassantActive) {
        if (mv.enPassantActive) {
            //1-1 full swap
            currHash ^= HASH_VALUES[static_cast<int>(SquareState::EN_PASSANT_FILE) 
                + (convertOuterBoardIndex(distToOldTarget, cornerIndex) % 8)];
            currHash ^= HASH_VALUES[static_cast<int>(SquareState::EN_PASSANT_FILE) 
                + (innerBoardIndex % 8)];
        } else {
            //1-0, undo old file only
            currHash ^= HASH_VALUES[static_cast<int>(SquareState::EN_PASSANT_FILE) 
                + (convertOuterBoardIndex(distToOldTarget, cornerIndex) % 8)];
        }
    } else if (mv.enPassantActive) {
        //0-1, add in new file only
        currHash ^= HASH_VALUES[static_cast<int>(SquareState::EN_PASSANT_FILE) 
            + (innerBoardIndex % 8)];
    }

    //En passant Capture was made.
    if (mv.enPassantActive 
            && mv.enPassantTarget 
            && mv.fromPieceType == PieceTypes::PAWN 
            && (diff % 15) 
            && !mv.toSq->getPiece()
            && mv.toSq->getOffset() == mv.enPassantTarget->getOffset()) {
        
        //Calculate the square containing the captured pawn and put it back
        const auto capturedColour = (mv.fromPieceColour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
        vectorTable[distToFromSquare + diff - 15 + (30 * (diff < 0))]->setPiece({PieceTypes::PAWN, capturedColour});
        
        //Hashing in the captured pawn
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES 
            * innerBoardIndex + pieceLookupTable[PieceTypes::PAWN] 
            + ((capturedColour == Colour::WHITE) ? 0 : 6)];
    }
    
    enPassantActive = mv.enPassantActive;
    enPassantTarget = mv.enPassantTarget;

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

        currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
        castleRights = mv.castleRights;
        currHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + castleRights];
        
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
    
    if (mv.fromPieceColour == Colour::WHITE) {
        moveCounter--;
    }
    
    halfMoveClock = mv.halfMoveClock;
}

void Board::ensureEnPassantValid() const {
    try {
        if (enPassantActive && !enPassantTarget) {
            throw std::logic_error("En passant target should never be null when en passant is active");
        }
    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        throw e;
    }
}

std::string Board::generateFEN() const {
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
        if (i % 15 == 14) {
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
        const auto distToCurrSquare = std::distance(vectorTable.cbegin(), 
            std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
            [this](const auto& currSquare){
                return *enPassantTarget == *currSquare;
            })
        );
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

size_t std::hash<Board>::operator()(const Board& b) const {
    if (b.currHash) {
        return b.currHash;
    }
    
    const auto& cornerCoords = b.findCorner();
    const int cornerIndex = (cornerCoords.first * 15) + cornerCoords.second;
    const auto& cornerPiece = b.vectorTable[cornerIndex]->getPiece();
    size_t newHash = 0;
    if (cornerPiece) {
        newHash = HASH_VALUES[pieceLookupTable[cornerPiece->getType()] 
            + ((cornerPiece->getColour() == Colour::WHITE) ? 0: 6)];
    }
    
    for (int i = cornerCoords.first; i < cornerCoords.first + INNER_BOARD_SIZE; ++i) {
        for (int j = cornerCoords.second; j < cornerCoords.second + INNER_BOARD_SIZE; ++j) {
            if (i == cornerCoords.first && j == cornerCoords.second) {
                continue;
            }
            //Black is (white hash + 6) for equivalent piece types
            if (b.vectorTable[(i * OUTER_BOARD_SIZE) + j]->getPiece()) {
                const auto& currPiece = b.vectorTable[(i * OUTER_BOARD_SIZE) + j]->getPiece();

                newHash ^= HASH_VALUES[NUM_SQUARE_STATES 
                    * b.convertOuterBoardIndex((i * OUTER_BOARD_SIZE) + j, cornerIndex)
                    + pieceLookupTable[currPiece->getType()] 
                    + ((currPiece->getColour() == Colour::WHITE) ? 0 : 6)];
            }
        }
    }
    if (b.isWhiteTurn) {
        newHash ^= HASH_VALUES[static_cast<int>(SquareState::WHITE_MOVE)];
    }
    
    newHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + b.castleRights];
    
    if (b.enPassantActive) {
         //Calculate en passant file
        const auto targetIndex = std::distance(b.vectorTable.cbegin(), 
            std::find_if(b.vectorTable.cbegin(), b.vectorTable.cend(), 
            [&b](const auto& sq){
                return *sq == *b.enPassantTarget;
            }
        ));
        const int fileNum = (targetIndex % 15) - cornerCoords.second;
        newHash ^= HASH_VALUES[static_cast<int>(SquareState::EN_PASSANT_FILE) + fileNum];
    }
    return newHash;
}

/*
 * Convert 15x15 board index that references square on the inner board, and
 * convert it to the relative index of the inner board.
 * Eg. If outerIndex references a2, this method returns the relative position
 * of a2 regardless of the current board shift state.
 */
int Board::convertOuterBoardIndex(const int outerIndex, const int cornerIndex) const {
    const auto remain = outerIndex - cornerIndex;
    return ((remain / OUTER_BOARD_SIZE) * INNER_BOARD_SIZE) 
        + (outerIndex % OUTER_BOARD_SIZE) - (cornerIndex % OUTER_BOARD_SIZE);
}

bool operator==(const Board& first, const Board& second) {
    return (first.currHash == second.currHash);
}

bool Board::drawByMaterial() const {
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
        std::cout << "Pawn promotion detected" << std::endl;
        std::cout << "Knight = [n/N], Bishop = [b/B], Rook = [r/R], Queen = [q/Q]" << std::endl;
        std::cout << "Enter choice for promotion:" << std::endl;
        std::getline(std::cin, input);
        std::transform(input.begin(), input.end(), input.begin(), ::toupper);
        if (input.length() == 1 && std::regex_match(input, reg)) {
            break;
        }
        std::cout << "Invalid input" << std::endl;
    }
    return input;
}

void Board::updateCheckStatus() {
    const auto blackKingDist = std::distance(vectorTable.cbegin(), 
        std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
            [](const auto& sq){
                const auto& piece = sq->getPiece();
                return piece && piece->getType() == PieceTypes::KING 
                    && piece->getColour() == Colour::BLACK;
            }));
    
    const auto whiteKingDist = std::distance(vectorTable.cbegin(), 
        std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
            [](const auto& sq){
                const auto& piece = sq->getPiece();
                return piece && piece->getType() == PieceTypes::KING 
                    && piece->getColour() == Colour::WHITE;
            }));
    
    blackInCheck = moveGen->inCheck(blackKingDist);
    whiteInCheck = moveGen->inCheck(whiteKingDist);
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
            if (std::isupper(fenSections[0][currStrPos])) {
                //White piece
                switch (fenSections[0][currStrPos]) {
                    case 'P':
                        vectorTable[cornerDist + (OUTER_BOARD_SIZE * i) 
                            + currSquareIdx]->setPiece({PieceTypes::PAWN, Colour::WHITE});
                        break;
                    case 'N':
                        vectorTable[cornerDist + (OUTER_BOARD_SIZE * i) 
                                + currSquareIdx]->setPiece({PieceTypes::KNIGHT, Colour::WHITE});
                        break;
                    case 'B':
                        vectorTable[cornerDist + (OUTER_BOARD_SIZE * i) 
                                + currSquareIdx]->setPiece({PieceTypes::BISHOP, Colour::WHITE});
                        break;
                    case 'R':
                        vectorTable[cornerDist + (OUTER_BOARD_SIZE * i) 
                            + currSquareIdx]->setPiece({PieceTypes::ROOK, Colour::WHITE});
                        break;
                    case 'Q':
                        vectorTable[cornerDist + (OUTER_BOARD_SIZE * i) 
                            + currSquareIdx]->setPiece({PieceTypes::QUEEN, Colour::WHITE});
                        break;
                    case 'K':
                        vectorTable[cornerDist + (OUTER_BOARD_SIZE * i) 
                            + currSquareIdx]->setPiece({PieceTypes::KING, Colour::WHITE});
                        break;
                    default:
                        break;
                }
            } else {
                //Black piece
                switch (fenSections[0][currStrPos]) {
                    case 'p':
                        vectorTable[cornerDist + (OUTER_BOARD_SIZE * i) 
                            + currSquareIdx]->setPiece({PieceTypes::PAWN, Colour::BLACK});
                        break;
                    case 'n':
                        vectorTable[cornerDist + (OUTER_BOARD_SIZE * i) 
                                + currSquareIdx]->setPiece({PieceTypes::KNIGHT, Colour::BLACK});
                        break;
                    case 'b':
                        vectorTable[cornerDist + (OUTER_BOARD_SIZE * i) 
                                + currSquareIdx]->setPiece({PieceTypes::BISHOP, Colour::BLACK});
                        break;
                    case 'r':
                        vectorTable[cornerDist + (OUTER_BOARD_SIZE * i) 
                            + currSquareIdx]->setPiece({PieceTypes::ROOK, Colour::BLACK});
                        break;
                    case 'q':
                        vectorTable[cornerDist + (OUTER_BOARD_SIZE * i) 
                            + currSquareIdx]->setPiece({PieceTypes::QUEEN, Colour::BLACK});
                        break;
                    case 'k':
                        vectorTable[cornerDist + (OUTER_BOARD_SIZE * i) 
                            + currSquareIdx]->setPiece({PieceTypes::KING, Colour::BLACK});
                        break;
                    default:
                        break;
                }
            }
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
}

//Testing method used to assert board state
bool Board::checkBoardValidity() {
    int cornerCoords;
    if ((cornerCoords = findCorner_1D()) == -1) {
        std::cerr << "Could not find corner\n";
        return false;
    }
    for (int i = 0; i < OUTER_BOARD_SIZE; ++i) {
        for (int j = 0; j < OUTER_BOARD_SIZE; ++j) {
            if (!vectorTable[cornerCoords + (i * OUTER_BOARD_SIZE) + j]) {
                std::cerr << "Board square is null\n";
                return false;
            }
            if (vectorTable[cornerCoords + (i * OUTER_BOARD_SIZE) + j]->checkSentinel()) {
                std::cerr << "Square is sentinel when it shouldn't be\n";
                return false;
            }
        }
    }
    Board temp(*this);
    temp.currHash = 0;
    if (currHash != std::hash<Board>()(temp)) {
        std::cerr << "Calculated hash does not match current board hash\n";
        return false;
    }
    return true;
}
