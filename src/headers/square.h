#ifndef SQUARE_H
#define SQUARE_H

#include <memory>
#include <iostream>
#include "piece.h"
#include "enums.h"

/**
 * Class representing a square on the board.
 */
class Square {
    std::unique_ptr<Piece> piece;
    int offset;
    
public:
    Square() = default;
    Square(const Square& sq) : piece((sq.piece) ? std::make_unique<Piece>(*sq.piece) : nullptr), offset(sq.offset) {}
    Square(Square&& sq) = default;
    Square(const int off) : piece(std::make_unique<Piece>(PieceTypes::UNKNOWN, Colour::UNKNOWN)), offset(off) {}
    Square(const Piece p, const int off = 0) : piece(std::make_unique<Piece>(p)), offset(off) {}
    Square(std::unique_ptr<Piece> p, const int off = 0) : piece(std::move(p)), offset(off) {}
    
    void setPiece(std::unique_ptr<Piece> p) {piece = std::move(p);}
    void setPiece(const Piece& p) {piece = std::make_unique<Piece>(p);}
    void setOffset(const int off) {offset = off;}
    auto getOffset() const {return offset;}
    auto getPiece() const {return piece.get();}
    auto releasePiece() {return piece.release();}
    
    bool checkSentinel() const {
        return (getPiece() && getPiece()->getColour() == Colour::UNKNOWN 
            && getPiece()->getType() == PieceTypes::UNKNOWN);
    }

    bool operator==(const Square& second) {
        return ((offset == second.offset) && !piece && !second.piece) 
            || ((offset == second.offset) && piece && second.piece && *piece == *second.piece);
    }

    bool operator!=(const Square& second) {
        return !(*this == second);
    }

    Square& operator=(const Square& sq) {
        piece = (sq.piece) ? std::make_unique<Piece>(*sq.piece) : nullptr;
        offset = sq.offset;
        return *this;
    }

    Square& operator=(Square&& sq) {
        piece = std::move(sq.piece);
        offset = sq.offset;
        return *this;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Square& square);
};

#endif
