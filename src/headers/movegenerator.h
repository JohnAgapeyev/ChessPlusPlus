#ifndef MOVEGEN_H
#define MOVEGEN_H

class Board::MoveGenerator {
    Board& board;
    void logMoveFailure(const int failureNum, const bool isSilent) const;
    
public:
    MoveGenerator(Board& b) : board(b) {}
    std::vector<Move> generateAll();
    bool validateMove(const Move& mv, const bool isSilent);
    bool inCheck(const Move& mv) const;
    bool inCheck(const int squareIndex) const;
    Move createMove(std::string& input);
    int getOffsetIndex(const int offset, const int startIndex = 0, const int vectorLen = 1) const;
    bool getCastleDirectionBool(const PieceTypes type, const Colour pieceColour, const int offset) const;
};

#endif
