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
#include <functional>

Move Board::MoveGenerator::createMove(std::string& input) {
    // If the characters are letters, convert them to digit chars
    if (input[0] > 8 && input[2] > 8) {
        input[0] -= 49;
        input[2] -= 49;
    }
    // Change input to zero-indexed value
    input[1] -= 1;
    input[3] -= 1;
    // Change chars to digits
    for (auto& ch : input) {
        ch -= '0';
    }
    Move result;
    const auto topLeftCornerIndex = board.findCorner_1D();
    
    result.fromSq = board.vectorTable[((INNER_BOARD_SIZE - 1 - input[1]) * OUTER_BOARD_SIZE) 
            + topLeftCornerIndex + input[0]].get();
            
    result.toSq = board.vectorTable[((INNER_BOARD_SIZE - 1 - input[3]) * OUTER_BOARD_SIZE) 
            + topLeftCornerIndex + input[2]].get();
    
    result.fromPiece = result.fromSq->getPiece();
    result.toPiece = result.toSq->getPiece();
    
    if (result.toPiece) {
        result.toPieceType = result.toPiece->getType();
        result.toPieceColour = result.toPiece->getColour();
    } else {
        result.toPieceType = PieceTypes::UNKNOWN;
        result.toPieceColour = Colour::UNKNOWN;
    }
    
    result.promotionType = (result.fromSq->getPiece()) ? result.fromSq->getPiece()->getType() : PieceTypes::UNKNOWN;
    
    result.isCastle = false;
    result.castleRights = board.castleRights;
    result.enPassantActive = false;
    result.enPassantTarget = nullptr;
    result.enPassantFileNum = 0;
    
    return result;
}
 /*
  * IMPORTANT:
  * Positive x: (30 * ((x + 7) / 15)) - x
  * Negative x: -((30 * ((abs(x) + 7) / 15)) - abs(x))
  * This function represents the number of spaces needed to be subtracted
  * from 0 to reach the corresponding offset square based on the fact
  * that the board is represented in a 1-D array with offsets increasing
  * left to right 
  * x is the desired offset square to find.
  * This method is critical for part of this validation method, and as such
  * I am commenting it here as a backup just in case the code gets altered and
  * breaks everything.
  */
