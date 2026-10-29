#pragma once

#include <string>

#include "chess/position.hpp"
#include "chess/types.hpp"

namespace chess {

// Standard Algebraic Notation for `move` played in `position`: "Nf3", "exd5", "Rac8",
// "0-0", "e8=Q+", "Qh4#". `move` must be legal in `position`.
[[nodiscard]] std::string move_to_san(const Position& position, const Move& move);

} // namespace chess
