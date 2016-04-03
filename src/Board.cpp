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

/*
 * Try to remove index from squares and instead calculate it based off the index
 * This will ensure that the offsets aren't rotated during board movement
 * 
 * Offset = 98 - (15 * i) + (j + 1)
 * IMPORTANT:
 * Positive x: (30 * ((x + 7) / 15)) - x
 * Negative x: -((30 * ((abs(x) + 7) / 15)) - abs(x))
 * 
 * Also, might have to replace std rotate in my board hifting methods to ensure
 * things all fit together nicely.
 * 
 * TODO: Add check to allow double pawn movement when selecting offset
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

const std::pair<int, int> Board::findCorner() const {
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
    return std::make_pair(-1, -1);
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
    
    if (!moveGen->validateMove(mv)) {
        return;
    }
    const auto diff = mv.toSq->getOffset() - mv.fromSq->getOffset();
    
    if (!ensureEnPassantValid()) {
        return;
    }
    
    const auto& fromPieceType = mv.fromSq->getPiece()->getType();
    
    if (enPassantActive && fromPieceType == PieceTypes::PAWN 
            && (diff % 15) && *mv.toSq == *enPassantTarget) {
        const auto distToFromSquare = std::distance(vectorTable.cbegin(), 
            std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
                [&mv](const auto& sq){return (*sq == *mv.fromSq);}));
        vectorTable[distToFromSquare + (diff % 15)]->setPiece(nullptr);
    }
    
    enPassantActive = false;
    enPassantTarget = nullptr;
    
    // Add en Passant target if pawn double move was made.
    if (std::abs(diff) == 30 && fromPieceType == PieceTypes::PAWN) {
        const auto distToEndSquare = std::distance(vectorTable.cbegin(), 
            std::find_if(vectorTable.cbegin(), vectorTable.cend(), 
                [&mv](const auto& sq){return (*sq == *mv.toSq);}));
        enPassantActive = true;
        enPassantTarget = vectorTable[distToEndSquare + (diff >> 1)].get();
    }
    
    // If moving to an occupied square, capture the piece
    if (mv.toSq->getPiece()) {
        mv.toSq->setPiece(nullptr);
    }
    std::swap(*mv.fromSq, *mv.toSq);
    swapOffsets(mv);
    isWhiteTurn = !isWhiteTurn;
}

bool Board::ensureEnPassantValid() const {
    try {
        if (enPassantActive && !enPassantTarget) {
            throw std::logic_error("En passant target should never be null when en passant is active");
        }
    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return false;
    }
    return true;
}
