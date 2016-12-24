#include "headers/move.h"
#include "headers/square.h"
#include "headers/enums.h"

Move::Move() : fromSq(nullptr), toSq(nullptr), fromPieceType(PieceTypes::UNKNOWN), 
        fromPieceColour(Colour::UNKNOWN), captureMade(false), 
        toPieceType(PieceTypes::UNKNOWN), toPieceColour(Colour::UNKNOWN), 
        promotionType(PieceTypes::UNKNOWN), isCastle(false), castleRights(0), 
        enPassantActive(false), enPassantTarget(nullptr), halfMoveClock(0) {}

void swapOffsets(const Move& mv) {
    const auto temp = mv.fromSq->getOffset();
    mv.fromSq->setOffset(mv.toSq->getOffset());
    mv.toSq->setOffset(temp);
}
