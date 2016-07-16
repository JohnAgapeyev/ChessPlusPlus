#ifndef MOVE_H
#define MOVE_H

#include "square.h"
#include "piece.h"

typedef struct Move Move;
struct Move {
    Square *fromSq;
    Square *toSq;
    Piece *fromPiece;
    Piece *toPiece;
    PieceTypes toPieceType;
    Colour toPieceColour;
    PieceTypes promotionType;
    bool isCastle;
    unsigned char castleRights;
    bool isEnPassant;
    Square* enPassantCaptureTarget;
};

void swapOffsets(const Move& mv);
#endif
