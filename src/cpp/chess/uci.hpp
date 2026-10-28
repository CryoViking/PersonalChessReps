#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "chess/types.hpp"

namespace chess {

// UCI move text: "e2e4", or "e7e8q" for a promotion (lowercase piece letter).
// This is the format used across the bridge and by engines such as Stockfish.
[[nodiscard]] std::string move_to_uci(const Move& move);
[[nodiscard]] std::optional<Move> move_from_uci(std::string_view text);

} // namespace chess