bool Board::MoveGenerator::validateMove(const Move& mv, const bool isSilent) {
    const auto& firstSquare = std::find_if(board.vectorTable.cbegin(), 
        board.vectorTable.cend(), [&mv](const auto& sq){
            return (*sq == *mv.fromSq);
        });
    const auto& secondSquare = std::find_if(board.vectorTable.cbegin(), 
        board.vectorTable.cend(), [&mv](const auto& sq){
            return (*sq == *mv.toSq);
        });
    // Try to find the start and end points
    if (firstSquare == board.vectorTable.cend() || secondSquare == board.vectorTable.cend()) {
        if (!isSilent) {
            std::cerr << "Could not find start or end squares" << std::endl;
        }
        return false;
    }
    
    // Check for either square being a sentinel
    if (mv.fromSq->checkSentinel() || mv.toSq->checkSentinel()) {
        if (!isSilent) {
            std::cerr << "You somehow referenced a sentinel square for your move. GJ" << std::endl;
        }
        return false;
    }
    
    if (!mv.fromPiece || (mv.fromPiece->getType() == PieceTypes::UNKNOWN 
            && mv.fromPiece->getColour() == Colour::UNKNOWN)) {
        if (!isSilent) {
            std::cout << "Cannot start a move on an empty square" << std::endl;
        }
        return false;
    }
    
    const auto& fromPieceType = mv.fromPiece->getType();
    const auto& fromPieceColour = mv.fromPiece->getColour();
    
    // Check if piece being moved matches the current player's colour
    if ((fromPieceColour == Colour::WHITE && !board.isWhiteTurn) 
            || (fromPieceColour == Colour::BLACK && board.isWhiteTurn)) {
        if (!isSilent) {
            if (board.isWhiteTurn) {
                std::cout << "Cannot move black piece on white's turn\n";
            } else {
                std::cout << "Cannot move white piece on black's turn\n";
            }
        }
        return false;
    }
    
    const auto& vectorOffsets = mv.fromPiece->getVectorList();
    const auto diff = mv.toSq->getOffset() - mv.fromSq->getOffset();
    const auto& secondSquareIndex = std::distance(board.vectorTable.cbegin(), secondSquare);
    
    // Find the offset that the move uses
    const auto& selectedOffset = std::find_if(vectorOffsets.cbegin(), 
        vectorOffsets.cend(), [diff](auto offset) {
            return (diff / offset > 0 && diff / offset < 8 && !(diff % offset));
        }
    );
    
    // Check if the move offset was found
    if (selectedOffset == vectorOffsets.cend()) {
        logMoveFailure(1, isSilent);
        return false;
    }
    
    const auto absOffset = std::abs(*selectedOffset);
    
    /*
     * Check if the colour of the piece on the starting square 
     * is the same colour as the piece on the ending square.
     */
    if (mv.toPiece && fromPieceColour == mv.toPiece->getColour()) {
        logMoveFailure(2, isSilent);
        return false;
    }
    
    const auto isDoublePawnMove = (fromPieceType == PieceTypes::PAWN && absOffset == 30);
    
    // Define number of squares to check along the selected vector
    const int moveLen = mv.fromPiece->getVectorLength() + isDoublePawnMove;
    
    auto currSquare = board.vectorTable[0].get();
    auto foundToSquare = false;
    
    // Iterate through to ensure sliding pieces aren't being blocked
    for (int i = 1; i < moveLen; ++i) {
        const auto realIndex = getOffsetIndex(*selectedOffset >> isDoublePawnMove, ZERO_LOCATION_1D, i);
        if (realIndex < 0 || realIndex >= OUTER_BOARD_SIZE * OUTER_BOARD_SIZE) {
            break;
        }
        currSquare = board.vectorTable[realIndex].get();
        
        if (*currSquare == *mv.toSq) {
            foundToSquare = true;
            break;
        }
        // Check if the square has a piece on it or is a sentinel
        if (currSquare->getPiece()) {
            logMoveFailure(3, isSilent);
            return false;
        }
    }
    // Check if square was found in previous loop
    if (!foundToSquare) {
        logMoveFailure(4, isSilent);
        return false;
    }
    
    /* 
     * Check that pawns don't double move except on their starting rows.
     * Since board is 1D array, finding the pawn row can be done by counting
     * the number of squares between the top left corner and the starting square
     * of the selected move.
     */
    if (absOffset == 30) {
        const auto dist = std::distance(board.vectorTable.cbegin(), firstSquare);
        const auto cornerIndex =  board.findCorner_1D();
        const auto distFromStartToCorner = dist - cornerIndex;
        const auto& rowPairs = (fromPieceColour == Colour::WHITE) ? std::make_pair(90, 104) : std::make_pair(15, 29);
        if (distFromStartToCorner < rowPairs.first || distFromStartToCorner > rowPairs.second) {
            logMoveFailure(5, isSilent);
            return false;
        }
    }
    
    board.ensureEnPassantValid();
    // Pawn related validation checks
    if (fromPieceType == PieceTypes::PAWN) {
        // Ensure pawns only move diagonally if they capture a piece, including en passant
        const int captureOffset = (fromPieceColour == Colour::WHITE) ? 15 : -15;
        
        if ((*selectedOffset % 15) 
                && !board.vectorTable[secondSquareIndex]->getPiece() 
                && !(board.enPassantActive && *board.vectorTable[secondSquareIndex] == *board.enPassantTarget 
                    && board.vectorTable[secondSquareIndex + captureOffset]->getPiece()
                    && board.vectorTable[secondSquareIndex + captureOffset]->getPiece()->getColour() != fromPieceColour)) {
            logMoveFailure(6, isSilent);
            return false;
        }
        // Prevent pawns from capturing vertically
        if (!(*selectedOffset % 15) && board.vectorTable[secondSquareIndex]->getPiece()) {
            logMoveFailure(7, isSilent);
            return false;
        }
    }
    
    //Prevent players from placing their own king in check
    if (inCheck(mv)) {
        logMoveFailure(8, isSilent);
        return false;
    }
    
    const bool castleDirectionChosen = getCastleDirectionBool(fromPieceType, fromPieceColour, *selectedOffset);
    // Prevent king from jumpng 2 spaces if not castling
    if (fromPieceType == PieceTypes::KING && absOffset == 2 && !castleDirectionChosen) {
        logMoveFailure(9, isSilent);
        return false;
    }
    
    if (fromPieceType == PieceTypes::KING && absOffset == 2) {
        const auto currIndex = std::distance(board.vectorTable.cbegin(), firstSquare);
        for (int i = 1; i <= 2; ++i) {
            const auto index = ((*selectedOffset > 0) ? i : -i);
            const Piece *currPiece = (firstSquare[index]) ? firstSquare[index]->getPiece() : nullptr;
            const auto checkIndex = currIndex + index;
            if ((currPiece && currPiece->getType() != PieceTypes::ROOK) || inCheck(checkIndex)) {
                logMoveFailure(10, isSilent);
                return false;
            }
        }
    }
    return true;
}

