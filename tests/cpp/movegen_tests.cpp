#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "chess/position.hpp"
#include "chess/types.hpp"

using namespace chess;

namespace {

constexpr std::string_view START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// Builds a move from square names, e.g. move("g1", "f3").
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): from, to is chess order
Move move(std::string_view from, std::string_view to,
          std::optional<PieceType> promotion = std::nullopt) {
  const auto from_square = square_from_name(from);
  const auto to_square = square_from_name(to);
  if(!from_square || !to_square) throw std::invalid_argument("bad square name in test");
  return Move{.from = *from_square, .to = *to_square, .promotion = promotion};
}

bool contains(const std::vector<Move>& moves, const Move& wanted) {
  return std::ranges::find(moves, wanted) != moves.end();
}

} // namespace

TEST_CASE("legal_moves: the starting position has 20 moves") {
  const auto moves = Position::from_fen(START_FEN).legal_moves();

  // 16 pawn moves (a single and a double push for each pawn) and 4 knight moves.
  REQUIRE(moves.size() == 20);
  REQUIRE(contains(moves, move("e2", "e3")));
  REQUIRE(contains(moves, move("e2", "e4")));
  REQUIRE(contains(moves, move("b1", "a3")));
  REQUIRE(contains(moves, move("b1", "c3")));
  REQUIRE(contains(moves, move("g1", "f3")));
  REQUIRE(contains(moves, move("g1", "h3")));
}

TEST_CASE("in_check reports whether the side to move is attacked") {
  SECTION("not in the starting position") {
    REQUIRE_FALSE(Position::from_fen(START_FEN).in_check());
  }
  SECTION("a rook on the king's file") {
    REQUIRE(Position::from_fen("4k3/8/8/8/8/8/8/4R2K b - - 0 1").in_check());
  }
  SECTION("only the side to move counts") {
    // Same position, but White to move: White's king on h1 isn't attacked.
    REQUIRE_FALSE(Position::from_fen("4k3/8/8/8/8/8/8/4R2K w - - 0 1").in_check());
  }
  SECTION("a board with no king is never in check") {
    REQUIRE_FALSE(Position::from_fen("8/8/8/8/8/8/8/8 w - - 0 1").in_check());
  }
}

TEST_CASE("legal_moves: piece mobility on an otherwise empty board") {
  const auto count = [](std::string_view fen) {
    return Position::from_fen(fen).legal_moves().size();
  };

  SECTION("a king in the centre and in a corner") {
    REQUIRE(count("8/8/8/8/4K3/8/8/8 w - - 0 1") == 8);
    REQUIRE(count("K7/8/8/8/8/8/8/8 w - - 0 1") == 3);
  }
  SECTION("a rook always has 14 moves") {
    REQUIRE(count("8/8/8/8/8/8/8/R7 w - - 0 1") == 14);
    REQUIRE(count("8/8/8/8/3R4/8/8/8 w - - 0 1") == 14);
  }
  SECTION("a bishop in the centre and in a corner") {
    REQUIRE(count("8/8/8/8/3B4/8/8/8 w - - 0 1") == 13);
    REQUIRE(count("8/8/8/8/8/8/8/B7 w - - 0 1") == 7);
  }
  SECTION("a queen combines both") { REQUIRE(count("8/8/8/8/3Q4/8/8/8 w - - 0 1") == 27); }
}

TEST_CASE("legal_moves: sliders stop at the first piece") {
  SECTION("blocked by a friendly piece, which it can't capture") {
    // Rook a1, own knight a4: the rook reaches a2 and a3 up the file, but not a4.
    const auto moves = Position::from_fen("8/8/8/8/N7/8/8/R7 w - - 0 1").legal_moves();
    REQUIRE(contains(moves, move("a1", "a3")));
    REQUIRE_FALSE(contains(moves, move("a1", "a4")));
    REQUIRE_FALSE(contains(moves, move("a1", "a5")));
  }
  SECTION("stopped by an enemy piece, which it can capture") {
    const auto moves = Position::from_fen("8/8/8/8/n7/8/8/R7 w - - 0 1").legal_moves();
    REQUIRE(contains(moves, move("a1", "a4")));
    REQUIRE_FALSE(contains(moves, move("a1", "a5")));
  }
}

TEST_CASE("legal_moves: moves that leave the king attacked are removed") {
  const auto count = [](std::string_view fen) {
    return Position::from_fen(fen).legal_moves().size();
  };

  SECTION("a pinned rook can only move along the pin") {
    // White rook e2 is pinned by the rook on e7: 5 moves up the e-file (including Rxe7),
    // none sideways, plus 4 king moves.
    const auto moves = Position::from_fen("4k3/4r3/8/8/8/8/4R3/4K3 w - - 0 1").legal_moves();
    REQUIRE(moves.size() == 9);
    REQUIRE(contains(moves, move("e2", "e7")));
    REQUIRE_FALSE(contains(moves, move("e2", "d2")));
  }
  SECTION("in check, only escapes are allowed") {
    // Rook a1 checks along the first rank: only d2, e2 and f2 get out of it.
    REQUIRE(count("4k3/8/8/8/8/8/8/r3K3 w - - 0 1") == 3);
  }
  SECTION("the king may capture an unprotected attacker") {
    const auto moves = Position::from_fen("4k3/8/8/8/8/8/3r4/4K3 w - - 0 1").legal_moves();
    REQUIRE(moves.size() == 2);
    REQUIRE(contains(moves, move("e1", "d2")));
  }
  SECTION("but not a protected one") {
    // The queen on d3 guards d2 and covers f1: no legal moves at all.
    REQUIRE(count("4k3/8/8/8/8/3q4/3r4/4K3 w - - 0 1") == 0);
  }
  SECTION("with no king on the board, nothing is filtered") {
    REQUIRE(count("8/8/8/8/8/8/8/R7 w - - 0 1") == 14);
  }
}
