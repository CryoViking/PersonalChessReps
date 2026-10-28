#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <stdexcept>
#include <string_view>

#include "chess/position.hpp"
#include "chess/types.hpp"

using namespace chess;

namespace {
constexpr std::string_view START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
} // namespace

TEST_CASE("from_fen places for static pieces") {
  const auto position = Position::from_fen(START_FEN);

  REQUIRE(position.piece_at(square_at(4, 0)) ==
          Piece{.colour = Colour::White, .type = PieceType::King}); // e1
  REQUIRE(position.piece_at(square_at(3, 7)) ==
          Piece{.colour = Colour::Black, .type = PieceType::Queen}); // d8
  REQUIRE(position.piece_at(square_at(0, 1)) ==
          Piece{.colour = Colour::White, .type = PieceType::Pawn}); // a2
  REQUIRE(position.piece_at(square_at(4, 3)) == std::nullopt);      // e4
}

TEST_CASE("from_fen fills exactly 32 squares in the starting position") {
  const auto position = Position::from_fen(START_FEN);

  int occupied = 0;
  for(Square s = 0; s < SQUARE_COUNT; ++s) {
    if(position.piece_at(s)) ++occupied;
  }
  REQUIRE(occupied == 32);
}

TEST_CASE("from_fen rejects malformed placements") {
  SECTION("too few ranks") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8 w - - 0 1"), std::invalid_argument);
  }
  SECTION("too many ranks") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8/8 w - - 0 1"), std::invalid_argument);
  }
  SECTION("a rank with too many files") {
    REQUIRE_THROWS_AS(Position::from_fen("ppppppppp/8/8/8/8/8/8/8 w - - 0 1"),
                      std::invalid_argument);
  }
  SECTION("a digit pushing a rank past 8 files") {
    REQUIRE_THROWS_AS(Position::from_fen("p8/8/8/8/8/8/8/8 w - - 0 1"), std::invalid_argument);
  }
  SECTION("a short rank") {
    REQUIRE_THROWS_AS(Position::from_fen("7/8/8/8/8/8/8/8 w - - 0 1"), std::invalid_argument);
  }
  SECTION("an invalid character") {
    REQUIRE_THROWS_AS(Position::from_fen("ppppxppp/8/8/8/8/8/8/8 w - - 0 1"),
                      std::invalid_argument);
  }
  SECTION("a trailing slash") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8/ w - - 0 1"), std::invalid_argument);
  }
}
