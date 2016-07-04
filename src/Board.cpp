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
    const auto& mv = moveGen->createMove(input);
    
    shiftBoard(input[0], INNER_BOARD_SIZE - 1 - input[1]);
    
    if (!moveGen->validateMove(mv, false)) {
        return;
    }
    ensureEnPassantValid();
    
    halfMoveClock++;
    
    const int cornerIndex = findCorner_1D();
    const auto diff = mv.toSq->getOffset() - mv.fromSq->getOffset();
    const auto& fromPiece = mv.fromSq->getPiece();
    auto fromPieceType = fromPiece->getType();
    const auto& fromPieceColour = fromPiece->getColour();
    
    const auto distToFromSquare = std::distance(vectorTable.cbegin(), 
        std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
            [&mv](const auto& sq){return (*sq == *mv.fromSq);}));
                
    const auto distToEndSquare = std::distance(vectorTable.cbegin(), 
        std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
            [&mv](const auto& sq){return (*sq == *mv.toSq);}));
    
    // If en passant move is made, capture the appropriate pawn
    if (enPassantActive && fromPieceType == PieceTypes::PAWN 
            && (diff % 15) && *mv.toSq == *enPassantTarget) {
        const int captureIndex = distToEndSquare + ((isWhiteTurn) ? 15: -15);
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
    
    enPassantActive = false;
    enPassantTarget = nullptr;
    
    // Add en Passant target if pawn double move was made.
    if (std::abs(diff) == 30 && fromPieceType == PieceTypes::PAWN) {
        enPassantActive = true;
        enPassantTarget = vectorTable[distToEndSquare + (diff >> 1)].get();
        
        //xor in en passant file
        currHash ^= HASH_VALUES[static_cast<int>(SquareState::EN_PASSANT_FILE) 
            + (distToEndSquare % OUTER_BOARD_SIZE) 
            - static_cast<int>(INNER_BOARD_SIZE - 1 - input[0])];
    }
    
    const bool castleDirectionChosen = moveGen->getCastleDirectionBool(
            fromPieceType, fromPieceColour, diff);
    
    //Perform castling
    if (fromPieceType == PieceTypes::KING && std::abs(diff) == 2 && castleDirectionChosen) {
        if (diff > 0) {
            //kingside
            std::swap(vectorTable[distToFromSquare + 3], vectorTable[distToFromSquare + 1]);
            const auto temp = vectorTable[distToFromSquare + 3]->getOffset();
            vectorTable[distToFromSquare + 3]->setOffset(vectorTable[distToFromSquare + 1]->getOffset());
            vectorTable[distToFromSquare + 1]->setOffset(temp);
            
            currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToFromSquare + 3, cornerIndex)
                + pieceLookupTable[PieceTypes::ROOK] + ((fromPieceColour == Colour::WHITE) ? 0 : 6)];
            currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToFromSquare + 1, cornerIndex)
                + pieceLookupTable[PieceTypes::ROOK] + ((fromPieceColour == Colour::WHITE) ? 0 : 6)];
        } else {
            //queenside
            std::swap(vectorTable[distToFromSquare - 4], vectorTable[distToFromSquare - 1]);
            const auto temp = vectorTable[distToFromSquare - 4]->getOffset();
            vectorTable[distToFromSquare - 4]->setOffset(vectorTable[distToFromSquare - 1]->getOffset());
            vectorTable[distToFromSquare - 1]->setOffset(temp);
            
            currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToFromSquare - 4, cornerIndex)
                + pieceLookupTable[PieceTypes::ROOK] + ((fromPieceColour == Colour::WHITE) ? 0 : 6)];
            currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToFromSquare - 1, cornerIndex)
                + pieceLookupTable[PieceTypes::ROOK] + ((fromPieceColour == Colour::WHITE) ? 0 : 6)];
        }
        if (fromPieceColour == Colour::WHITE) {
            whiteCastleKing = false;
            whiteCastleQueen = false;
            currHash ^= HASH_VALUES[static_cast<int>(SquareState::WHITE_CASTLE_KING)];
            currHash ^= HASH_VALUES[static_cast<int>(SquareState::WHITE_CASTLE_QUEEN)];
        } else {
            blackCastleKing = false;
            blackCastleQueen = false;
            currHash ^= HASH_VALUES[static_cast<int>(SquareState::BLACK_CASTLE_KING)];
            currHash ^= HASH_VALUES[static_cast<int>(SquareState::BLACK_CASTLE_QUEEN)];
        }
    }
    // Disable castling if the appropriate rook moves
    if (fromPieceType == PieceTypes::ROOK) {
        if (fromPieceColour == Colour::WHITE && (whiteCastleKing || whiteCastleQueen)) {
            const int backRankIndex = findCorner_1D() + (7 * OUTER_BOARD_SIZE);
            const auto& fromSquare = *mv.fromSq;
            if (*(vectorTable[backRankIndex].get()) == fromSquare) {
                whiteCastleQueen = false;
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::WHITE_CASTLE_QUEEN)];
            }
            if (*(vectorTable[backRankIndex + 7].get()) == fromSquare) {
                whiteCastleKing = false;
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::WHITE_CASTLE_KING)];
            }
        } else if (fromPieceColour == Colour::BLACK && (blackCastleKing || blackCastleQueen)) {
            const int backRankIndex = findCorner_1D();
            const auto& fromSquare = *mv.fromSq;
            if (*(vectorTable[backRankIndex].get()) == fromSquare) {
                blackCastleQueen = false;
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::BLACK_CASTLE_QUEEN)];
            }
            if (*(vectorTable[backRankIndex + 7].get()) == fromSquare) {
                blackCastleKing = false;
                currHash ^= HASH_VALUES[static_cast<int>(SquareState::BLACK_CASTLE_KING)];
            }
        }
    }
    
    if (fromPieceType == PieceTypes::PAWN) {
        halfMoveClock = 0;
        const auto cornerIndex =  findCorner_1D();
        const auto distFromStartToCorner = distToEndSquare - cornerIndex;
        const auto& rowPairs = (fromPieceColour == Colour::WHITE) ? std::make_pair(0, 14) : std::make_pair(105, 119);
        
        if (distFromStartToCorner >= rowPairs.first && distFromStartToCorner <= rowPairs.second) {
            //Prompt for promotion
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
            mv.fromSq->getPiece()->promote(static_cast<PieceTypes>(input.front()));
            fromPieceType = static_cast<PieceTypes>(input.front());
        }
    }
    // If moving to an occupied square, capture the piece
    if (mv.toSq->getPiece()) {
        currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToEndSquare, cornerIndex)
            + pieceLookupTable[mv.toSq->getPiece()->getType()] + ((mv.toSq->getPiece()->getColour() == Colour::WHITE) ? 0 : 6)];
        mv.toSq->setPiece(nullptr);
        halfMoveClock = 0;
    }
    //xor out from piece at old square
    currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToFromSquare, cornerIndex)
        + pieceLookupTable[fromPieceType] + ((fromPieceColour == Colour::WHITE) ? 0 : 6)];
        
    //xor in from piece at new square
    currHash ^= HASH_VALUES[NUM_SQUARE_STATES * convertOuterBoardIndex(distToEndSquare, cornerIndex)
        + pieceLookupTable[fromPieceType] + ((fromPieceColour == Colour::WHITE) ? 0 : 6)];
        
    std::swap(*mv.fromSq, *mv.toSq);
    swapOffsets(mv);
    isWhiteTurn = !isWhiteTurn;
    
    //xor the change in turn
    currHash ^= HASH_VALUES[static_cast<int>(SquareState::WHITE_MOVE)];
    
    if (isWhiteTurn) {
        moveCounter++;
    }
    
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
    
    moveGen->generateAll();
    
    // For testing purposes, display list of opponents legal moves
    //for (const auto& mo : moveGen->getMoveList()) {
        //const auto& moveFromPiece = mo.fromSq->getPiece();
        //if (moveFromPiece && moveFromPiece->getType() == PieceTypes::PAWN 
                //&& mo.promotionType != moveFromPiece->getType()) {
            //std::cout << *mo.fromSq << ", " << *mo.toSq << " Promoting to: " << static_cast<char>(mo.promotionType) << std::endl;
        //} else {
            //std::cout << *mo.fromSq << ", " << *mo.toSq << std::endl;
        //}
    //}
    //std::cout << moveGen->getMoveList().size() << std::endl;
    
    std::rotate(repititionList.begin(), repititionList.begin() + 1, repititionList.end());
    repititionList[repititionList.size() - 1] = std::hash<Board>()(*this);
    
    //Opponent has no legal moves
    if (!moveGen->getMoveList().size()) {
        const auto& opponentCheck = (!isWhiteTurn) ? whiteInCheck : blackInCheck;
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
    bool castleFlagsActive = false;
    if (whiteCastleKing) {
        output += 'K';
        castleFlagsActive = true;
    }
    if (whiteCastleQueen) {
        output += 'Q';
        castleFlagsActive = true;
    }
    if (blackCastleKing) {
        output += 'k';
        castleFlagsActive = true;
    }
    if (blackCastleQueen) {
        output += 'q';
        castleFlagsActive = true;
    } 
    if (!castleFlagsActive) {
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
    size_t newHash = HASH_VALUES[NUM_SQUARE_STATES 
        + pieceLookupTable[cornerPiece->getType()] 
        + ((cornerPiece->getColour() == Colour::WHITE) ? 0: 6)];
    
    for (int i = cornerCoords.first; i < cornerCoords.first + INNER_BOARD_SIZE; ++i) {
        for (int j = cornerCoords.second; j < cornerCoords.second + INNER_BOARD_SIZE; ++j) {
            if (i == cornerCoords.first && j == cornerCoords.second) {
                continue;
            }
            //Black is (white hash + 6) for equivalent piece types
            if (b.vectorTable[i * 15 + j]->getPiece()) {
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
    if (b.whiteCastleKing) {
        newHash ^= HASH_VALUES[static_cast<int>(SquareState::WHITE_CASTLE_KING)];
    }
    if (b.whiteCastleQueen) {
        newHash ^= HASH_VALUES[static_cast<int>(SquareState::WHITE_CASTLE_QUEEN)];
    }
    if (b.blackCastleKing) {
        newHash ^= HASH_VALUES[static_cast<int>(SquareState::BLACK_CASTLE_KING)];
    }
    if (b.blackCastleQueen) {
        newHash ^= HASH_VALUES[static_cast<int>(SquareState::BLACK_CASTLE_QUEEN)];
    }
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
        if ((((whiteBishopCoords.first & 1) + (whiteBishopCoords.second & 1)) & 1) 
            == (((blackBishopCoords.first & 1) + (blackBishopCoords.second & 1)) & 1)) {
            return true;
        }
    }
    return false;
}
