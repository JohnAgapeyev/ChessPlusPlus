#include <algorithm>
#include <utility>
#include <functional>
#include "headers/hash.h"
#include "headers/board.h"

size_t std::hash<Board>::operator() (const Board& b) const {
    if (b.currHash) {
        return b.currHash;
    }
    
    const auto& cornerCoords = const_cast<Board&>(b).findCorner();
    const int cornerIndex = (cornerCoords.first * OUTER_BOARD_SIZE) + cornerCoords.second;
    const auto& cornerPiece = b.vectorTable[cornerIndex]->getPiece();
    size_t newHash = 0;
    if (cornerPiece) {
        newHash = HASH_VALUES[pieceLookupTable[cornerPiece->getType()] 
            + ((cornerPiece->getColour() == Colour::WHITE) ? 0: 6)];
    }
    
    for (int i = cornerCoords.first; i < cornerCoords.first + INNER_BOARD_SIZE; ++i) {
        for (int j = cornerCoords.second; j < cornerCoords.second + INNER_BOARD_SIZE; ++j) {
            if (i == cornerCoords.first && j == cornerCoords.second) {
                continue;
            }
            //Black is (white hash + 6) for equivalent piece types
            if (b.vectorTable[(i * OUTER_BOARD_SIZE) + j]->getPiece()) {
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
    
    newHash ^= HASH_VALUES[static_cast<int>(SquareState::CASTLE_RIGHTS) + b.castleRights];
    
    if (b.enPassantActive) {
         //Calculate en passant file
        const auto targetIndex = std::distance(b.vectorTable.cbegin(), 
            std::find_if(b.vectorTable.cbegin(), b.vectorTable.cend(), 
            [&b](const auto& sq){
                return *sq == *b.enPassantTarget;
            }
        ));
        const int fileNum = Board::convertOuterBoardIndex(targetIndex, cornerIndex) % INNER_BOARD_SIZE;
        newHash ^= HASH_VALUES[static_cast<int>(SquareState::EN_PASSANT_FILE) + fileNum];
    }
    return newHash;
}

size_t std::hash<Piece>::operator()(const Piece& p) const {
    return std::hash<char>()(static_cast<char>(p.type)) ^ std::hash<char>()(static_cast<char>(p.pieceColour));
}

size_t std::hash<PieceTypes>::operator()(const PieceTypes p) const {
    return std::hash<int>()(static_cast<int>(p));
}
