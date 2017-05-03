#include "headers/move.h"
#include "headers/square.h"
#include "headers/enums.h"

Move::Move() : fromSq(nullptr), toSq(nullptr), fromPieceType(PieceTypes::UNKNOWN), 
        fromPieceColour(Colour::UNKNOWN), captureMade(false), 
        toPieceType(PieceTypes::UNKNOWN), toPieceColour(Colour::UNKNOWN), 
        promotionType(PieceTypes::UNKNOWN), promotionMade(false), isCastle(false), castleRights(0), 
        enPassantActive(false), enPassantTarget(nullptr), halfMoveClock(0), moveCounter(0) {}

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
        && first.promotionMade == second.promotionMade && first.isCastle == second.isCastle 
        && first.castleRights == second.castleRights && first.enPassantActive == second.enPassantActive 
        && first.enPassantTarget == second.enPassantTarget && first.halfMoveClock == second.halfMoveClock;
}

bool operator!=(const Move& first, const Move& second) {
    return !(first == second);
}

std::ostream& operator<<(std::ostream& os, const Move& mv) {
    os << mv.fromSq << ", " << mv.toSq;
    if (mv.fromSq) {
        os << ", " << *mv.fromSq;
    }
    if (mv.toSq) {
        os << ", " << *mv.toSq;
    }

    os << ", " << static_cast<char>(mv.fromPieceType) << ", " << static_cast<char>(mv.fromPieceColour) 
        << ", " << mv.captureMade << ", " << static_cast<char>(mv.toPieceType) 
        << ", " << static_cast<char>(mv.toPieceColour) << "," << static_cast<char>(mv.promotionType) 
        << ", " << mv.promotionMade << ", " << mv.isCastle << "," << mv.castleRights << ", " 
        << mv.enPassantActive << ", " << mv.enPassantTarget;

    if (mv.enPassantTarget) {
        os << ", " << *mv.enPassantTarget;
    }
    os << ", " << mv.halfMoveClock;
    return os;
}
