#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>
#include <string_view>

#include "chess/position.hpp"
#include "chess/san.hpp"
#include "chess/uci.hpp"

using namespace chess;

namespace {

constexpr std::string_view START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// The SAN for a move given in UCI, played in the position given by `fen`.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): position, then move
std::string san(std::string_view fen, std::string_view uci) {
  const auto move = move_from_uci(uci);
  if(!move) throw std::invalid_argument("bad UCI move in test");
  return move_to_san(Position::from_fen(fen), *move);
}

} // namespace

TEST_CASE("move_to_san: ordinary moves") {
  REQUIRE(san(START_FEN, "e2e4") == "e4");
  REQUIRE(san(START_FEN, "g1f3") == "Nf3");
}

TEST_CASE("move_to_san: captures") {
  SECTION("a pawn capture names the pawn's file") {
    REQUIRE(san("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2", "e4d5") ==
            "exd5");
  }
  SECTION("en passant is a capture onto an empty square") {
    REQUIRE(san("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3", "e5f6") ==
            "exf6");
  }
}

TEST_CASE("move_to_san: castling") {
  constexpr std::string_view CASTLING_FEN = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
  REQUIRE(san(CASTLING_FEN, "e1g1") == "O-O");
  REQUIRE(san(CASTLING_FEN, "e1c1") == "O-O-O");
}

TEST_CASE("move_to_san: disambiguation") {
  SECTION("by file when the pieces share a rank") {
    REQUIRE(san("k7/8/8/8/8/8/8/R6R w - - 0 1", "a1d1") == "Rad1");
  }
  SECTION("by rank when the pieces share a file") {
    REQUIRE(san("R7/8/8/8/7k/8/8/R6K w - - 0 1", "a1a4") == "R1a4+");
  }
  SECTION("by both when neither alone is enough") {
    // Queens on a1, a4 and d1 can all reach d4.
    REQUIRE(san("7K/8/7k/8/Q7/8/8/Q2Q4 w - - 0 1", "a1d4") == "Qa1d4");
  }
  SECTION("two knights that can reach the same square") {
    REQUIRE(san("4k3/8/8/8/8/2N3N1/8/4K3 w - - 0 1", "c3e4") == "Nce4");
  }
  SECTION("not when the other piece is pinned") {
    // The c3 knight is pinned to the king by the bishop on b4, so it can't go to e4.
    REQUIRE(san("4k3/8/8/8/1b6/2N3N1/8/4K3 w - - 0 1", "g3e4") == "Ne4");
  }
}

TEST_CASE("move_to_san: promotion") {
  SECTION("with check") {
    REQUIRE(san("8/P7/8/8/8/8/8/k6K w - - 0 1", "a7a8q") == "a8=Q+");
  }
  SECTION("capturing, to a knight") {
    REQUIRE(san("1n6/P7/8/8/8/8/8/k6K w - - 0 1", "a7b8n") == "axb8=N");
  }
}

TEST_CASE("move_to_san: mate") {
  // Fool's mate.
  REQUIRE(san("rnbqkbnr/pppp1ppp/8/4p3/6P1/5P2/PPPPP2P/RNBQKBNR b KQkq g3 0 2", "d8h4") ==
          "Qh4#");
}

TEST_CASE("move_to_san rejects a move from an empty square") {
  REQUIRE_THROWS_AS(san(START_FEN, "e4e5"), std::invalid_argument);
}