bool Board::MoveGenerator::inCheck(const int squareIndex) const {
    const auto& checkVectors = Piece(PieceTypes::UNKNOWN, Colour::UNKNOWN).getVectorList();
    const int cornerIndex =  board.findCorner_1D();
    
    /*
     * If square is empty, AKA castling validity check, set friendly colour 
     * based on which half of the board the square is.
     */
    Colour friendlyPieceColour;
    if (!board.vectorTable[squareIndex]->getPiece()) {
        friendlyPieceColour = (squareIndex - cornerIndex >= 52) ? Colour::WHITE : Colour::BLACK;
    } else {
        friendlyPieceColour = board.vectorTable[squareIndex]->getPiece()->getColour();
    }
    
    int vectorLength = 7;
    for (const auto& offset : checkVectors) {
        const auto absOffset = std::abs(offset);
        // Change depth if current offset is a knight offset
        vectorLength = (absOffset == 13 || absOffset > 16) ? 1 : 7;
        
        for (int i = 1; i <= vectorLength; ++i) {
            if (squareIndex + (i * -offset) < 0 
                    || squareIndex + (i * -offset) >= OUTER_BOARD_SIZE * OUTER_BOARD_SIZE) {
                break;
            }
            const auto realIndex = getOffsetIndex(offset, squareIndex, i);
            if (realIndex < 0 || realIndex >= OUTER_BOARD_SIZE * OUTER_BOARD_SIZE) {
                break;
            }
            const auto currSquare = board.vectorTable[realIndex].get();
            const auto& currPiece = currSquare->getPiece();
            if (currPiece) {
                const auto currPieceColour = currPiece->getColour();
                if (currPieceColour == friendlyPieceColour 
                        || currPieceColour == Colour::UNKNOWN) {
                    break;
                }
                const auto& pieceVector = currPiece->getVectorList();
                if (std::find(pieceVector.cbegin(), pieceVector.cend(), offset) == pieceVector.cend()) {
                    break;
                }
                return true;
            }
        }
    }
    return false;
}

