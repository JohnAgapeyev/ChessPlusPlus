#include "headers/square.h"
#include "headers/board.h"
#include "headers/consts.h"
#include <vector>
#include <array>
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>

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
                    vectorTable[i][j] = INIT_BOARD[i][j];
                    vectorTable[i][j]->setOffset(genOffset(i, j));
                } else {
                    vectorTable[i][j] = std::make_shared<Square>(genOffset(i, j));
                }
            } else {
                vectorTable[i][j] = std::make_shared<Square>(genOffset(i, j));
            }
        }
    }
}

auto Board::findCorner() {
    auto colOffset = -1;
    auto rowOffset = -1;
    
    auto findValid = [](auto sq) {
        auto pc = sq->getPiece();
        return (pc && pc->getColour() != Colour::UNKNOWN && pc->getType() != PieceTypes::UNKNOWN);
    };
        
    for (int i = 0; i < 15; ++i) {
        if (colOffset != -1 && rowOffset != -1) {
            break;
        }
        for (int j = 0; j < 15; ++j) {
            if (findValid((vectorTable[i][j]).get())) {
                colOffset = i;
                rowOffset = j;
                break;
            }
        }
    }
    if (colOffset != -1 && rowOffset != -1) {
        return std::make_pair(colOffset, rowOffset);
    }
    return std::make_pair(-1, -1);
}

void Board::shiftHorizontal(int count) {
    auto it = vectorTable[0].begin();
    for (auto& row : vectorTable) {
        it = (count > 0) ? row.begin() : row.end();
        std::rotate(row.begin(), it + count, row.end());
    }
}

void Board::shiftVertical(int count) {
    auto it = (count > 0) ? vectorTable.begin() : vectorTable.end();
    std::rotate(vectorTable.begin(), it + count, vectorTable.end());
}

void Board::shiftBoard(int col, int row) {
    auto startCoords = findCorner();
    auto colDiff = startCoords.first + col - ZERO_LOCATION.first;
    auto rowDiff = startCoords.second + row - ZERO_LOCATION.second;
    shiftHorizontal(colDiff);
    shiftVertical(rowDiff);
}
