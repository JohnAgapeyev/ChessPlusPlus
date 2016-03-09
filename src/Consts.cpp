#include "headers/consts.h"

std::array<std::array<std::shared_ptr<Square>, 8>, 8> fillInitBoard() {
    auto pickColour = [](auto T){return (T < 5) ? Colour::BLACK : Colour::WHITE;};
    std::array<std::array<std::shared_ptr<Square>, 8>, 8> result;
    for (int i = 0; i < 8; ++i) {
        std::array<std::shared_ptr<Square>, 8> row;
        for (int j = 0; j < 8; ++j) {
            switch(i) {
                case 0:
                case 7:
                    switch(j) {
                        case 0:
                        case 7:
                            row[j] = std::make_shared<Square>(Piece(PieceTypes::ROOK, pickColour(i)));
                            break;
                        case 1:
                        case 6:
                            row[j] = std::make_shared<Square>(Piece(PieceTypes::KNIGHT, pickColour(i)));
                            break;
                        case 2:
                        case 5:
                            row[j] = std::make_shared<Square>(Piece(PieceTypes::BISHOP, pickColour(i)));
                            break;
                        case 3:
                            row[j] = std::make_shared<Square>(Piece(PieceTypes::QUEEN, pickColour(i)));
                            break;
                        case 4:
                            row[j] = std::make_shared<Square>(Piece(PieceTypes::KING, pickColour(i)));
                            break;
                    }
                    break;
                case 1:
                case 6:
                    row[j] = std::make_shared<Square>(Piece(PieceTypes::PAWN, pickColour(i)));
                    break;
                default:
                    row[j] = std::make_shared<Square>();
                    break;
            }
        }
        result[i] = std::move(row);
    }
    return result;
}
