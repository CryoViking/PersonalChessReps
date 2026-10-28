#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>

#include "chess/types.hpp"

using namespace chess;

TEST_CASE("square_at numbers the corners a1, h1, a8, h8") {
  STATIC_REQUIRE(square_at(0, 0) == 0);
  STATIC_REQUIRE(square_at(7, 0) == 7);
  STATIC_REQUIRE(square_at(0, 7) == 56);
  STATIC_REQUIRE(square_at(7, 7) == 63);
  STATIC_REQUIRE(square_from_name("e3") == square_at(4, 2));
  STATIC_REQUIRE(!square_from_name("i1"));
  STATIC_REQUIRE(square_from_name("a1") == 0); // corners: the edges of both ranges.
  STATIC_REQUIRE(square_from_name("h8") == 63);
  STATIC_REQUIRE(!square_from_name("a9"));  // rank out of range
  STATIC_REQUIRE(!square_from_name("e"));   // too short
  STATIC_REQUIRE(!square_from_name("e33")); // too long
  STATIC_REQUIRE(!square_from_name("E3"));  // uppercase rejected
}

TEST_CASE("col_of and row_of invert square_at for every square") {
  for(Square s = 0; s < BOARD_SIZE * BOARD_SIZE; ++s) {
    CAPTURE(s);
    REQUIRE(square_at(col_of(s), row_of(s)) == s);
  }
}

TEST_CASE("pieces compare by colour and type") {
  const Piece white_knight{.colour = Colour::White, .type = PieceType::Knight};

  REQUIRE(white_knight == Piece{Colour::White, PieceType::Knight});
  REQUIRE(white_knight != Piece{Colour::Black, PieceType::Knight});
  REQUIRE(white_knight != Piece{Colour::White, PieceType::Bishop});
}
