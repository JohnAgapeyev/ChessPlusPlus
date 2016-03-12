#ifndef MOVE_H
#define MOVE_H

#include "square.h"
#include <memory>

typedef struct Move Move;
struct Move {
    std::shared_ptr<Square> fromSq;
    std::shared_ptr<Square> toSq;
};
#endif
