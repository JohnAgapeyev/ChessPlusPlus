#ifndef MOVE_H
#define MOVE_H

#include "square.h"
#include <memory>

typedef struct Move Move;
struct Move {
    Square *fromSq;
    Square *toSq;
};
#endif
