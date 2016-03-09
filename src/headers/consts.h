#ifndef CONSTS_H
#define CONSTS_H

#include <array>
#include <memory>
#include <utility>
#include "square.h"
#include "enums.h"

std::array<std::array<std::shared_ptr<Square>, 8>, 8> fillInitBoard();

static const std::array<std::array<std::shared_ptr<Square>, 8>, 8> INIT_BOARD = fillInitBoard();

static constexpr auto ZERO_LOCATION = std::make_pair(7, 7);

static const auto INNER_BOARD_SIZE = 8;

static const auto OUTER_BOARD_SIZE = 15;

#endif
