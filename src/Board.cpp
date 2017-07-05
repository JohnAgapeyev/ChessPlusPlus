#include <vector>
#include <array>
#include <iostream>
#include <iomanip>
#include <regex>
#include <cassert>
#include <string>
#include <omp.h>
#include "headers/board.h"
#include "headers/square.h"
#include "headers/consts.h"
#include "headers/enums.h"
#include "headers/move.h"

/*
 * Offset = 98 - (15 * i) + (j + 1)
 * IMPORTANT:
 * Positive x: (30 * ((x + 7) / 15)) - x
 * Negative x: -((30 * ((abs(x) + 7) / 15)) - abs(x))
 */
 
/**
 * Board constructor performs a deep copy of the initial board state.
 * It also initializes the repitionList and the current board hash.
 */
Board::Board() {
    for (int i = 0; i < OUTER_BOARD_SIZE; ++i) {
        for (int j = 0; j < OUTER_BOARD_SIZE; ++j) {
            if (i >= 0 && i <= 7) {
                if (j >= 0 && j <= 7) {
                    vectorTable[(i * OUTER_BOARD_SIZE) + j] = std::make_unique<Square>(*INIT_BOARD[i][j]);
                    vectorTable[(i * OUTER_BOARD_SIZE) + j]->setOffset(genOffset(i, j));
                } else {
                    vectorTable[(i * OUTER_BOARD_SIZE) + j] = std::make_unique<Square>(genOffset(i, j));
                }
            } else {
                vectorTable[(i * OUTER_BOARD_SIZE) + j] = std::make_unique<Square>(genOffset(i, j));
            }
        }
    }
    
    for (size_t i = 0; i < repititionList.size(); ++i) {
        repititionList[i] = i;
    }
    currHash = std::hash<Board>()(*this);
}

/**
 * Copy constructor performs a deep board copy of another board.
 * This is not used outside of threading specific board creation.
 */
Board::Board(const Board& b) : moveGen(this), currentGameState(b.currentGameState), castleRights(b.castleRights), 
        blackInCheck(b.blackInCheck), whiteInCheck(b.whiteInCheck), isWhiteTurn(b.isWhiteTurn), enPassantActive(b.enPassantActive), 
        halfMoveClock(b.halfMoveClock), moveCounter(b.moveCounter), 
        currHash(b.currHash), repititionList(b.repititionList), cornerCache(b.cornerCache) {

    for (int i = 0; i < TOTAL_BOARD_SIZE; ++i) {
        vectorTable[i] = std::make_unique<Square>(*b.vectorTable[i]);
    }

    if (b.enPassantTarget) {
        const auto targetIndex = const_cast<Board&>(b).getSquareIndex(b.enPassantTarget);
        assert(targetIndex != -1);
        enPassantTarget = vectorTable[targetIndex].get();
    } else {
        enPassantTarget = nullptr;
    }

    assert(checkBoardValidity());
}

/**
 * Deep copy copy assignment operator.
 * Same as above, not used outside of threading purposes.
 */
Board& Board::operator=(const Board& b) {
    moveGen = std::move(MoveGenerator{this});
    currentGameState = b.currentGameState;
    castleRights = b.castleRights;
    blackInCheck = b.blackInCheck;
    whiteInCheck = b.whiteInCheck;
    isWhiteTurn = b.isWhiteTurn;
    enPassantActive = b.enPassantActive;
    if (b.enPassantTarget) {
        const auto targetIndex = const_cast<Board&>(b).getSquareIndex(b.enPassantTarget);
        assert(targetIndex != -1);
        enPassantTarget = vectorTable[targetIndex].get();
    } else {
        enPassantTarget = nullptr;
    }
    halfMoveClock = b.halfMoveClock;
    moveCounter = b.moveCounter;
    currHash = b.currHash;
    repititionList = b.repititionList;
    cornerCache = b.cornerCache;
    for (int i = 0; i < TOTAL_BOARD_SIZE; ++i) {
        vectorTable[i] = std::make_unique<Square>(*b.vectorTable[i]);
    }
    assert(checkBoardValidity());
    return *this;
}

/**
 * Due to the nature of the board shifting around inside of the array, this function returns
 * the 2D coordinates of the corner of the board.
 */
