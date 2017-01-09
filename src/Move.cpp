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

bool operator==(const Move& first, const Move& second) {
    return first.fromSq == second.fromSq && first.toSq == second.toSq 
        && first.fromPieceType == second.fromPieceType && first.fromPieceColour == second.fromPieceColour 
        && first.captureMade == second.captureMade && first.toPieceType == second.toPieceType 
        && first.toPieceColour == second.toPieceColour && first.promotionType == second.promotionType 
        && first.isCastle == second.isCastle && first.castleRights == second.castleRights 
        && first.enPassantActive == second.enPassantActive && first.enPassantTarget == second.enPassantTarget 
        && first.halfMoveClock == second.halfMoveClock;
}

bool operator!=(const Move& first, const Move& second) {
    return !(first == second);
}
