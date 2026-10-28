#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "chess/position.hpp"
#include "chess/types.hpp"

using namespace chess;

namespace {

// Builds a move from square names, e.g. move("e2", "e4").
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): from, to is chess order
Move move(std::string_view from, std::string_view to,
          std::optional<PieceType> promotion = std::nullopt) {
  const auto from_square = square_from_name(from);
  const auto to_square = square_from_name(to);
  if(!from_square || !to_square) throw std::invalid_argument("bad square name in test");
  return Move{.from = *from_square, .to = *to_square, .promotion = promotion};
}

std::string after(std::string_view fen, const Move& m) {
  return Position::from_fen(fen).make_move(m).to_fen();
}

} // namespace

TEST_CASE("make_move plays ordinary moves") {
  SECTION("a double pawn push leaves an en passant square") {
    REQUIRE(after("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", move("e2", "e4")) ==
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
  }
  SECTION("the fullmove number goes up after Black moves") {
    REQUIRE(
        after("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", move("e7", "e5")) ==
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2");
  }
  SECTION("a quiet piece move counts towards the halfmove clock") {
    REQUIRE(
        after("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2", move("g1", "f3")) ==
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");
  }
}

TEST_CASE("make_move castles") {
  constexpr std::string_view CASTLING_FEN = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";

  SECTION("White kingside") {
    REQUIRE(after(CASTLING_FEN, move("e1", "g1")) == "r3k2r/8/8/8/8/8/8/R4RK1 b kq - 1 1");
  }
  SECTION("White queenside") {
    REQUIRE(after(CASTLING_FEN, move("e1", "c1")) == "r3k2r/8/8/8/8/8/8/2KR3R b kq - 1 1");
  }
  SECTION("Black kingside") {
    REQUIRE(after("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1", move("e8", "g8")) ==
            "r4rk1/8/8/8/8/8/8/R3K2R w KQ - 1 2");
  }
}

TEST_CASE("make_move handles special captures and promotion") {
  SECTION("en passant removes the pawn beside the destination") {
    REQUIRE(
        after("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3", move("e5", "f6")) ==
        "rnbqkbnr/ppp1p1pp/5P2/3p4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3");
  }
  SECTION("a pawn promotes to the chosen piece") {
    REQUIRE(after("8/P7/8/8/8/8/8/k6K w - - 0 1", move("a7", "a8", PieceType::Queen)) ==
            "Q7/8/8/8/8/8/8/k6K b - - 0 1");
  }
  SECTION("capturing a rook on its home square removes that castling right too") {
    REQUIRE(after("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", move("a1", "a8")) ==
            "R3k2r/8/8/8/8/8/8/4K2R b Kk - 0 1");
  }
  SECTION("capturing on h8 removes Black's kingside right, not White's queenside") {
    REQUIRE(after("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", move("h1", "h8")) ==
            "r3k2R/8/8/8/8/8/8/R3K3 b Qq - 0 1");
  }
}

TEST_CASE("make_move rejects a move from an empty square") {
  REQUIRE_THROWS_AS(Position::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
                        .make_move(move("e4", "e5")),
                    std::invalid_argument);
}
