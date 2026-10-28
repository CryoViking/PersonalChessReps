#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string_view>

#include "chess/position.hpp"
#include "chess/types.hpp"

using namespace chess;

namespace {

// Is `square_name` attacked by `by` in the position given by `fen`?
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): position, then square
bool attacked(std::string_view fen, std::string_view square_name, Colour by) {
  const auto square = square_from_name(square_name);
  if(!square) throw std::invalid_argument("bad square name in test");
  return Position::from_fen(fen).is_square_attacked(*square, by);
}

} // namespace

TEST_CASE("is_square_attacked: knights") {
  constexpr std::string_view KNIGHT_ON_E4 = "8/8/8/8/4N3/8/8/8 w - - 0 1";

  SECTION("all eight knight squares") {
    for(const std::string_view target : {"f6", "g5", "g3", "f2", "d2", "c3", "c5", "d6"}) {
      CAPTURE(target);
      REQUIRE(attacked(KNIGHT_ON_E4, target, Colour::White));
    }
  }
  SECTION("not an adjacent square") { REQUIRE_FALSE(attacked(KNIGHT_ON_E4, "e5", Colour::White)); }
  SECTION("no wrap-around from the h-file") {
    constexpr std::string_view KNIGHT_ON_H1 = "8/8/8/8/8/8/8/7N w - - 0 1";
    REQUIRE(attacked(KNIGHT_ON_H1, "g3", Colour::White));
    REQUIRE(attacked(KNIGHT_ON_H1, "f2", Colour::White));
    REQUIRE_FALSE(attacked(KNIGHT_ON_H1, "a2", Colour::White));
    REQUIRE_FALSE(attacked(KNIGHT_ON_H1, "a3", Colour::White));
    REQUIRE_FALSE(attacked(KNIGHT_ON_H1, "b3", Colour::White));
  }
}

TEST_CASE("is_square_attacked: kings") {
  constexpr std::string_view KING_ON_E1 = "8/8/8/8/8/8/8/4K3 w - - 0 1";

  REQUIRE(attacked(KING_ON_E1, "d2", Colour::White));
  REQUIRE(attacked(KING_ON_E1, "f1", Colour::White));
  REQUIRE_FALSE(attacked(KING_ON_E1, "e3", Colour::White));
}

TEST_CASE("is_square_attacked: sliders") {
  SECTION("a rook down an open file") {
    REQUIRE(attacked("8/8/8/8/8/8/8/R7 w - - 0 1", "a8", Colour::White));
  }
  SECTION("a rook is blocked by the first piece") {
    constexpr std::string_view ROOK_BEHIND_PAWN = "8/8/8/8/P7/8/8/R7 w - - 0 1";
    REQUIRE(attacked(ROOK_BEHIND_PAWN, "a4", Colour::White));
    REQUIRE_FALSE(attacked(ROOK_BEHIND_PAWN, "a5", Colour::White));
  }
  SECTION("a bishop along a diagonal, but not straight") {
    constexpr std::string_view BISHOP_ON_C1 = "8/8/8/8/8/8/8/2B5 w - - 0 1";
    REQUIRE(attacked(BISHOP_ON_C1, "h6", Colour::White));
    REQUIRE_FALSE(attacked(BISHOP_ON_C1, "c2", Colour::White));
  }
  SECTION("a queen both ways, but not like a knight") {
    constexpr std::string_view QUEEN_ON_D4 = "8/8/8/8/3Q4/8/8/8 w - - 0 1";
    REQUIRE(attacked(QUEEN_ON_D4, "d8", Colour::White));
    REQUIRE(attacked(QUEEN_ON_D4, "h8", Colour::White));
    REQUIRE_FALSE(attacked(QUEEN_ON_D4, "e6", Colour::White));
  }
}

TEST_CASE("is_square_attacked: pawns") {
  SECTION("a White pawn attacks diagonally upwards") {
    constexpr std::string_view WHITE_PAWN_ON_E4 = "8/8/8/8/4P3/8/8/8 w - - 0 1";
    REQUIRE(attacked(WHITE_PAWN_ON_E4, "d5", Colour::White));
    REQUIRE(attacked(WHITE_PAWN_ON_E4, "f5", Colour::White));
    REQUIRE_FALSE(attacked(WHITE_PAWN_ON_E4, "e5", Colour::White));
    REQUIRE_FALSE(attacked(WHITE_PAWN_ON_E4, "d3", Colour::White));
  }
  SECTION("a Black pawn attacks diagonally downwards") {
    constexpr std::string_view BLACK_PAWN_ON_E5 = "8/8/8/4p3/8/8/8/8 w - - 0 1";
    REQUIRE(attacked(BLACK_PAWN_ON_E5, "d4", Colour::Black));
    REQUIRE(attacked(BLACK_PAWN_ON_E5, "f4", Colour::Black));
    REQUIRE_FALSE(attacked(BLACK_PAWN_ON_E5, "d6", Colour::Black));
  }
  SECTION("no wrap-around from the a-file") {
    constexpr std::string_view WHITE_PAWN_ON_A2 = "8/8/8/8/8/8/P7/8 w - - 0 1";
    REQUIRE(attacked(WHITE_PAWN_ON_A2, "b3", Colour::White));
    REQUIRE_FALSE(attacked(WHITE_PAWN_ON_A2, "h3", Colour::White));
    REQUIRE_FALSE(attacked(WHITE_PAWN_ON_A2, "h4", Colour::White));
  }
}

TEST_CASE("is_square_attacked only counts the given side") {
  REQUIRE(attacked("8/8/8/8/4N3/8/8/8 w - - 0 1", "f6", Colour::White));
  REQUIRE_FALSE(attacked("8/8/8/8/4N3/8/8/8 w - - 0 1", "f6", Colour::Black));
}
