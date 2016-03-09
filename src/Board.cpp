#include "headers/square.h"
#include "headers/board.h"
#include "headers/consts.h"
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

void Board::setVector() {
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

auto Board::findCorner() {
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
    for (int i = 0, col = 0; i < INNER_BOARD_SIZE; ++i) {
        for (int j = 0, row = 0; j < INNER_BOARD_SIZE; ++j) {
            col = (count > 0) ? INNER_BOARD_SIZE - 1 - i : i;
            row = (count > 0) ? INNER_BOARD_SIZE - 1 - j : j;
            
            std::swap(
                vectorTable[
                (startCoords.first * OUTER_BOARD_SIZE) + startCoords.second + (col * OUTER_BOARD_SIZE) + row]
                ,
                vectorTable[
                count + (startCoords.first * OUTER_BOARD_SIZE) + startCoords.second + (col * OUTER_BOARD_SIZE) + row]
            );
        }
    }
}

void Board::shiftVertical(int count) {
    if (!count) {
        return;
    }
    auto startCoords = findCorner();
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

void Board::shiftBoard(int col, int row) {
    auto startCoords = findCorner();
    auto colDiff = ZERO_LOCATION.first - (startCoords.first + col);
    auto rowDiff = ZERO_LOCATION.second - (startCoords.second + row);
    shiftHorizontal(colDiff);
    shiftVertical(rowDiff);
}
