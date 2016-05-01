#ifndef MOVEGEN_H
#define MOVEGEN_H

class Board::MoveGenerator {
        const Board& board;
        std::vector<Move> moveList;
        
    public:
        MoveGenerator(const Board& b) : board(b) {}
        auto getMoveList() const {return moveList;}
        void generateAll();
        bool validateMove(const Move& mv);
        bool inCheck(const int squareIndex);
        Move createMove(std::string& input);
        int getOffsetIndex(const int offset, const int startIndex = 0, const int vectorLen = 1);
};

#endif
