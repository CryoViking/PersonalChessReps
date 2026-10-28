#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <string_view>

#include "chess/perft.hpp"
#include "chess/position.hpp"

using namespace chess;

// Known perft totals: https://www.chessprogramming.org/Perft_Results
// Tests tagged [slow] take around half a second in a debug build. Skip them while
// iterating with: pcr_tests "~[slow]"

namespace {

constexpr std::string_view START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// Castling, pins, en passant and promotions all in one position.
constexpr std::string_view KIWIPETE =
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

// A sparse endgame full of checks and en passant discovered-check traps.
constexpr std::string_view ENDGAME = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";

std::uint64_t perft_from(std::string_view fen, int depth) {
  return perft(Position::from_fen(fen), depth);
}

} // namespace

TEST_CASE("perft: starting position") {
  REQUIRE(perft_from(START_FEN, 1) == 20);
  REQUIRE(perft_from(START_FEN, 2) == 400);
  REQUIRE(perft_from(START_FEN, 3) == 8'902);
}

TEST_CASE("perft: starting position, depth 4", "[slow]") {
  REQUIRE(perft_from(START_FEN, 4) == 197'281);
}

TEST_CASE("perft: Kiwipete") {
  REQUIRE(perft_from(KIWIPETE, 1) == 48);
  REQUIRE(perft_from(KIWIPETE, 2) == 2'039);
}

TEST_CASE("perft: Kiwipete, depth 3", "[slow]") { REQUIRE(perft_from(KIWIPETE, 3) == 97'862); }

TEST_CASE("perft: endgame position") {
  REQUIRE(perft_from(ENDGAME, 1) == 14);
  REQUIRE(perft_from(ENDGAME, 2) == 191);
  REQUIRE(perft_from(ENDGAME, 3) == 2'812);
}

TEST_CASE("perft: endgame position, depth 4", "[slow]") {
  REQUIRE(perft_from(ENDGAME, 4) == 43'238);
}

TEST_CASE("perft_divide splits the total by first move") {
  const auto counts = perft_divide(Position::from_fen(START_FEN), 2);

  REQUIRE(counts.size() == 20);

  // After any first move, Black has 20 replies, so every entry is 20.
  REQUIRE(std::ranges::all_of(counts, [](const auto& entry) { return entry.second == 20; }));

  const auto e2e4 =
      std::ranges::find_if(counts, [](const auto& entry) { return entry.first == "e2e4"; });
  REQUIRE(e2e4 != counts.end());
}
