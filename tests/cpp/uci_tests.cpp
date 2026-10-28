#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string_view>

#include "chess/types.hpp"
#include "chess/uci.hpp"

using namespace chess;

TEST_CASE("move_to_uci writes from, to and any promotion") {
  SECTION("an ordinary move") {
    REQUIRE(move_to_uci(
                Move{.from = square_at(4, 1), .to = square_at(4, 3), .promotion = std::nullopt}) ==
            "e2e4");
  }
  SECTION("each promotion piece as a lowercase letter") {
    const Square from = square_at(0, 6); // a7
    const Square to = square_at(0, 7);   // a8
    REQUIRE(move_to_uci(Move{.from = from, .to = to, .promotion = PieceType::Queen}) == "a7a8q");
    REQUIRE(move_to_uci(Move{.from = from, .to = to, .promotion = PieceType::Rook}) == "a7a8r");
    REQUIRE(move_to_uci(Move{.from = from, .to = to, .promotion = PieceType::Bishop}) == "a7a8b");
    REQUIRE(move_to_uci(Move{.from = from, .to = to, .promotion = PieceType::Knight}) == "a7a8n");
  }
}

TEST_CASE("move_from_uci parses well-formed moves") {
  SECTION("an ordinary move") {
    REQUIRE(move_from_uci("e2e4") ==
            Move{.from = square_at(4, 1), .to = square_at(4, 3), .promotion = std::nullopt});
  }
  SECTION("a promotion") {
    REQUIRE(move_from_uci("e7e8q") ==
            Move{.from = square_at(4, 6), .to = square_at(4, 7), .promotion = PieceType::Queen});
    REQUIRE(move_from_uci("a2a1n") ==
            Move{.from = square_at(0, 1), .to = square_at(0, 0), .promotion = PieceType::Knight});
  }
}

TEST_CASE("move_from_uci rejects malformed text") {
  for(const std::string_view text :
      {"", "e2", "e2e", "e2e4e5", "z2e4", "e2e9", "e7e8k", "e7e8Q", "E2E4"}) {
    CAPTURE(text);
    REQUIRE(move_from_uci(text) == std::nullopt);
  }
}

TEST_CASE("move_to_uci and move_from_uci invert each other") {
  for(const std::string_view text :
      {"e2e4", "g1f3", "e1g1", "h7h8q", "h7h8r", "h7h8b", "h7h8n", "b2a1n"}) {
    CAPTURE(text);
    const auto move = move_from_uci(text);
    REQUIRE(move.has_value());
    // The `if` is for clang-tidy, which can't see that REQUIRE stops the test on failure.
    if(move) REQUIRE(move_to_uci(*move) == text);
  }
}
