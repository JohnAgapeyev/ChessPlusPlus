#include "headers/square.h"
#include "headers/board.h"
#include "headers/consts.h"
#include "headers/movegenerator.h"
#include "headers/enums.h"
#include "headers/move.h"
#include <vector>
#include <array>
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <iterator>

/*
 * Try to remove index from squares and instead calculate it based off the index
 * This will ensure that the offsets aren't rotated during board movement
 * 
 * Offset = 98 - (15 * i) + (j + 1)
 * 
 * 
 * Also, might have to replace std rotate in my board hifting methods to ensure
 * things all fit together nicely.
 * 
 */
 
constexpr auto genOffset = [=](auto a, auto b){return 98 - (15 * a) + b;};

Board::Board() : moveGen(*this) {
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
}

auto Board::findCorner() const {
    auto result = std::find_if(
    vectorTable.cbegin(), vectorTable.cend(), 
    [](auto sq) {
        auto pc = sq->getPiece();
        return (pc && pc->getColour() != Colour::UNKNOWN && pc->getType() != PieceTypes::UNKNOWN);
    });
    
    if (result != vectorTable.cend()) {
        auto dist = std::distance(vectorTable.cbegin(), result);
        return std::make_pair(dist / OUTER_BOARD_SIZE, dist % OUTER_BOARD_SIZE);
    }
    return std::make_pair(-1L, -1L);
}

void Board::shiftHorizontal(const int count) {
    if (!count) {
        return;
    }
    auto startCoords = findCorner();
    //if (startCoords.second + count < 0 || startCoords.second + count + INNER_BOARD_SIZE - 1 > OUTER_BOARD_SIZE) {
        //std::cerr << "Invalid board shift; No movement performed" << std::endl;
        //return;
    //}
    for (int i = 0, col = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0, row = 0; j < INNER_BOARD_SIZE; ++j) {
            col = (count > 0) ? INNER_BOARD_SIZE - 1 - i : i;
            row = (count > 0) ? INNER_BOARD_SIZE - 1 - j : j;
            
            std::swap(
                vectorTable[
                (startCoords.first * OUTER_BOARD_SIZE) 
                    + startCoords.second + (col * OUTER_BOARD_SIZE) + row],
                vectorTable[
                count + (startCoords.first * OUTER_BOARD_SIZE) 
                    + startCoords.second + (col * OUTER_BOARD_SIZE) + row]
            );
        }
    }
}

void Board::shiftVertical(int count) {
    if (!count) {
        return;
    }
    auto startCoords = findCorner();
    //if (startCoords.first + count < 0 || startCoords.first + count + INNER_BOARD_SIZE - 1 > OUTER_BOARD_SIZE) {
        //std::cerr << "Invalid board shift; No movement performed" << std::endl;
        //return;
    //}
    for (int i = 0, col = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0, row = 0; j < INNER_BOARD_SIZE; ++j) {
            col = (count > 0) ? INNER_BOARD_SIZE - 1 - i : i;
            row = (count > 0) ? INNER_BOARD_SIZE - 1 - j : j;
            
            std::swap(
                vectorTable[
                (startCoords.first * OUTER_BOARD_SIZE) + startCoords.second + (col * OUTER_BOARD_SIZE) + row]
                ,
                vectorTable[
                (count * OUTER_BOARD_SIZE) + (startCoords.first * OUTER_BOARD_SIZE) + startCoords.second + (col * OUTER_BOARD_SIZE) + row]
            );
        }
    }
}

/*
 * TODO: Board is being shifted from its current position rather than 0,0
 * Aka, calling shiftBoard(1,1) on a move board will move it up and right by 1
 * rather than shifting it so that 1,1 is at offset 0
 */
void Board::shiftBoard(int col, int row) {
    auto startCoords = findCorner();
    auto colDiff = ZERO_LOCATION.first - (startCoords.second + col);
    auto rowDiff = ZERO_LOCATION.second - (startCoords.first + row);
    shiftHorizontal(colDiff);
    std::cout << colDiff << ", " << rowDiff << std::endl;
    shiftVertical(rowDiff);
    for (int i = 0; i < OUTER_BOARD_SIZE; ++i) {
        for (int j = 0; j < OUTER_BOARD_SIZE; ++j) {
            vectorTable[(i* OUTER_BOARD_SIZE) + j]->setOffset(genOffset(i, j));
        }
    }
}