std::pair<int, int> Board::findCorner() {
    if (cornerCache != -1) {
        return {cornerCache / OUTER_BOARD_SIZE, cornerCache % OUTER_BOARD_SIZE};
    }
    for (size_t i = 0; i < TOTAL_BOARD_SIZE; ++i) {
        if (!vectorTable[i]->checkSentinel()) {
            cornerCache = i;
            return {i / OUTER_BOARD_SIZE, i % OUTER_BOARD_SIZE};
        }
    }
    return {-1, -1};
}

/**
 * Same as above, but returns a 1D index of the board corner.
 */
int Board::findCorner_1D() {
    if (cornerCache != -1) {
        return cornerCache;
    }
    for (size_t i = 0; i < TOTAL_BOARD_SIZE; ++i) {
        if (!vectorTable[i]->checkSentinel()) {
            cornerCache = i;
            return i; 
        }
    }
    return -1;
}

/**
 * Main output method for displaying the board.
 */
void Board::printBoardState() const {
#ifdef NDEBUG
    const auto range = INNER_BOARD_SIZE;
    std::vector<Square *> outputTable;
    for (size_t i = 0; i < TOTAL_BOARD_SIZE; ++i) {
        const auto pc = vectorTable[i]->getPiece();
        if (!(pc && pc->getColour() == Colour::UNKNOWN && pc->getType() == PieceTypes::UNKNOWN)) {
            outputTable.push_back(vectorTable[i].get());
        }
    }
#else
    const auto range = OUTER_BOARD_SIZE;
#endif
    for (auto i = 0; i < range; ++i) {
#ifdef NDEBUG
        std::cout << "  ";
#endif
#pragma omp parallel for schedule(static)
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

/**
 * Shifts the board horizontally by a given count.
 */
void Board::shiftHorizontal(const int count) {
    if (!count) {
        return;
    }
    auto startCoords = findCorner();
    
    assert(!(startCoords.second + count < 0 
        || startCoords.second + count + INNER_BOARD_SIZE - 1 > OUTER_BOARD_SIZE));

    for (int i = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0; j < INNER_BOARD_SIZE; ++j) {
            int col = INNER_BOARD_SIZE - 1 - i + ((count <= 0) * ((2 * i) - (INNER_BOARD_SIZE - 1)));
            int row = INNER_BOARD_SIZE - 1 - j + ((count <= 0) * ((2 * j) - (INNER_BOARD_SIZE - 1)));
            
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

/**
 * Shifts the board vertically by a given count.
 */
void Board::shiftVertical(const int count) {
    if (!count) {
        return;
    }
    auto startCoords = findCorner();
    
    assert(!(startCoords.first + count < 0 
        || startCoords.first + count + INNER_BOARD_SIZE - 1 > OUTER_BOARD_SIZE));
    
    for (int i = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0; j < INNER_BOARD_SIZE; ++j) {
            int col = INNER_BOARD_SIZE - 1 - i + ((count <= 0) * ((2 * i) - (INNER_BOARD_SIZE - 1)));
            int row = INNER_BOARD_SIZE - 1 - j + ((count <= 0) * ((2 * j) - (INNER_BOARD_SIZE - 1)));
            
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

/**
 * The nature of the board requires that the current square a piece is moving
 * from has the square offset of 0, which is equal to 7,7 on the 15x15 board.
 * This method calls the above 2 shift calls to shift a given coordinate square to the
 * center of the board.
 */
void Board::shiftBoard(const int col, const int row) {
    const auto startCoords = findCorner();
    const auto colDiff = ZERO_LOCATION.first - (startCoords.second + col);
    const auto rowDiff = ZERO_LOCATION.second - (startCoords.first + row);
    shiftHorizontal(colDiff);
    cornerCache = -1; //Invalidate corner cache after shifting the board
    shiftVertical(rowDiff);
    cornerCache = -1; //Invalidate corner cache after shifting the board

    for (int i = 0; i < OUTER_BOARD_SIZE; ++i) {
        for (int j = 0; j < OUTER_BOARD_SIZE; ++j) {
            vectorTable[(i* OUTER_BOARD_SIZE) + j]->setOffset(genOffset(i, j));
        }
    }
}

/**
 * Takes a given user input and performs the move it indicates.
 * This method is only used when interacting with a user.
 */
bool Board::makeMove(std::string& input) {
    assert(checkBoardValidity());
    auto mv = moveGen.createMove(input);
    
    shiftBoard(input[0], INNER_BOARD_SIZE - 1 - input[1]);
    
    if (!moveGen.validateMove(mv, false)) {
        return false;
    }
    
    mv.halfMoveClock = halfMoveClock;
    mv.moveCounter = moveCounter;
    
    ++halfMoveClock;
    
    const auto cornerIndex = findCorner_1D();
    const auto diff = mv.toSq->getOffset() - mv.fromSq->getOffset();
    
    const auto distToFromSquare = getSquareIndex(mv.fromSq);
    const auto distToEndSquare = getSquareIndex(mv.toSq);

    assert(distToFromSquare != -1);
    assert(distToEndSquare != -1);

    captureEnPassant(mv, diff, distToEndSquare);

    if (enPassantActive) {
        const auto distToEnPassantTarget = getSquareIndex(enPassantTarget);
        assert(distToEnPassantTarget != -1);
                
        //xor out en passant file
        hashEnPassantFile(convertOuterBoardIndex(distToEnPassantTarget, cornerIndex) % INNER_BOARD_SIZE);
    }
    
    mv.enPassantActive = enPassantActive;
    mv.enPassantTarget = enPassantTarget;
    
    enPassantActive = false;
    enPassantTarget = nullptr;

    addEnPassantTarget(mv, diff, input[0], distToEndSquare);

    performCastling(mv, diff, distToFromSquare);
    
    disableCastling(mv);

    promotePawn(mv, distToEndSquare, false);
    
    // If moving to an occupied square, capture the piece
    if (mv.toSq->getPiece()) {
        mv.toSq->setPiece(nullptr);
        halfMoveClock = 0;
        hashPieceChange(convertOuterBoardIndex(distToEndSquare, cornerIndex), mv.toPieceType, mv.toPieceColour);
    }
    //xor out from piece at old square
    hashPieceChange(convertOuterBoardIndex(distToFromSquare, cornerIndex), (mv.promotionMade) ? PieceTypes::PAWN : mv.fromPieceType, mv.fromPieceColour);

    //xor in from piece at new square
    hashPieceChange(convertOuterBoardIndex(distToEndSquare, cornerIndex), mv.fromPieceType, mv.fromPieceColour);

    std::swap(*mv.fromSq, *mv.toSq);
    swapOffsets(mv);
    isWhiteTurn = !isWhiteTurn;
    
    //hash the change in turn
    hashTurnChange();

    moveCounter += isWhiteTurn;
    
    updateCheckStatus();
    
    std::rotate(repititionList.begin(), repititionList.begin() + 1, repititionList.end());
    repititionList[repititionList.size() - 1] = currHash;

    detectGameEnd();

    assert(checkBoardValidity());
    return true;
}

/**
 * This method makes a move provided to it.
 * In contrast to the above method, this is only ever called from the AI side
 * of things.
 * This separation of user and automated move making is done for small tweaks related to
 * movement such as whether to prompt the user or not for promotion types, etc.
 */
bool Board::makeMove(Move& mv) {
    assert(checkBoardValidity());
    assert(mv.fromSq && mv.toSq);
    
    const auto oldFromDist = getSquareIndex(mv.fromSq);
    assert(oldFromDist != -1);
    const auto cornerCoords = findCorner_1D();
    assert(cornerCoords != -1);

    const std::pair<int, int> shiftCoords{(oldFromDist - cornerCoords) / OUTER_BOARD_SIZE, 
        (oldFromDist - cornerCoords) % OUTER_BOARD_SIZE};

    assert(shiftCoords.first != -1);
    assert(shiftCoords.second != -1);
    shiftBoard(shiftCoords.second, shiftCoords.first);

    assert(moveGen.validateMove(mv, false));

    mv.halfMoveClock = halfMoveClock;
    mv.moveCounter = moveCounter;

    ++halfMoveClock;
    
    const auto cornerIndex = findCorner_1D();
    const auto diff = mv.toSq->getOffset() - mv.fromSq->getOffset();
    
    const auto distToFromSquare = getSquareIndex(mv.fromSq);
    const auto distToEndSquare = getSquareIndex(mv.toSq);

    assert(distToFromSquare != -1);
    assert(distToEndSquare != -1);

    captureEnPassant(mv, diff, distToEndSquare);
    
    if (enPassantActive) {
        const auto distToEnPassantTarget = getSquareIndex(enPassantTarget);
        assert(distToEnPassantTarget != -1);
                
        //xor out en passant file
        hashEnPassantFile(convertOuterBoardIndex(distToEnPassantTarget, cornerIndex) % INNER_BOARD_SIZE);
    }

    mv.enPassantActive = enPassantActive;
    mv.enPassantTarget = enPassantTarget;
    
    enPassantActive = false;
    enPassantTarget = nullptr;

    addEnPassantTarget(mv, diff, shiftCoords.second, distToEndSquare);

    performCastling(mv, diff, distToFromSquare);
    
    disableCastling(mv);

    promotePawn(mv, distToEndSquare, true);
    
    // If moving to an occupied square, capture the piece
    if (mv.toSq->getPiece()) {
        mv.toSq->setPiece(nullptr);
        halfMoveClock = 0;
        hashPieceChange(convertOuterBoardIndex(distToEndSquare, cornerIndex), mv.toPieceType, mv.toPieceColour);
    }
    //xor out from piece at old square
    hashPieceChange(convertOuterBoardIndex(distToFromSquare, cornerIndex), (mv.promotionMade) ? PieceTypes::PAWN: mv.fromPieceType, mv.fromPieceColour);
    //xor in from piece at new square
    hashPieceChange(convertOuterBoardIndex(distToEndSquare, cornerIndex), mv.fromPieceType, mv.fromPieceColour);

    std::swap(*mv.fromSq, *mv.toSq);
    swapOffsets(mv);
    isWhiteTurn = !isWhiteTurn;
    
    //hash the change in turn
    hashTurnChange();

    moveCounter += isWhiteTurn;
    
    updateCheckStatus();
    
    std::rotate(repititionList.begin(), repititionList.begin() + 1, repititionList.end());
    repititionList[repititionList.size() - 1] = currHash;
    
    assert(checkBoardValidity());
    return true;
}

/**
 * Unmakes a given move for the current board.
 * This is only ever called by the chess engine during tree traversal.
 * If the move provided is invalid or has not been immeditely made previously, 
 * the behaviour is undefined.
 */
void Board::unmakeMove(const Move& mv) {
    assert(checkBoardValidity());
    
    const auto distToFromSquare = getSquareIndex(mv.fromSq);
    const auto distToEndSquare = getSquareIndex(mv.toSq);

    assert(distToFromSquare != -1 && distToEndSquare != -1);
                
    auto distToOldTarget = 0;
    
    if (enPassantTarget) {
        distToOldTarget = getSquareIndex(enPassantTarget);
        assert(distToOldTarget != -1);
    }
    const auto cornerIndex = findCorner_1D();
    const auto diff = mv.toSq->getOffset() - mv.fromSq->getOffset();

    std::swap(*mv.fromSq, *mv.toSq);
    swapOffsets(mv);
    isWhiteTurn = !isWhiteTurn;
    
    //hash the change in turn
    hashTurnChange();
    
    //xor out from piece at old square
    hashPieceChange(convertOuterBoardIndex(distToEndSquare, cornerIndex), mv.fromPieceType, mv.fromPieceColour);

    if (mv.promotionMade) {
        mv.fromSq->setPiece({PieceTypes::PAWN, mv.fromPieceColour});
        //xor in from piece at new square
        hashPieceChange(convertOuterBoardIndex(distToFromSquare, cornerIndex), PieceTypes::PAWN, mv.fromPieceColour);
    } else {
        //xor in from piece at new square
        hashPieceChange(convertOuterBoardIndex(distToFromSquare, cornerIndex), mv.fromPieceType, mv.fromPieceColour);
    }
        
    if (mv.captureMade) {
        mv.toSq->setPiece({mv.toPieceType, mv.toPieceColour});
        hashPieceChange(convertOuterBoardIndex(distToEndSquare, cornerIndex), mv.toPieceType, mv.toPieceColour);
    }
    
    if (enPassantActive) {
        //Hash out the current file
        hashEnPassantFile((convertOuterBoardIndex(distToOldTarget, cornerIndex) % INNER_BOARD_SIZE));
    }
    
    if (mv.enPassantTarget) {
        //Hash in the previous file num
        const auto distToCurrTarget = getSquareIndex(mv.enPassantTarget);
        assert(distToCurrTarget != -1);
        const auto fileNum = convertOuterBoardIndex(distToCurrTarget, cornerIndex) % INNER_BOARD_SIZE;
        
        hashEnPassantFile(fileNum);
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
        hashPieceChange(convertOuterBoardIndex(enPassantCaptureIndex, cornerIndex), PieceTypes::PAWN, capturedColour);
    }
    
    enPassantActive = mv.enPassantActive;
    enPassantTarget = mv.enPassantTarget;
    
    if (mv.isCastle || mv.fromPieceType == PieceTypes::ROOK || mv.fromPieceType == PieceTypes::KING) {
        hashCastleRights();
        castleRights = mv.castleRights;
        hashCastleRights();
    }
    
    if (mv.isCastle) {
        const auto isQueenSide = (mv.toSq->getOffset() - mv.fromSq->getOffset() < 0);
        
        std::swap(vectorTable[distToEndSquare - 1 + (isQueenSide << 1)], 
            vectorTable[distToEndSquare + 1 - (isQueenSide * 3)]);
            
        const auto temp = vectorTable[distToEndSquare - 1 + (isQueenSide << 1)]->getOffset();
        
        vectorTable[distToEndSquare - 1 + (isQueenSide << 1)]->setOffset(
            vectorTable[distToEndSquare + 1 - (isQueenSide * 3)]->getOffset());
            
        vectorTable[distToEndSquare + 1 - (isQueenSide * 3)]->setOffset(temp);

        hashPieceChange(convertOuterBoardIndex(distToFromSquare + 3 - (isQueenSide * 7), cornerIndex), PieceTypes::ROOK, mv.fromPieceColour);
        hashPieceChange(convertOuterBoardIndex(distToFromSquare + 1 - (isQueenSide << 1), cornerIndex), PieceTypes::ROOK, mv.fromPieceColour);
    }

    std::rotate(repititionList.rbegin(), repititionList.rbegin() + 1, repititionList.rend());
    repititionList[0] = currHash;
    
    updateCheckStatus();

    halfMoveClock = mv.halfMoveClock;
    moveCounter = mv.moveCounter;
    
    assert(checkBoardValidity());
}

/**
 * Generates a FEN string represnting the current board state.
 */
std::string Board::generateFEN() {
    std::string output;
    int emptySquareCounter = 0;
    int rowCount = 1;
    bool rowContainsGameBoard = false;
    for (size_t i = 0; i < TOTAL_BOARD_SIZE; ++i) {
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

/**
 * Counts the material on the board to check if it would cause a draw due to
 * insufficient material.
 */
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

/**
 * Prompts the user to enter a character representing their desire promotion type.
 */
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

/**
 * Updates the current check status of the board after a move is made or unmade.
 */
void Board::updateCheckStatus() {
    const auto corner = findCorner_1D();
    int whiteIndex = -1;
    int blackIndex = -1;
    for (int i = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0; j < INNER_BOARD_SIZE; ++j) {
            const auto piece = vectorTable[corner + (i * OUTER_BOARD_SIZE) + j]->getPiece();
            if (piece && piece->getType() == PieceTypes::KING) {
                if (piece->getColour() == Colour::WHITE) {
                    assert(whiteIndex == -1); //Ensure no multiple kings exist
                    whiteIndex = corner + (i * OUTER_BOARD_SIZE) + j;
                } else {
                    assert(blackIndex == -1); //Ensure no multiple kings exist
                    blackIndex = corner + (i * OUTER_BOARD_SIZE) + j;
                }
            }
        }
    }
    assert(whiteIndex != -1);
    assert(blackIndex != -1);
    whiteInCheck = moveGen.inCheck(whiteIndex);
    blackInCheck = moveGen.inCheck(blackIndex);
}

/**
 * Sets a board's position based on a given FEN string.
 * This method is used only for perft testing, and has no validation that the
 * position is a valid chess position, or that the input string is valid.
 * As such, it is not publicly exposed to the end user in the interface.
 */
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
    
    //Resetting the check status
    blackInCheck = false;
    whiteInCheck = false;
    updateCheckStatus();
}

/**
 * This method is a testing method used for validating the current board
 * state is as it should be.
 * This includes checks for en passant having a valid target when it is active,
 * ensuring the 8x8 board is consistent and contains no sentinel values.
 * The most important check this method performs is a forced reset validation of the board hash,
 * ensuring the incremental update is fully equivalent to a complete rehash.
 */
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
    
    const auto tempHash = currHash;
    currHash = 0;
    currHash = std::hash<Board>()(*this);

    if (currHash != tempHash) {
        std::cerr << "Calculated hash does not match current board hash\n";
        std::cout << currHash << "\t" << tempHash << "\n";
        return false;
    }
    return true;
}

/**
 * Converts a given square pointer to text.
 * If a square pointer points to g3, returns the text "g3".
 */
std::string Board::convertSquareToCoordText(const Square *sq) {
    const auto cornerCoords = findCorner_1D();
    const auto squareDist = getSquareIndex(sq);
    assert(squareDist != -1);
    const auto innerDist = convertOuterBoardIndex(squareDist, cornerCoords);
    
    return static_cast<char>('a' + (innerDist % INNER_BOARD_SIZE)) 
        + std::to_string(INNER_BOARD_SIZE - (innerDist / INNER_BOARD_SIZE));
}

/**
 * Complement to the above method, converts the from and to squares to text.
 */
std::string Board::convertMoveToCoordText(const Move& mv) {
    return convertSquareToCoordText(mv.fromSq) + convertSquareToCoordText(mv.toSq);
}

/**
 * Performs a linear search on the board to find a given square
 * based on a pointer to it.
 * It only searches for valid on-board squares as it searches only the 8x8 board, instead of
 * the full 15x15.
 */
int Board::getSquareIndex(const Square *sq) {
    if (!sq) {
        return -1;
    }

    const auto cornerIndex = findCorner_1D();

    assert(cornerIndex >= 0);
    assert(cornerIndex < 225);

    for (int i = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0; j < INNER_BOARD_SIZE; ++j) {
            assert((cornerIndex + (i * OUTER_BOARD_SIZE) + j) < 225);
            assert((cornerIndex + (i * OUTER_BOARD_SIZE) + j) >= 0);
            if (*vectorTable[cornerIndex + (i * OUTER_BOARD_SIZE) + j].get() == *sq) {
                return cornerIndex + (i * OUTER_BOARD_SIZE) + j;
            }
        }
    }
    return -1;
}

/**
 * Checks whether en passant is valid after a given move based on an en passant capture
 * resulting in uncovering a pin, resulting in check, which is an invalid move.
 */
bool Board::checkEnPassantValidity(Square *sq, const Move& mv) {
    //Enemy pawn and enemy king - perform check check on left square
    std::unique_ptr<Piece> temp{sq->releasePiece()};

    const auto cornerIndex = findCorner_1D();

    //Get opposite colour king position
    int idx = -1;
    for (int i = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0; j < INNER_BOARD_SIZE; ++j) {
            const auto piece = vectorTable[cornerIndex + (i * OUTER_BOARD_SIZE) + j]->getPiece();
            if (piece && piece->getType() == PieceTypes::KING && piece->getColour() != mv.fromPieceColour) {
                idx = cornerIndex + (i * OUTER_BOARD_SIZE) + j;
                break;
            }
        }
    }
    assert(idx != -1);

    const auto result = moveGen.inCheck(idx);
    sq->setPiece(std::move(temp));

    return !result;
}

/**
 * Used during move making to remove castling rights from the board.
 */
void Board::removeCastlingRights(const unsigned char flag) {
    hashCastleRights();
    castleRights &= ~flag;
    hashCastleRights();
}

/**
 * This method disables castling based on the piece that moved.
 */
void Board::disableCastling(const Move& mv) {
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
}

/**
 * Adds an en passant target to the board is it is valid.
 * Part of that validity check involves calling checkEnPassantValidity on the neighbouring squares.
 */
void Board::addEnPassantTarget(const Move& mv, const int offset, const int columnNum, const int endSquareIndex) {
    // Add en Passant target if pawn double move was made.
    if (std::abs(offset) == 30 && mv.fromPieceType == PieceTypes::PAWN) {
        auto left = vectorTable[endSquareIndex - 1].get();
        auto right = vectorTable[endSquareIndex + 1].get();

        if (left->getPiece() || right->getPiece()) {
            if (left->getPiece() && right->getPiece()) {
                const auto isLeftPawn = (left->getPiece()->getType() == PieceTypes::PAWN 
                        && left->getPiece()->getColour() != mv.fromPieceColour);
                const auto isRightPawn = (right->getPiece()->getType() == PieceTypes::PAWN 
                        && right->getPiece()->getColour() != mv.fromPieceColour);
                if (isLeftPawn ^ isRightPawn) {
                    if (isLeftPawn) {
                        //Check right square for its piece type
                        if (right->getPiece()->getVectorLength() == INNER_BOARD_SIZE 
                                || (right->getPiece()->getType() == PieceTypes::KING 
                                    && right->getPiece()->getColour() != mv.fromPieceColour)) {
                            //Enemy pawn and enemy king - perform check check on left square
                            if (!checkEnPassantValidity(left, mv)) {
                                return;
                            }
                        }
                    } else {
                        //Check left square for its piece type
                        if (left->getPiece()->getVectorLength() == INNER_BOARD_SIZE 
                                || (left->getPiece()->getType() == PieceTypes::KING 
                                    && left->getPiece()->getColour() != mv.fromPieceColour)) {
                            //Enemy pawn and enemy king - perform check check on right square
                            if (!checkEnPassantValidity(right, mv)) {
                                return;
                            }
                        }
                    }
                }
            } else if (left->getPiece() && left->getPiece()->getType() == PieceTypes::PAWN 
                    && left->getPiece()->getColour() != mv.fromPieceColour) {
                //Perform check check on left square
                if (!checkEnPassantValidity(left, mv)) {
                    return;
                }
            } else if (right->getPiece() && right->getPiece()->getType() == PieceTypes::PAWN 
                    && right->getPiece()->getColour() != mv.fromPieceColour) {
                //Perform check check on right square
                if (!checkEnPassantValidity(right, mv)) {
                    return;
                }
            }
        }

        enPassantActive = true;
        enPassantTarget = vectorTable[endSquareIndex + (offset / 2)].get();
        
        //xor in en passant file
        hashEnPassantFile((endSquareIndex % OUTER_BOARD_SIZE) - (INNER_BOARD_SIZE - 1 - columnNum));
    }
}

/**
 * Performs castling on the board for a given move.
 */
void Board::performCastling(Move& mv, const int offset, const int fromSquareIndex) {
    const bool castleDirectionChosen = moveGen.getCastleDirectionBool(mv.fromPieceType, mv.fromPieceColour, offset);
    //Perform castling
    if (mv.fromPieceType == PieceTypes::KING && std::abs(offset) == 2 && castleDirectionChosen) {
        const auto cornerIndex = findCorner_1D();
        mv.isCastle = true;
        
        const auto isQueenSide = (offset < 0);
        
        std::swap(vectorTable[fromSquareIndex + 3 - (isQueenSide * 7)], 
            vectorTable[fromSquareIndex + 1 - (isQueenSide << 1)]);
        const auto temp = vectorTable[fromSquareIndex + 3 - (isQueenSide * 7)]->getOffset();
        vectorTable[fromSquareIndex + 3 - (isQueenSide * 7)]->setOffset(
            vectorTable[fromSquareIndex + 1 - (isQueenSide << 1)]->getOffset());
        vectorTable[fromSquareIndex + 1 - (isQueenSide << 1)]->setOffset(temp);
        
        hashPieceChange(convertOuterBoardIndex(fromSquareIndex + 3 - (isQueenSide * 7), cornerIndex), 
                PieceTypes::ROOK, mv.fromPieceColour);
        hashPieceChange(convertOuterBoardIndex(fromSquareIndex + 1 - (isQueenSide << 1), cornerIndex), 
                PieceTypes::ROOK, mv.fromPieceColour);
            
        if (mv.fromPieceColour == Colour::WHITE) {
            removeCastlingRights(WHITE_CASTLE_FLAG);
        } else {
            removeCastlingRights(BLACK_CASTLE_FLAG);
        }
    }
}

/**
 * Performs an en passant catpure and removes the captured piece.
 */
void Board::captureEnPassant(const Move& mv, const int offset, const int toSquareIndex) {
    const auto captureIndex = toSquareIndex - OUTER_BOARD_SIZE + ((isWhiteTurn) * OUTER_BOARD_SIZE << 1);
    const auto cornerIndex = findCorner_1D();
    // If en passant move is made, capture the appropriate pawn
    if (enPassantActive && mv.fromPieceType == PieceTypes::PAWN 
            && (offset % OUTER_BOARD_SIZE) && *mv.toSq == *enPassantTarget
            && vectorTable[captureIndex]->getPiece() 
            && vectorTable[captureIndex]->getPiece()->getColour() != mv.fromPieceColour) {
        vectorTable[captureIndex]->setPiece(nullptr);
        
        //xor out the captured pawn
        hashPieceChange(convertOuterBoardIndex(captureIndex, cornerIndex), PieceTypes::PAWN, 
                (mv.fromPieceColour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE);
    }
}

/**
 * Incrementally updates the hashbased on a change in the piece structure of the board.
 * Every time a piece moves, it must be hashed out of its old square and hashed into its new square.
 * The hashing call is done to this function.
 */
inline void Board::hashPieceChange(const int index, const PieceTypes type, const Colour colour) {
    assert(pieceLookupTable.find(type) != pieceLookupTable.end());
    assert(index >= 0);
    assert(pieceLookupTable.find(type)->second >= 0);
    currHash ^= HASH_VALUES[NUM_SQUARE_STATES * index + pieceLookupTable.find(type)->second + (6 * (colour == Colour::BLACK))];
}

/**
 * Incrementally updates the hash based on the change in turn.
 */
inline void Board::hashTurnChange() {
    currHash ^= HASH_VALUES[static_cast<unsigned int>(SquareState::WHITE_MOVE)];
}

/**
 * Incrementally updates the hash based on the change in the en passant file
 */
inline void Board::hashEnPassantFile(const int fileNum) {
    currHash ^= HASH_VALUES[static_cast<unsigned int>(SquareState::EN_PASSANT_FILE) + fileNum];
}

/**
 * Incrementally updates the hash based on the change in castling rights.
 */
inline void Board::hashCastleRights() {
    currHash ^= HASH_VALUES[static_cast<unsigned int>(SquareState::CASTLE_RIGHTS) + castleRights];
}

/**
 * Promotes a pawn to a given type when it is appropriate to do so.
 */
void Board::promotePawn(Move& mv, const int endSquareIndex, const bool isSilent) {
    const auto cornerIndex = findCorner_1D();
    //Promote pawns on the end ranks
    if (mv.fromPieceType == PieceTypes::PAWN) {
        halfMoveClock = 0;
        const auto distFromStartToCorner = endSquareIndex - cornerIndex;
        const auto rowPairs = (mv.fromPieceColour == Colour::WHITE) ? std::make_pair(0, 14) : std::make_pair(105, 119);
        
        if (distFromStartToCorner >= rowPairs.first && distFromStartToCorner <= rowPairs.second) {
            if (isSilent) {
                mv.fromSq->getPiece()->promote(mv.promotionType);
                mv.fromPieceType = static_cast<PieceTypes>(mv.promotionType);
            } else {
                const auto& input = promptPromotionType();
                mv.fromSq->getPiece()->promote(static_cast<PieceTypes>(input.front()));
                mv.fromPieceType = static_cast<PieceTypes>(input.front());
            }
            mv.promotionMade = true;
        }
    }
}

/**
 * Detects if the board is at an end state, the type of that state, and ends the
 * current game in progress if one has been reached.
 */
void Board::detectGameEnd() {
    //Opponent has no legal moves
    if (!moveGen.generateAll().size()) {
        const auto opponentCheck = (isWhiteTurn) ? whiteInCheck : blackInCheck;
        if (opponentCheck) {
            //Checkmate
            std::cout << "CHECKMATE\n";
            currentGameState = GameState::MATE;
            return;
        }
        //Stalemate
        std::cout << "STALEMATE\n";
        currentGameState = GameState::DRAWN;
        return;
    }
    if (halfMoveClock >= 100) {
        //50 move rule
        std::cout << "DRAW\n";
        std::cout << "50 move rule\n";
        currentGameState = GameState::DRAWN;
        return;
    }
    
    if (repititionList[0] == repititionList[4] && repititionList[4] == repititionList[8]) {
        //Three move Repitition
        std::cout << "DRAW\n";
        std::cout << "Three move repitition\n";
        currentGameState = GameState::DRAWN;
        return;
    }
    
    if (drawByMaterial()) {
        //Insufficient Material
        std::cout << "DRAW\n";
        std::cout << "Insufficient Material\n";
        currentGameState = GameState::DRAWN;
        return;
    }
}