bool Board::MoveGenerator::inCheck(const Move& mv) const {
    const auto& checkVectors = Piece(PieceTypes::UNKNOWN, Colour::UNKNOWN).getVectorList();
    Colour friendlyPieceColour = (board.isWhiteTurn) ? Colour::WHITE : Colour::BLACK;
    
    const auto toPiece = mv.toSq->getPiece();
    auto toPieceCopy = Piece(PieceTypes::UNKNOWN, Colour::UNKNOWN);
    if (toPiece) {
        toPieceCopy = Piece(toPiece->getType(), toPiece->getColour());
    }
    
    mv.toSq->setPiece(nullptr);
    std::swap(*mv.fromSq, *mv.toSq);

    const int squareIndex = std::distance(board.vectorTable.cbegin(), 
        std::find_if(board.vectorTable.cbegin(), board.vectorTable.cend(), 
            [friendlyPieceColour](const auto& sq){
                const auto& piece = sq->getPiece();
                return piece && piece->getType() == PieceTypes::KING 
                    && piece->getColour() == friendlyPieceColour;
            }));
    
    int vectorLength = 7;
    for (const auto& offset : checkVectors) {
        const auto absOffset = std::abs(offset);
        // Change depth if current offset is a knight offset
        vectorLength = (absOffset == 13 || absOffset > 16) ? 1 : 7;
        
        for (int i = 1; i <= vectorLength; ++i) {
            const auto realIndex = getOffsetIndex(offset, squareIndex, i);
            if (realIndex < 0 || realIndex >= OUTER_BOARD_SIZE * OUTER_BOARD_SIZE) {
                break;
            }
            const auto currSquare = board.vectorTable[realIndex].get();
            const auto& currPiece = currSquare->getPiece();
            
            if (currPiece) {
                const auto currPieceColour = currPiece->getColour();
                const auto currPieceType = currPiece->getType();
                
                if (currPieceColour == friendlyPieceColour 
                        || currPieceColour == Colour::UNKNOWN) {
                    break;
                }
                //Prevent single space mover pieces from being treated as sliding pieces
                if (i > 1 && (currPieceType == PieceTypes::KING 
                        || currPieceType == PieceTypes::PAWN 
                        || currPieceType == PieceTypes::KNIGHT)) {
                    break;
                }
                
                const auto& pieceVector = currPiece->getVectorList();
                
                if (std::find_if(pieceVector.cbegin(), pieceVector.cend(), 
                        [currPieceType, offset](const auto off){
                            if (currPieceType == PieceTypes::PAWN) {
                                if (off % 15) {
                                    return off == -offset;
                                }
                                return false;
                            }
                            return off == offset;
                        }
                    ) == pieceVector.cend()) {
                    break;
                }
                std::swap(*mv.fromSq, *mv.toSq);
                if (toPiece) {
                    mv.toSq->setPiece(toPieceCopy);
                }
                return true;
            }
        }
    }
    std::swap(*mv.fromSq, *mv.toSq);
    if (toPiece) {
        mv.toSq->setPiece(toPieceCopy);
    }
    return false;
}

int Board::MoveGenerator::getOffsetIndex(const int offset, const int startIndex, 
        const int vectorLen) const {
    constexpr auto THIRTY = OUTER_BOARD_SIZE << 1;
    const auto absOffset = std::abs(offset);
    const auto addSubtract = [=](const auto a, const auto b){
        return (offset > 0) ? a - b : a + b;
    };
    return addSubtract(startIndex, (vectorLen * ((THIRTY 
        * ((absOffset + INNER_BOARD_SIZE - 1) / OUTER_BOARD_SIZE)) - absOffset)));
}

bool Board::MoveGenerator::getCastleDirectionBool(const PieceTypes type, 
        const Colour pieceColour, const int offset) const {
    if (type == PieceTypes::KING && std::abs(offset) == 2) {
        if (pieceColour == Colour::WHITE) {
            return board.castleRights & ((offset < 0) ? WHITE_CASTLE_QUEEN_FLAG : WHITE_CASTLE_KING_FLAG);
        }
        return board.castleRights & ((offset < 0) ? BLACK_CASTLE_QUEEN_FLAG : BLACK_CASTLE_KING_FLAG);
    }
    return false;
}

void Board::MoveGenerator::logMoveFailure(const int failureNum, const bool isSilent) const {
    if (isSilent) {
        return;
    }
#ifdef DEBUG
    std::cout << "Move is not legal " << failureNum << std::endl;
#else
    std::cout << "Move is not legal\n";
#endif
}

