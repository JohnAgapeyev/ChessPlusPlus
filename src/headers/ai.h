#ifndef AI_H
#define AI_H

#include <climits>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include "move.h"
#include "board.h"
#include "tt.h"
#include "consts.h"

#ifndef CACHE_MB
#define CACHE_MB 4096
#endif

class AI {
    static constexpr auto MATE = SHRT_MAX;
    static constexpr auto DRAW = 0;
    
    static constexpr auto PAWN_VAL = 100;
    static constexpr auto KNIGHT_VAL = 300;
    static constexpr auto BISHOP_VAL = 300;
    static constexpr auto ROOK_VAL = 500;
    static constexpr auto QUEEN_VAL = 900;
    static constexpr auto KING_VAL = 3000;
    
    static constexpr auto MOBILITY_VAL = 1;
    
    static constexpr auto DOUBLED_PAWN_PENALTY = 40;
    static constexpr auto ISOLATED_PAWN_PENALTY = 50;
    static constexpr auto BACKWARD_PAWN_PENALTY = 30;
    static constexpr auto PASSED_PAWN_VAL = 30;
    static constexpr auto PAWN_SIX_VAL = 70;
    static constexpr auto PAWN_SEVEN_VAL = 100;
    
    static constexpr auto OPEN_FILE_VAL = 20;
    static constexpr auto HALF_OPEN_FILE_VAL = 10;
    static constexpr auto ROOK_SEVEN_VAL = 10;
    
    static constexpr auto CASTLE_BONUS = 30;
    
    static std::unordered_multimap<Piece, std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>> initializeMap();
    
    static const std::unordered_multimap<Piece, std::array<int, INNER_BOARD_SIZE * INNER_BOARD_SIZE>> pieceSquareTables;

    using cache_key = Board;
    using cache_value = std::tuple<int, int, SearchBoundary, Move, int, int, int>;
    using cache_pointer_type = Cache<cache_key, cache_value, (static_cast<uint64_t>(CACHE_MB) << 20ul) / sizeof(Cache<cache_key, cache_value, 1>)>;
    static std::unique_ptr<cache_pointer_type> boardCache;

    static const Move emptyMove;
    
    std::array<Move, 6 * INNER_BOARD_SIZE * INNER_BOARD_SIZE> counterMove; 
    
    const int DEPTH = 7;
    
    bool isWhitePlayer = false;
    
    bool hasBlackCastled = false;
    bool hasWhiteCastled = false;

    Board& gameBoard;
    
    int eval = 0;
    
    Move prev = Move();
    int previousToSquareIndex = -1;

    std::atomic_bool usingTimeLimit{false};
    std::condition_variable cv;
    std::mutex mut;
    std::atomic_bool isTimeUp{true};
    std::chrono::seconds moveTimeLimit{10};

    std::atomic_bool isAIActive{true};

    std::thread timeLimitThread;

    int reduceKnightMobilityScore(const std::vector<Move>& moveList, const int cornerIndex, const Board& board) const;
    std::tuple<Move, int, int, int, int> iterativeDeepening();
    std::tuple<Move, int, int, int, int> MTD(const int guess, const int depth, Board& board);
    std::tuple<Move, int, int, int, int> AlphaBeta(const int alpha, const int beta, const int depth, Board& board);
    int getPieceValue(const PieceTypes type) const;
    unsigned long long perft(int depth, Board& board);
    unsigned long long perftDivide(int depth, Board& board);
    std::vector<Move> orderMoveList(std::vector<Move>&& list, Board& board);
    void translateMovePointers(Board& b, Move& mv, std::tuple<int, int, int> conversionOffsets);
    
public:
    AI(Board& b);
    ~AI();
    void evaluate(Board& board);
    void search();
    auto getEval() const {return static_cast<double>(eval) / 100;}
    void benchmarkPerft();
    void setInfiniteMode(const bool val) {usingTimeLimit = !val;}
    void setMoveTimeLimit(const unsigned long secs) {usingTimeLimit = true; moveTimeLimit = std::chrono::seconds(secs);}
};

#endif
