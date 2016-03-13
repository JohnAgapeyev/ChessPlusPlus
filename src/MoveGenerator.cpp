#include "headers/movegenerator.h"
#include "headers/enums.h"
#include "headers/consts.h"
#include "headers/move.h"
#include "headers/square.h"
#include <vector>
#include <array>
#include <algorithm>
#include <iterator>

Move MoveGenerator::createMove(std::string input) {
    if ((int) input[0] > 8 && (int) input[2] > 8) {
        input[0] = (int) input[0] - 96;
        input[2] = (int) input[2] - 96;
    }
    Move result;
    result.fromSq = board[(input[0] * OUTER_BOARD_SIZE) + input[1]];
    result.toSq = board[(input[2] * OUTER_BOARD_SIZE) + input[3]];
    return result;
}

/*
 * TODO: Need to shift squares according to 8x8 board, not the default 15x15
 * AKA a1 should always be valid square, not a sentinel if the board hasn't shifted
 */
bool MoveGenerator::validateMove(Move mv) {
    auto firstSquare = std::find(board.cbegin(), board.cend(), mv.fromSq);
    auto secondSquare = std::find(board.cbegin(), board.cend(), mv.toSq);
    // Try to find the start and end points
    if (firstSquare == board.cend() || secondSquare == board.cend()) {
        std::cerr << "Could not find start or end squares" << std::endl;
        return false;
    }
    auto fromPiece = mv.fromSq->getPiece();
    if (fromPiece || (fromPiece->getType() == PieceTypes::UNKNOWN 
                  && fromPiece->getColour() == Colour::UNKNOWN)) {
        std::cout << "Cannot start a move on an empty square" << std::endl;
        return false;
    }
    auto firstIndex = std::distance(board.cbegin(), firstSquare);
    auto secondIndex = std::distance(board.cbegin(), secondSquare);
    auto vectorOffsets = fromPiece->getVectorList();
    auto vectorStart = vectorOffsets.cbegin();
    auto vectorEnd = vectorOffsets.cend();
    auto diff = secondIndex - firstIndex;
    // Check if the move offset is a legal one
    if (!std::binary_search(vectorStart, vectorEnd, diff)) {
        std::cout << "Move is not legal" << std::endl;
        return false;
    }
    /*
     * Check if the colour of the piece on the starting square 
     * is the same colour as the piece on the ending square.
     */
    if (fromPiece && fromPiece->getColour() == mv.toSq->getPiece()->getColour()) {
        std::cout << "Move is not legal" << std::endl;
        return false;
    }
    auto currSquare = board[0].get();
    // Iterate through to ensure sliding pieces aren't being blocked
    for (int i = 1; i < INNER_BOARD_SIZE; ++i) {
        currSquare = board[(ZERO_LOCATION.first * OUTER_BOARD_SIZE) + ZERO_LOCATION.second + (i * diff)].get();
        if (currSquare == mv.toSq.get()) {
            break;
        }
        // Check if the square has a piece on it or is a sentinel
        if (currSquare->getPiece() 
                || (currSquare->getPiece()->getType() == PieceTypes::UNKNOWN 
                && currSquare->getPiece()->getColour() == Colour::UNKNOWN)) {
            std::cout << "Move is not legal" << std::endl;
            return false;
        }
    }
    
    // ensure pawns only move diagonally if they capture a piece, including en passant
    
    // call inCheck to ensure the board state doesn't leave the same coloured king in check
    
    
    
    return true;
}
