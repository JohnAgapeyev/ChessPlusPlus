#ifndef MOVE_H
#define MOVE_H

#include "square.h"
#include "piece.h"
#include "enums.h"

typedef struct Move Move;
struct Move {
    Move();
    
    Square *fromSq;
    Square *toSq;
    PieceTypes fromPieceType;
    Colour fromPieceColour;
    bool captureMade;
    PieceTypes toPieceType;
    Colour toPieceColour;
    PieceTypes promotionType;
    bool isCastle;
    unsigned char castleRights;
    bool enPassantActive;
    Square* enPassantTarget;
    int halfMoveClock;
};

void swapOffsets(const Move& mv);

bool operator==(const Move& first, const Move& second);
bool operator!=(const Move& first, const Move& second);

#endif
