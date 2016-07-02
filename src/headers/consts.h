#ifndef CONSTS_H
#define CONSTS_H

#include <array>
#include <memory>
#include <utility>
#include <cstdint>
#include "square.h"
#include "enums.h"

static constexpr auto NUM_SQUARE_STATES = 12;

/*
 * Value meaning:
 * 64 squares in a chess board
 * 12 states for each square
 * 1 for side to move
 * 4 for castling rights
 * 8 for en passant target file
 */
static constexpr auto HASH_BOARD_LENGTH = (NUM_SQUARE_STATES * 64) + 1 + 4 + 9;


std::array<std::array<std::shared_ptr<Square>, 8>, 8> fillInitBoard();
std::array<uint_fast64_t, HASH_BOARD_LENGTH> populateHashTable();


static const std::array<std::array<std::shared_ptr<Square>, 8>, 8> INIT_BOARD = fillInitBoard();

static constexpr auto INNER_BOARD_SIZE = 8;

static constexpr auto OUTER_BOARD_SIZE = 15;

static constexpr auto ZERO_LOCATION = std::make_pair(7, 7);

static constexpr auto ZERO_LOCATION_1D = (ZERO_LOCATION.first * OUTER_BOARD_SIZE) + ZERO_LOCATION.second;

static const std::array<uint_fast64_t, HASH_BOARD_LENGTH> HASH_VALUES = populateHashTable();

#endif
