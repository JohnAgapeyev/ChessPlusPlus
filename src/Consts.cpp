#include "headers/consts.h"
#include <array>
#include <memory>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "headers/square.h"
#include "headers/enums.h"

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

std::unordered_map<char, std::vector<int>> initPieceOffsets() {
    std::unordered_map<char, std::vector<int>> rtn;
    std::vector<int> vecOffsets;
    const auto& sortOffsets = [](const auto& first, const auto& second){
        return std::abs(first) > std::abs(second);
    };
    vecOffsets.push_back(14);
    vecOffsets.push_back(15);
    vecOffsets.push_back(16);
    vecOffsets.push_back(30);
    std::sort(vecOffsets.begin(), vecOffsets.end(), sortOffsets);
    
    // White Pawn
    rtn.insert(std::make_pair('P', vecOffsets));
    vecOffsets.clear();
    vecOffsets.reserve(4);
    
    vecOffsets.push_back(-14);
    vecOffsets.push_back(-15);
    vecOffsets.push_back(-16);
    vecOffsets.push_back(-30);
    std::sort(vecOffsets.begin(), vecOffsets.end(), sortOffsets);
    
    // Black Pawn
    rtn.insert(std::make_pair('p', vecOffsets));
    vecOffsets.clear();
    vecOffsets.reserve(8);
    
    vecOffsets.push_back(13);
    vecOffsets.push_back(29);
    vecOffsets.push_back(31);
    vecOffsets.push_back(17);
    vecOffsets.push_back(-13);
    vecOffsets.push_back(-29);
    vecOffsets.push_back(-31);
    vecOffsets.push_back(-17);
    std::sort(vecOffsets.begin(), vecOffsets.end(), sortOffsets);
    
    // Knight
    rtn.insert(std::make_pair(static_cast<char>(PieceTypes::KNIGHT), vecOffsets));
    vecOffsets.clear();
    vecOffsets.reserve(4);
    
    vecOffsets.push_back(14);
    vecOffsets.push_back(16);
    vecOffsets.push_back(-14);
    vecOffsets.push_back(-16);
    std::sort(vecOffsets.begin(), vecOffsets.end(), sortOffsets);
    
    // Bishop
    rtn.insert(std::make_pair(static_cast<char>(PieceTypes::BISHOP), vecOffsets));
    vecOffsets.clear();
    vecOffsets.reserve(4);
    
    vecOffsets.push_back(15);
    vecOffsets.push_back(1);
    vecOffsets.push_back(-15);
    vecOffsets.push_back(-1);
    std::sort(vecOffsets.begin(), vecOffsets.end(), sortOffsets);
    
    // Rook
    rtn.insert(std::make_pair(static_cast<char>(PieceTypes::ROOK), vecOffsets));
    vecOffsets.clear();
    vecOffsets.reserve(8);
    
    vecOffsets.push_back(15);
    vecOffsets.push_back(1);
    vecOffsets.push_back(-15);
    vecOffsets.push_back(-1);
    vecOffsets.push_back(14);
    vecOffsets.push_back(16);
    vecOffsets.push_back(-14);
    vecOffsets.push_back(-16);
    std::sort(vecOffsets.begin(), vecOffsets.end(), sortOffsets);
    
    // King/Queen
    rtn.insert(std::make_pair(static_cast<char>(PieceTypes::KING), vecOffsets));
    rtn.insert(std::make_pair(static_cast<char>(PieceTypes::QUEEN), vecOffsets));
    vecOffsets.clear();
    vecOffsets.reserve(16);
    
    const auto& kingQueenOffsets = rtn[static_cast<char>(PieceTypes::KING)];
    const auto& knightOffsets = rtn[static_cast<char>(PieceTypes::KNIGHT)];
    
    std::merge(kingQueenOffsets.begin(), kingQueenOffsets.end(), 
            knightOffsets.begin(), knightOffsets.end(), 
            vecOffsets.begin(), sortOffsets);
    
    rtn.insert(std::make_pair(static_cast<char>(PieceTypes::UNKNOWN), vecOffsets));
    
    return rtn;
}