void Board::makeMove(std::string input) {
    Move mv = moveGen.createMove(input);
    
    // If the characters are letters, convert them to digits
    if (input[0] > 8 && input[2] > 8) {
        input[0] -= 49;
        input[2] -= 49;
    }
    // Change input to zero-indexed value
    input[1] -= 1;
    input[3] -= 1;
    
    shiftBoard((input[0] - '0'), INNER_BOARD_SIZE - 1 - (input[1] - '0'));
    
    if (!moveGen.validateMove(mv)) {
        return;
    }
    if (mv.toSq->getPiece()) {
        mv.toSq->setPiece(nullptr);
    }
    std::swap(mv.fromSq, mv.toSq);
}

Move Board::MoveGenerator::createMove(std::string input) {
    // If the characters are letters, convert them to digits
    if (input[0] > 8 && input[2] > 8) {
        input[0] -= 49;
        input[2] -= 49;
    }
    // Change input to zero-indexed value
    input[1] -= 1;
    input[3] -= 1;
    Move result;
    auto topLeftCorner = board.findCorner();
    result.fromSq = board.vectorTable[((INNER_BOARD_SIZE - 1 - (input[1] - '0')) * OUTER_BOARD_SIZE) 
            + (topLeftCorner.first * OUTER_BOARD_SIZE) 
            + input[0] - '0' + topLeftCorner.second];
    result.toSq = board.vectorTable[((INNER_BOARD_SIZE - 1 - (input[3] - '0')) * OUTER_BOARD_SIZE) 
            + (topLeftCorner.first * OUTER_BOARD_SIZE) + input[2] - '0' 
            + topLeftCorner.second];
    return result;
}

/*
 * TODO: Need to shift squares according to 8x8 board, not the default 15x15
 * AKA a1 should always be valid square, not a sentinel if the board hasn't shifted
 */
bool Board::MoveGenerator::validateMove(Move mv) {
    auto firstSquare = std::find(board.vectorTable.cbegin(), board.vectorTable.cend(), mv.fromSq);
    auto secondSquare = std::find(board.vectorTable.cbegin(), board.vectorTable.cend(), mv.toSq);
    // Try to find the start and end points
    if (firstSquare == board.vectorTable.cend() || secondSquare == board.vectorTable.cend()) {
        std::cerr << "Could not find start or end squares" << std::endl;
        return false;
    }
    auto fromPiece = mv.fromSq->getPiece();
    if (!fromPiece || (fromPiece->getType() == PieceTypes::UNKNOWN 
                  && fromPiece->getColour() == Colour::UNKNOWN)) {
        std::cout << "Cannot start a move on an empty square" << std::endl;
        return false;
    }
    auto firstIndex = std::distance(board.vectorTable.cbegin(), firstSquare);
    auto secondIndex = std::distance(board.vectorTable.cbegin(), secondSquare);
    auto vectorOffsets = fromPiece->getVectorList();
    auto vectorStart = vectorOffsets.cbegin();
    auto vectorEnd = vectorOffsets.cend();
    auto diff = firstIndex - secondIndex;
    // Check if the move offset is a legal one
    if (!std::binary_search(vectorStart, vectorEnd, diff)) {
        std::cout << "Move is not legal1" << std::endl;
        return false;
    }
    /*
     * Check if the colour of the piece on the starting square 
     * is the same colour as the piece on the ending square.
     */
    if (mv.toSq->getPiece() && fromPiece->getColour() == mv.toSq->getPiece()->getColour()) {
        std::cout << "Move is not legal2" << std::endl;
        return false;
    }
    auto currSquare = board.vectorTable[0];
    // Iterate through to ensure sliding pieces aren't being blocked
    for (int i = 1; i < INNER_BOARD_SIZE; ++i) {
        currSquare = board.vectorTable[(ZERO_LOCATION.first * OUTER_BOARD_SIZE) + ZERO_LOCATION.second - (i * diff)];
        if (*currSquare == *(mv.toSq)) {
            break;
        }
        // Check if the square has a piece on it or is a sentinel
        if (currSquare->getPiece()) {
            std::cout << "Move is not legal3" << std::endl;
            return false;
        }
    }
    
    // ensure pawns only move diagonally if they capture a piece, including en passant
    
    // call inCheck to ensure the board state doesn't leave the same coloured king in check
    
    return true;
}
