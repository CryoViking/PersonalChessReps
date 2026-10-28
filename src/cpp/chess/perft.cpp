#include "chess/perft.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "chess/position.hpp"
#include "chess/types.hpp"

namespace chess {

namespace {
// UCI-style move text: "e2e4", or "e7e8q" for a promotion.
std::string move_name(const Move& move) {
  std::string name = square_name(move.from) + square_name(move.to);
  if(move.promotion) {
    switch(*move.promotion) {
      case PieceType::Knight:
        name += 'n';
      case PieceType::Bishop:
        name += 'b';
      case PieceType::Rook:
        name += 'r';
      case PieceType::Queen:
        name += 'q';
      case PieceType::Pawn:
      case PieceType::King:
        break;
    }
  }
  return name;
}

} // namespace

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
    counts.emplace_back(move_name(move), perft(position.make_move(move), depth - 1));
  }
  return counts;
}

} // namespace chess
