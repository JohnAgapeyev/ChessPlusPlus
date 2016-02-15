#include "headers/enums.h"
#include "headers/piece.h"
#include "headers/square.h"
#include "headers/board.h"
#include <vector>
#include <array>
#include <iostream>

//const auto Board::initialState = {
    //{
        //Square(Piece<PieceTypes::ROOK, Colour::BLACK>),
        //Square(Piece<PieceTypes::KNIGHT, Colour::BLACK>),
        //Square(Piece<PieceTypes::BISHOP, Colour::BLACK>),
        //Square(Piece<PieceTypes::QUEEN, Colour::BLACK>),
        //Square(Piece<PieceTypes::KING, Colour::BLACK>),
        //Square(Piece<PieceTypes::BISHOP, Colour::BLACK>),
        //Square(Piece<PieceTypes::KNIGHT, Colour::BLACK>),
        //Square(Piece<PieceTypes::ROOK, Colour::BLACK>)
    //},
    //{Square(Piece<PieceTypes::PAWN, Colour::BLACK>)},
    //{Square()},
    //{Square()},
    //{Square()},
    //{Square()},
    //{Square(Piece<PieceTypes::PAWN, Colour::WHITE>)},
    //{
        //Square(Piece<PieceTypes::ROOK, Colour::WHITE>),
        //Square(Piece<PieceTypes::KNIGHT, Colour::WHITE>),
        //Square(Piece<PieceTypes::BISHOP, Colour::WHITE>),
        //Square(Piece<PieceTypes::QUEEN, Colour::WHITE>),
        //Square(Piece<PieceTypes::KING, Colour::WHITE>),
        //Square(Piece<PieceTypes::BISHOP, Colour::WHITE>),
        //Square(Piece<PieceTypes::KNIGHT, Colour::WHITE>),
        //Square(Piece<PieceTypes::ROOK, Colour::WHITE>)
    //}
    //};

const std::array<std::array<int, 15>, 15> Board::vectorTable = fillVectorTable();

//const std::array<std::array<Square, 8>, 8> boardSquares = initialState;

//See if the following loops can be replaced with iterator
constexpr auto Board::fillVectorTable() {
    std::array<std::array<int, 15>, 15> output{0};
    int count = -112;
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            output[i][j] = count++;
        }
    }
    return output;
}
