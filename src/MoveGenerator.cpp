#include <vector>
#include <array>
#include <iostream>
#include <algorithm>
#include <cassert>
#include "headers/board.h"
#include "headers/square.h"
#include "headers/consts.h"
#include "headers/enums.h"
#include "headers/move.h"

Move Board::MoveGenerator::createMove(std::string& input) const {
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
    
    if (result.fromSq->getPiece()) {
        result.fromPieceType = result.fromSq->getPiece()->getType();
        result.fromPieceColour = result.fromSq->getPiece()->getColour();
    } else {
        result.fromPieceType = PieceTypes::UNKNOWN;
        result.fromPieceColour = Colour::UNKNOWN;
    }
    if (result.toSq->getPiece()) {
        result.toPieceType = result.toSq->getPiece()->getType();
        result.toPieceColour = result.toSq->getPiece()->getColour();
        result.captureMade = true;
    } else {
        result.toPieceType = PieceTypes::UNKNOWN;
        result.toPieceColour = Colour::UNKNOWN;
        result.captureMade = false;
    }
    
    result.promotionType = (result.fromSq->getPiece()) ? result.fromSq->getPiece()->getType() : PieceTypes::UNKNOWN;
    result.promotionMade = false;
    
    result.isCastle = false;
    result.castleRights = board.castleRights;
    result.enPassantActive = false;
    result.enPassantTarget = nullptr;
    result.halfMoveClock = 0;
    result.moveCounter = 0;
    
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
    // Try to find the start and end points
    assert(!(board.getSquareIndex(mv.fromSq) == -1 || board.getSquareIndex(mv.toSq) == -1));
    
    // Check for either square being a sentinel
    assert(!(mv.fromSq->checkSentinel() || mv.toSq->checkSentinel()));
    
    //Don't perform basic checks if the computer is generating the moves
    if (!isSilent) {
        if (!mv.fromSq->getPiece() || (mv.fromPieceType == PieceTypes::UNKNOWN 
                && mv.fromPieceColour == Colour::UNKNOWN)) {
            std::cout << "Cannot start a move on an empty square\n";
            return false;
        }
        
        // Check if piece being moved matches the current player's colour
        if ((mv.fromPieceColour == Colour::WHITE && !board.isWhiteTurn) 
                || (mv.fromPieceColour == Colour::BLACK && board.isWhiteTurn)) {
            if (board.isWhiteTurn) {
                std::cout << "Cannot move black piece on white's turn\n";
            } else {
                std::cout << "Cannot move white piece on black's turn\n";
            }
            return false;
        }

        /*
         * Check if the colour of the piece on the starting square 
         * is the same colour as the piece on the ending square.
         */
        if (mv.toSq->getPiece() && mv.fromPieceColour == mv.toPieceColour) {
            logMoveFailure(1, isSilent);
            return false;
        }
        
        //Prevent pieces from capturing a king
        if (mv.toPieceType == PieceTypes::KING) {
            logMoveFailure(2, isSilent);
            return false;
        }
    }
    
    // Find the offset that the move uses
    const int selectedOffset = getMoveOffset(mv);

    // Check if the move offset was found
    assert(selectedOffset != -100);
    
    const auto absOffset = std::abs(selectedOffset);
    
    const auto isPawnMove = (mv.fromPieceType == PieceTypes::PAWN);
    const auto isDoublePawnMove = (isPawnMove && absOffset == 30);
    
    // Define number of squares to check along the selected vector
    const int moveLen = mv.fromSq->getPiece()->getVectorLength() + isDoublePawnMove;
    
    auto foundToSquare = false;
    
    // Iterate through to ensure sliding pieces aren't being blocked
    for (int i = 1; i < moveLen; ++i) {
        const auto realIndex = getOffsetIndex(selectedOffset >> isDoublePawnMove, ZERO_LOCATION_1D, i);
        if (realIndex < 0 || realIndex >= OUTER_BOARD_SIZE * OUTER_BOARD_SIZE) {
            break;
        }
        const auto currSquare = board.vectorTable[realIndex].get();
        
        if (currSquare == mv.toSq) {
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
    if (isDoublePawnMove) {
        const auto distFromStartToCorner = board.getSquareIndex(mv.fromSq) - board.findCorner_1D();
        const auto whiteOffset = board.isWhiteTurn * 75;
        const auto rowPairs = std::make_pair(15 + whiteOffset, 29 + whiteOffset);
        if (distFromStartToCorner < rowPairs.first || distFromStartToCorner > rowPairs.second) {
            logMoveFailure(5, isSilent);
            return false;
        }
    }
    
    // Pawn related validation checks
    if (isPawnMove) {
        // Ensure pawns only move diagonally if they capture a piece, including en passant
        const auto distToCaptureSquare = board.getSquareIndex(mv.toSq) 
            - OUTER_BOARD_SIZE + ((mv.fromPieceColour == Colour::WHITE) * OUTER_BOARD_SIZE << 1);
        
        if (selectedOffset % OUTER_BOARD_SIZE) {
            if (!mv.toSq->getPiece() && !(board.enPassantActive && *mv.toSq == *board.enPassantTarget 
                        && board.vectorTable[distToCaptureSquare]->getPiece()
                        && board.vectorTable[distToCaptureSquare]->getPiece()->getColour() != mv.fromPieceColour)) {
                logMoveFailure(6, isSilent);
                return false;
            }
        } else if (mv.toSq->getPiece()) {
            // Prevent pawns from capturing vertically
            logMoveFailure(7, isSilent);
            return false;
        }
    }
    
    //Prevent players from placing their own king in check
    if (inCheck(mv)) {
        logMoveFailure(8, isSilent);
        return false;
    }
    
    if (mv.fromPieceType == PieceTypes::KING && absOffset == 2) {
        // Prevent king from jumping 2 spaces if not castling
        if (!getCastleDirectionBool(mv.fromPieceType, mv.fromPieceColour, selectedOffset)) {
            logMoveFailure(9, isSilent);
            return false;
        }
        //Prevent castling if king is currently in check
        if ((mv.fromPieceColour == Colour::BLACK && board.blackInCheck) 
                || (mv.fromPieceColour == Colour::WHITE && board.whiteInCheck)) {
            logMoveFailure(10, isSilent);
            return false;
        }
        const bool isKingSide = (selectedOffset > 0);
        const auto currIndex = board.getSquareIndex(mv.fromSq);

        const auto rookSquare = board.vectorTable[currIndex - 3 + (isKingSide * 3 << 1) - !isKingSide]->getPiece();
        if (!rookSquare || rookSquare->getType() != PieceTypes::ROOK 
                || rookSquare->getColour() != (board.isWhiteTurn ? Colour::WHITE : Colour::BLACK)) {
            logMoveFailure(11, isSilent); 
            return false;
        }

        for (int i = 1; i <= 2 + !isKingSide; ++i) {
            const auto index = -i + (isKingSide * i << 1);
            const auto currPiece = board.vectorTable[currIndex + index]->getPiece();
            const auto checkIndex = currIndex + index;
            if ((currPiece && currPiece->getType() != PieceTypes::ROOK) || (i <= 2 && inCheck(checkIndex))) {
                logMoveFailure(12, isSilent);
                return false;
            }
        }
    }
    return true;
}

bool Board::MoveGenerator::inCheck(const int squareIndex) const {
    assert(squareIndex >= 0 && squareIndex < TOTAL_BOARD_SIZE);
    
    static const auto& checkVectors{Piece(PieceTypes::UNKNOWN, Colour::UNKNOWN).getVectorList()};
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
    for (const auto offset : checkVectors) {
        const auto absOffset = std::abs(offset);
        // Change depth if current offset is a knight offset
        vectorLength = 7 - ((absOffset == 13 || absOffset > 16) * 6);
        
        for (int i = 1; i <= vectorLength; ++i) {
            const auto realIndex = getOffsetIndex(offset, squareIndex, i);
            if (realIndex < 0 || realIndex >= TOTAL_BOARD_SIZE) {
                break;
            }
            const auto currPiece = board.vectorTable[realIndex]->getPiece();
            if (currPiece) {
                if (currPiece->getColour() == friendlyPieceColour 
                        || currPiece->getColour() == Colour::UNKNOWN) {
                    break;
                }
                if (i > currPiece->getVectorLength() - 1) {
                    break;
                }
                if (currPiece->getType() == PieceTypes::PAWN && (offset % OUTER_BOARD_SIZE) == 0) {
                    break;
                }
                const auto& pieceVector = currPiece->getVectorList();
                bool offsetFound = false;
                for (size_t j = 0; j < pieceVector.size(); ++j) {
                    if (pieceVector[j] == -offset) {
                        offsetFound = true;
                        break;
                    }
                }
                if (!offsetFound) {
                    break;
                }
                return true;
            }
        }
    }
    return false;
}

bool Board::MoveGenerator::inCheck(const Move& mv) const {
    static const auto& checkVectors = Piece(PieceTypes::UNKNOWN, Colour::UNKNOWN).getVectorList();
    std::unique_ptr<Piece> toPieceCopy{mv.toSq->releasePiece()};
    bool enPassantCaptureMade = false;
    int captureIndex = -1;
    std::unique_ptr<Piece> enPassantPiece{nullptr};

    if (mv.enPassantActive && mv.fromPieceType == PieceTypes::PAWN && mv.toSq == mv.enPassantTarget
            && ((mv.toSq->getOffset() - mv.fromSq->getOffset()) % OUTER_BOARD_SIZE)) {
        enPassantCaptureMade = true;
        captureIndex = board.getSquareIndex(mv.toSq) - OUTER_BOARD_SIZE + ((board.isWhiteTurn) * OUTER_BOARD_SIZE << 1);
        enPassantPiece.reset(board.vectorTable[captureIndex]->releasePiece());
    }

    std::swap(*mv.fromSq, *mv.toSq);

    //Check if friendly king can be found
    assert(std::find_if(board.vectorTable.cbegin(), board.vectorTable.cend(), 
            [&](const auto& sq){
                const auto& piece = sq->getPiece();
                return piece && piece->getType() == PieceTypes::KING 
                    && piece->getColour() == mv.fromPieceColour;
            }) != board.vectorTable.cend());
            
    int squareIndex = -1;
    for (size_t i = 0; i < TOTAL_BOARD_SIZE; ++i) {
        const auto& piece = board.vectorTable[i]->getPiece();
        if (piece && piece->getType() == PieceTypes::KING && piece->getColour() == mv.fromPieceColour) {
            squareIndex = i;
            break;
        }
    }
    //Ensure index was found during previous loop
    assert(squareIndex != -1);
    
    int vectorLength;
    for (const auto offset : checkVectors) {
        const auto absOffset = std::abs(offset);
        // Change depth if current offset is a knight offset
        vectorLength = 7 - ((absOffset == 13 || absOffset > 16) * 6);
        
        for (int i = 1; i <= vectorLength; ++i) {
            const auto realIndex = getOffsetIndex(offset, squareIndex, i);
            if (realIndex < 0 || realIndex >= TOTAL_BOARD_SIZE) {
                break;
            }
            const auto currPiece = board.vectorTable[realIndex]->getPiece();
            if (currPiece) {
                if (currPiece->getColour() == mv.fromPieceColour 
                        || currPiece->getColour() == Colour::UNKNOWN) {
                    break;
                }

                //Prevent single space mover pieces from being treated as sliding pieces
                if (i > currPiece->getVectorLength() - 1) {
                    break;
                }
                
                bool found = false;
                for (const auto off : currPiece->getVectorList()) {
                    if (currPiece->getType() == PieceTypes::PAWN) {
                        if (off % OUTER_BOARD_SIZE && off == -offset) {
                            found = true;
                            break;
                        }
                    } else if (off == -offset) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    break;
                }
                std::swap(*mv.fromSq, *mv.toSq);
                if (mv.captureMade) {
                    mv.toSq->setPiece(std::move(toPieceCopy));
                }
                if (enPassantCaptureMade) {
                    board.vectorTable[captureIndex]->setPiece(std::move(enPassantPiece));
                }
                return true;
            }
        }
    }
    std::swap(*mv.fromSq, *mv.toSq);
    if (mv.captureMade) {
        mv.toSq->setPiece(std::move(toPieceCopy));
    }
    if (enPassantCaptureMade) {
        board.vectorTable[captureIndex]->setPiece(std::move(enPassantPiece));
    }
    return false;
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
#ifndef NDEBUG
    std::cout << "Move is not legal " << failureNum << '\n';
#else
    std::cout << "Move is not legal\n";
#endif
}

std::vector<Move> Board::MoveGenerator::generateAll() {
    std::vector<Move> moveList;
    moveList.reserve(100); //More moves are possible, but this shoud cover 99% of positions without reallocating
        
    const auto currentPlayerColour = (board.isWhiteTurn) ? Colour::WHITE : Colour::BLACK;
    const auto& startCoords = board.findCorner();
    
    std::vector<std::tuple<int, int, Piece*>> pieceCoords;
    pieceCoords.reserve(16); //Cannot have more than 16 pieces for one player
    for (int i = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0; j < INNER_BOARD_SIZE; ++j) {
            const auto sq = board.vectorTable[((i + startCoords.first) * OUTER_BOARD_SIZE) + j + startCoords.second].get();
            const auto fromPiece = sq->getPiece();
            if (fromPiece && fromPiece->getType() != PieceTypes::UNKNOWN 
                    && fromPiece->getColour() == currentPlayerColour) {
                pieceCoords.emplace_back(std::forward_as_tuple(i, j, fromPiece));
            }
        }
    }
    
    Move mv;
    for (const auto& p : pieceCoords) {
        board.shiftBoard(std::get<1>(p), std::get<0>(p));
        for (const auto offset : std::get<2>(p)->getVectorList()) {
             //Define number of squares to check along the selected vector
            const auto moveLen = std::get<2>(p)->getVectorLength();
            
            for (int j = 1; j < moveLen; ++j) {
                const auto toSquareIndex = getOffsetIndex(offset, ZERO_LOCATION_1D, j);
                if (toSquareIndex < 0 || toSquareIndex > TOTAL_BOARD_SIZE) {
                    break;
                }
                mv.fromSq = board.vectorTable[ZERO_LOCATION_1D].get();
                mv.toSq = board.vectorTable[toSquareIndex].get();

                if (mv.toSq->checkSentinel()) {
                    break;
                }
                mv.fromPieceType = mv.fromSq->getPiece()->getType();
                mv.fromPieceColour = mv.fromSq->getPiece()->getColour();
                
                if (mv.toSq->getPiece()) {
                    mv.toPieceType = mv.toSq->getPiece()->getType();
                    mv.toPieceColour = mv.toSq->getPiece()->getColour();
                    mv.captureMade = true;
                    if (mv.fromPieceColour == mv.toPieceColour) {
                        break;
                    }
                } else {
                    mv.toPieceType = PieceTypes::UNKNOWN;
                    mv.toPieceColour = Colour::UNKNOWN;
                    mv.captureMade = false;
                }

                mv.promotionType = (mv.fromSq->getPiece()) ? mv.fromPieceType : PieceTypes::UNKNOWN;
                mv.promotionMade = false;
                mv.halfMoveClock = board.halfMoveClock;
                mv.moveCounter = board.moveCounter;
                mv.castleRights = board.castleRights;
                mv.isCastle = false; //Let the move make function set this to its true value
                
                mv.enPassantActive = board.enPassantActive;
                mv.enPassantTarget = board.enPassantTarget;

                //Promotion check
                if (mv.promotionType == PieceTypes::PAWN) {
                    const auto distToEndSquare = board.getSquareIndex(mv.toSq);
                            
                    //Calculate the corner index based on the above board shift to prevent a linear search
                    const auto cornerIndex = ((7 - std::get<0>(p)) * OUTER_BOARD_SIZE) + (7 - std::get<1>(p));
                    const auto distFromStartToCorner = distToEndSquare - cornerIndex;
                    const auto& rowPairs = (mv.fromPieceColour == Colour::WHITE) 
                        ? std::make_pair(0, 14) : std::make_pair(105, 119);
                    
                    //Promotion validity
                    if (distFromStartToCorner >= rowPairs.first && distFromStartToCorner <= rowPairs.second) {
                        if (!validateMove(mv, true)) {
                            continue;
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
                if (!validateMove(mv, true)) {
                    continue;
                }
                moveList.push_back(std::move(mv));
            }
        }
    }
    return moveList;
}

int Board::MoveGenerator::getMoveOffset(const Move& mv) const {
    assert(mv.fromSq && mv.toSq);
    assert(mv.fromSq->getPiece());
    const auto& vectorOffsets = mv.fromSq->getPiece()->getVectorList();
    const auto vectorLength = mv.fromSq->getPiece()->getVectorLength();
    const auto diff = mv.toSq->getOffset() - mv.fromSq->getOffset();
    
    // Find the offset that the move uses
    for (const auto offset : vectorOffsets) {
        if ((diff / offset) > 0 && (diff / offset) < vectorLength && !(diff % offset)) {
            return offset;
        }
    }
    return -100;
}
