#ifndef HASH_H
#define HASH_H

#include <utility>
#include <functional>

class Board;
class Piece;
enum class PieceTypes : unsigned char;

namespace std {

    template<>
    class hash<Board> {
    public:
        size_t operator() (const Board& b) const;
    };

    template<>
    class hash<Piece> {
    public:
        size_t operator() (const Piece& p) const;
    };

    template<>
    class hash<PieceTypes> {
    public:
        size_t operator()(const PieceTypes p) const;
    };

}

#endif
