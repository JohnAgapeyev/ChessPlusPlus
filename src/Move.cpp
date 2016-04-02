#include "headers/move.h"
#include "headers/square.h"

void swapOffsets(const Move& mv) {
    const auto temp = mv.fromSq->getOffset();
    mv.fromSq->setOffset(mv.toSq->getOffset());
    mv.toSq->setOffset(temp);
}
