#include "headers/square.h"
#include "headers/board.h"
#include "headers/consts.h"
#include "headers/enums.h"
#include "headers/move.h"
#include <vector>
#include <array>
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <stdexcept>

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
    const auto& topLeftCorner = board.findCorner();
    result.fromSq = board.vectorTable[((INNER_BOARD_SIZE - 1 - input[1]) * OUTER_BOARD_SIZE) 
            + (topLeftCorner.first * OUTER_BOARD_SIZE) + topLeftCorner.second + input[0]].get();
    result.toSq = board.vectorTable[((INNER_BOARD_SIZE - 1 - input[3]) * OUTER_BOARD_SIZE) 
            + (topLeftCorner.first * OUTER_BOARD_SIZE) + topLeftCorner.second + input[2]].get();
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
bool Board::MoveGenerator::validateMove(const Move& mv) {
    const auto& firstSquare = std::find_if(board.vectorTable.cbegin(), 
            board.vectorTable.cend(), [&mv](const auto& sq){
        return (sq->getOffset() == mv.fromSq->getOffset());
        });
    const auto& secondSquare = std::find_if(board.vectorTable.cbegin(), 
            board.vectorTable.cend(), [&mv](const auto& sq){
        return (sq->getOffset() == mv.toSq->getOffset());
        });
    // Try to find the start and end points
    if (firstSquare == board.vectorTable.cend() || secondSquare == board.vectorTable.cend()) {
        std::cerr << "Could not find start or end squares" << std::endl;
        return false;
    }
    
    // Check for either square being a sentinel
    if ((*firstSquare)->checkSentinel() || (*secondSquare)->checkSentinel()) {
        std::cerr << "You somehow referenced a sentinel square for your move. GJ\n";
        return false;
    }
    const auto& fromPiece = mv.fromSq->getPiece();
    
    if (!fromPiece || (fromPiece->getType() == PieceTypes::UNKNOWN 
                  && fromPiece->getColour() == Colour::UNKNOWN)) {
        std::cout << "Cannot start a move on an empty square" << std::endl;
        return false;
    }
    
    const auto& fromPieceType = fromPiece->getType();
    const auto& fromPieceColour = fromPiece->getColour();
    const auto& toPiece = mv.toSq->getPiece();
    
    // Check if piece being moved matches the current player's colour
    if ((fromPieceColour == Colour::WHITE && !board.isWhiteTurn) 
            || (fromPieceColour == Colour::BLACK && board.isWhiteTurn)) {
        if (board.isWhiteTurn) {
            std::cout << "Cannot move black piece on white's turn\n";
        } else {
            std::cout << "Cannot move white piece on black's turn\n";
        }
        return false;
    }
    
    const auto& vectorOffsets = fromPiece->getVectorList();
    const auto diff = (*secondSquare)->getOffset() - (*firstSquare)->getOffset();
    const auto& secondSquareIndex = std::distance(board.vectorTable.cbegin(), secondSquare);
    
    // Find the offset that the move uses
    const auto& selectedOffset = std::find_if(vectorOffsets.cbegin(), 
        vectorOffsets.cend(), [diff](auto offset){
            return (diff / offset > 0 && diff / offset < 8 && !(diff % offset));
        }
    );
    
    // Check if the move offset was found
    if (selectedOffset == vectorOffsets.cend()) {
#ifdef DEBUG
        std::cout << "Move is not legal1\n";
#else
        std::cout << "Move is not legal\n";
#endif
        return false;
    }
    
    /*
     * Check if the colour of the piece on the starting square 
     * is the same colour as the piece on the ending square.
     */
    if (toPiece && fromPieceColour == toPiece->getColour()) {
#ifdef DEBUG
        std::cout << "Move is not legal2\n";
#else
        std::cout << "Move is not legal\n";
#endif
        return false;
    }
    
    // Define number of squares to check along the selected vector
    int moveLen;
    switch (fromPieceType) {
        case PieceTypes::KING:
        case PieceTypes::PAWN:
        case PieceTypes::KNIGHT:
            moveLen = 2;
            break;
        default:
            moveLen = INNER_BOARD_SIZE;
            break;
    }
    
    auto currSquare = board.vectorTable[0];
    auto foundToSquare = false;
    
    // Iterate through to ensure sliding pieces aren't being blocked
    for (int i = 1; i < moveLen; ++i) {
        if (*selectedOffset > 0) {
            currSquare = board.vectorTable[ZERO_LOCATION_1D 
                - (i * (((OUTER_BOARD_SIZE * 2) * ((*selectedOffset + INNER_BOARD_SIZE - 1) 
                / OUTER_BOARD_SIZE)) - *selectedOffset))];
        } else {
            currSquare = board.vectorTable[ZERO_LOCATION_1D 
                + (i * (((OUTER_BOARD_SIZE * 2) * ((std::abs(*selectedOffset) + INNER_BOARD_SIZE - 1) 
                / OUTER_BOARD_SIZE)) - std::abs(*selectedOffset)))];
        }
        if (currSquare->getOffset() == mv.toSq->getOffset()) {
            foundToSquare = true;
            break;
        }
        // Check if the square has a piece on it or is a sentinel
        if (currSquare->getPiece()) {
#ifdef DEBUG
        std::cout << "Move is not legal3\n";
#else
        std::cout << "Move is not legal\n";
#endif
            return false;
        }
    }
    
    // Check if square was found in previous loop
    if (!foundToSquare) {
#ifdef DEBUG
        std::cout << "Move is not legal4\n";
#else
        std::cout << "Move is not legal\n";
#endif
        return false;
    }
    
    // Check that pawns don't double move except on their starting rows
    if (*selectedOffset == 30) {
        const auto& startCoords = board.findCorner();
        const auto& dist = std::distance(board.vectorTable.cbegin(), firstSquare);
        const auto cornerIndex = (startCoords.first * OUTER_BOARD_SIZE) + startCoords.second;
        const auto& rowPairs = (fromPieceColour == Colour::WHITE) ? std::make_pair(90, 104) : std::make_pair(15, 29);
        const auto distFromStartToCorner = dist - cornerIndex;
        if (distFromStartToCorner < rowPairs.first || distFromStartToCorner > rowPairs.second) {
#ifdef DEBUG
            std::cout << "Move is not legal5\n";
#else
            std::cout << "Move is not legal\n";
#endif
            return false;
        }
    }
    
    if (!board.ensureEnPassantValid()) {
        return false;
    }
    
    // Pawn related validation checks
    if (fromPieceType == PieceTypes::PAWN) {
        // Ensure pawns only move diagonally if they capture a piece, including en passant
        if ((*selectedOffset % 15) 
                && !board.vectorTable[secondSquareIndex]->getPiece() 
                && !board.enPassantActive) {
    #ifdef DEBUG
            std::cout << "Move is not legal6\n";
    #else
            std::cout << "Move is not legal\n";
    #endif
            return false;
        }
        // Prevent pawns from capturing vertically
        if (!(*selectedOffset % 15) && board.vectorTable[secondSquareIndex]->getPiece()) {
    #ifdef DEBUG
            std::cout << "Move is not legal7\n";
    #else
            std::cout << "Move is not legal\n";
    #endif
            return false;
        }
    }
    
    // call inCheck to ensure the board state doesn't leave the same coloured king in check
    
    return true;
}
