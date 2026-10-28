#include "chess/perft.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "chess/position.hpp"
#include "chess/types.hpp"
#include "chess/uci.hpp"

namespace chess {

// NOLINTNEXTLINE(misc-no-recursion): recursion depth is the perft depth, a small number
std::uint64_t perft(const Position& position, int depth) {
  if(depth == 0) return 1;

  std::uint64_t total = 0;
  for(const Move& move : position.legal_moves())
    total += perft(position.make_move(move), depth - 1);
  return total;
}

std::vector<std::pair<std::string, std::uint64_t>> perft_divide(const Position& position,
                                                                int depth) {
  std::vector<std::pair<std::string, std::uint64_t>> counts;
  for(const Move& move : position.legal_moves()) {
    counts.emplace_back(move_to_uci(move), perft(position.make_move(move), depth - 1));
  }
  return counts;
}

} // namespace chess