void Board::MoveGenerator::generateAll() {
    Move mv{nullptr, nullptr, nullptr, nullptr, PieceTypes::UNKNOWN, 
        Colour::UNKNOWN, PieceTypes::UNKNOWN, false, 0, false, nullptr, 0};
        
    const auto tableSize = static_cast<int>(board.vectorTable.size());
    const auto& currentPlayerColour = (board.isWhiteTurn) ? Colour::WHITE : Colour::BLACK;
    
    const std::vector<std::tuple<int, int, Piece*>> pieceCoords = [&]{
        std::vector<std::tuple<int, int, Piece*>> internal;
        for (int i = 0; i < OUTER_BOARD_SIZE; ++i) {
            for (int j = 0; j < OUTER_BOARD_SIZE; ++j) {
                const auto& sq = board.vectorTable[(i * OUTER_BOARD_SIZE) + j].get();
                const auto& fromPiece = sq->getPiece();
                if (fromPiece && fromPiece->getType() != PieceTypes::UNKNOWN 
                        && fromPiece->getColour() == currentPlayerColour) {
                    internal.push_back(std::make_tuple(i, j, fromPiece));
                }
            }
        }
        return internal;
    }();
    const auto& startCoords = board.findCorner();
    
    moveList.clear();
    
    for (const auto& p : pieceCoords) {
        board.shiftBoard(std::get<1>(p) - startCoords.second, std::get<0>(p) - startCoords.first);
        for (const auto& offset : std::get<2>(p)->getVectorList()) {
             //Define number of squares to check along the selected vector
            const int moveLen = std::get<2>(p)->getVectorLength();
        
            for (int j = 1; j < moveLen; ++j) {
                const auto toSquareIndex = getOffsetIndex(offset, ZERO_LOCATION_1D, j);
                if (toSquareIndex < 0 || toSquareIndex > tableSize) {
                    break;
                }
                mv.fromSq = board.vectorTable[ZERO_LOCATION_1D].get();
                mv.toSq = board.vectorTable[toSquareIndex].get();
                
                mv.fromPiece = mv.fromSq->getPiece();
                mv.toPiece = mv.toSq->getPiece();
                
                if (mv.toPiece) {
                    mv.toPieceType = mv.toPiece->getType();
                    mv.toPieceColour = mv.toPiece->getColour();
                } else {
                    mv.toPieceType = PieceTypes::UNKNOWN;
                    mv.toPieceColour = Colour::UNKNOWN;
                }
                
                mv.promotionType = (mv.fromSq->getPiece()) ? mv.fromSq->getPiece()->getType() : PieceTypes::UNKNOWN;
                
                //Promotion check
                if (mv.promotionType == PieceTypes::PAWN) {
                    const auto distToEndSquare = std::distance(board.vectorTable.cbegin(), 
                        std::find_if(board.vectorTable.cbegin(), board.vectorTable.cend(), 
                            [&mv](const auto& sq){return (*sq == *mv.toSq);}));
                            
                    //Calculate the corner index based on the above board shift to prevent a linear search
                    const auto cornerIndex = ((7 - std::get<0>(p) + startCoords.first) * 15) + (7 - std::get<1>(p) + startCoords.second);
                    const auto distFromStartToCorner = distToEndSquare - cornerIndex;
                    const auto& rowPairs = (mv.fromSq->getPiece()->getColour() == Colour::WHITE) ? std::make_pair(0, 14) : std::make_pair(105, 119);
                    
                    //Promotion validity
                    if (distFromStartToCorner >= rowPairs.first && distFromStartToCorner <= rowPairs.second) {
                        if (mv.toSq->checkSentinel()) {
                            break;
                        }
                        if (!validateMove(mv, true)) {
                            break;
                        }
                        mv.promotionType = PieceTypes::KNIGHT;
                        moveList.push_back(mv);
                        mv.promotionType = PieceTypes::BISHOP;
                        moveList.push_back(mv);
                        mv.promotionType = PieceTypes::ROOK;
                        moveList.push_back(mv);
                        mv.promotionType = PieceTypes::QUEEN;
                        moveList.push_back(mv);
                        continue;
                    }
                }
                
                if (mv.toSq->checkSentinel()) {
                    break;
                }
                
                if (!validateMove(mv, true)) {
                    break;
                }
                if (mv.fromPiece->getType() == PieceTypes::PAWN && !((toSquareIndex - ZERO_LOCATION_1D) % 30)) {
                    mv.enPassantFileNum = std::get<1>(p) - startCoords.second;
                } else {
                    mv.enPassantFileNum = 0;
                }
                
                mv.enPassantActive = board.enPassantActive;
                mv.enPassantTarget = board.enPassantTarget;
                
                if (mv.fromPiece->getType() == PieceTypes::KING 
                        && std::abs((toSquareIndex - ZERO_LOCATION_1D)) == 2 
                        && getCastleDirectionBool(mv.fromPiece->getType(), 
                            mv.fromPiece->getColour(), (toSquareIndex - ZERO_LOCATION_1D))) {
                    mv.isCastle = true;
                    mv.castleRights = board.castleRights;
                }
                
                moveList.push_back(mv);
            }
        }
    }
}
