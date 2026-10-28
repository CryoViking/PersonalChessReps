#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "chess/position.hpp"

namespace chess {

// Counts the leaf positions reachable in exactly `depth` moves. Compare against known
// totals to verify move generation.
[[nodiscard]] std::uint64_t perft(const Position& position, int depth);

// The perft count below each legal first move, as ("e2e4", count) pairs. Compare with
// Stockfish's `go perft <depth>` output to find with move's subtree is wrong.
[[nodiscard]] std::vector<std::pair<std::string, std::uint64_t>>
perft_divide(const Position& position, int depth);

} // namespace chess
