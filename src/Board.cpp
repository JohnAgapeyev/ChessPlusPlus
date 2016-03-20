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
    if (startCoords.second + count < 0 || startCoords.second + count + INNER_BOARD_SIZE - 1 > OUTER_BOARD_SIZE) {
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
    std::swap(*mv.fromSq, *mv.toSq);
    auto temp = mv.fromSq->getOffset();
    mv.fromSq->setOffset(mv.toSq->getOffset());
    mv.toSq->setOffset(temp);
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
    auto vectorOffsets = fromPiece->getVectorList();
    auto diff = (*secondSquare)->getOffset() - (*firstSquare)->getOffset();
    
    auto selectedOffset = std::find_if(vectorOffsets.cbegin(), 
        vectorOffsets.cend(), [diff](auto offset){
            return (diff / offset > 0 && diff / offset < 8 && !(diff % offset));
        }
    );
    
    // Check if the move offset is a legal one
    if (selectedOffset == vectorOffsets.cend()) {
#ifdef DEBUG
        std::cout << "Move is not legal1" << std::endl;
#else
        std::cout << "Move is not legal" << std::endl;
#endif
        return false;
    }
    
    /*
     * Check if the colour of the piece on the starting square 
     * is the same colour as the piece on the ending square.
     */
    if (mv.toSq->getPiece() && fromPiece->getColour() == mv.toSq->getPiece()->getColour()) {
#ifdef DEBUG
        std::cout << "Move is not legal2" << std::endl;
#else
        std::cout << "Move is not legal" << std::endl;
#endif
        return false;
    }
    
    int moveLen;
    switch (fromPiece->getType()) {
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
    const auto verticalDisplacement = (OUTER_BOARD_SIZE 
        * ((*selectedOffset) / OUTER_BOARD_SIZE));
    
    // Iterate through to ensure sliding pieces aren't being blocked
    for (int i = 1; i < moveLen; ++i) {
        currSquare = board.vectorTable[ZERO_LOCATION_1D 
                        - (i * (verticalDisplacement 
                        - ((*selectedOffset) - verticalDisplacement))
                    )];
        if (currSquare->getOffset() == mv.toSq->getOffset()) {
            foundToSquare = true;
            break;
        }
        // Check if the square has a piece on it or is a sentinel
        if (currSquare->getPiece()) {
#ifdef DEBUG
        std::cout << "Move is not legal3" << std::endl;
#else
        std::cout << "Move is not legal" << std::endl;
#endif
            return false;
        }
    }
    
    // Check if square was found in previous loop
    if (!foundToSquare) {
#ifdef DEBUG
        std::cout << "Move is not legal4" << std::endl;
#else
        std::cout << "Move is not legal" << std::endl;
#endif
        return false;
    }
    
    auto secondSquareIndex = std::distance(board.vectorTable.cbegin(), secondSquare);
    
    /* 
     * Ensure pawns only move diagonally if they capture a piece, including en passant
     * En passant not implemented, but when it is, replace false with 
     * the check of the neighbouring squares
     */
    if (fromPiece->getType() == PieceTypes::PAWN 
            && std::abs(*selectedOffset) != 15 
            && ((!board.vectorTable[secondSquareIndex]->getPiece() 
                    || board.vectorTable[secondSquareIndex]->checkSentinel()) 
                || false)) {
#ifdef DEBUG
        std::cout << "Move is not legal5" << std::endl;
#else
        std::cout << "Move is not legal" << std::endl;
#endif
        return false;
    }
    
    // call inCheck to ensure the board state doesn't leave the same coloured king in check
    
    return true;
}
