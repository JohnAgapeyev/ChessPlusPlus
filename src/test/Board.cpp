#include "headers/enums.h"

class Board {

private:
    static const auto initialState = {
        {
            Square(Piece<PieceTypes::ROOK, Colour::BLACK>),
            Square(Piece<PieceTypes::KNIGHT, Colour::BLACK>),
            Square(Piece<PieceTypes::BISHOP, Colour::BLACK>),
            Square(Piece<PieceTypes::QUEEN, Colour::BLACK>),
            Square(Piece<PieceTypes::KING, Colour::BLACK>),
            Square(Piece<PieceTypes::BISHOP, Colour::BLACK>),
            Square(Piece<PieceTypes::KNIGHT, Colour::BLACK>),
            Square(Piece<PieceTypes::ROOK, Colour::BLACK>)
        },
        {Square(Piece<PieceTypes::PAWN, Colour::BLACK>)},
        {Square()},
        {Square()},
        {Square()},
        {Square()},
        {Square(Piece<PieceTypes::PAWN, Colour::WHITE>)},
        {
            Square(Piece<PieceTypes::ROOK, Colour::WHITE>),
            Square(Piece<PieceTypes::KNIGHT, Colour::WHITE>),
            Square(Piece<PieceTypes::BISHOP, Colour::WHITE>),
            Square(Piece<PieceTypes::QUEEN, Colour::WHITE>),
            Square(Piece<PieceTypes::KING, Colour::WHITE>),
            Square(Piece<PieceTypes::BISHOP, Colour::WHITE>),
            Square(Piece<PieceTypes::KNIGHT, Colour::WHITE>),
            Square(Piece<PieceTypes::ROOK, Colour::WHITE>)
        }
        };
    
    static const std::array<std::array<int, 15>, 15> vectorTable = fillVectorTable(vectorTable);
    
    const std::array<std::array<Square, 8>, 8> boardSquares = initialState;
    
    //See if the following loops can be replaced with iterator
    static constexpr void fillVectorTable(auto& out) {
        static int count = -112;
        for (int i = 0; i < 15; ++i) {
            for (int j = 0; j < 15; ++j) {
                out[i][j] = count++;
            }
        }
    }
    
public:
    
}
