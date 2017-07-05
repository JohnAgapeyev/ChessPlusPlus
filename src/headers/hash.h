#ifndef HASH_H
#define HASH_H

#include <utility>
#include <functional>

class Board;
class Piece;

/**
 * Template specialization of the std::hash class for use
 * in the transposition table
 */
namespace std {
    template<>
    class hash<Board> {
    public:
        size_t operator() (Board& b) const;
    };

    template<>
    class hash<Piece> {
    public:
        size_t operator() (const Piece& p) const;
    };
}

#endif
