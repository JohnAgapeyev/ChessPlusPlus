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
        bool inCheck();
        Move createMove(std::string& input);
};

#endif
