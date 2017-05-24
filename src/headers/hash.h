#ifndef HASH_H
#define HASH_H

class Board;

namespace std {
    template<>
    class hash<Board> {
    public:
        size_t operator() (const Board& b) const;
    };
}

#endif
