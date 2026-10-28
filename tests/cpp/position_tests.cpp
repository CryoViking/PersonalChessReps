#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <optional>
#include <stdexcept>
#include <string>
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

TEST_CASE("from_fen reads the side to move") {
  REQUIRE(Position::from_fen(START_FEN).side_to_move() == Colour::White);
  REQUIRE(Position::from_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1")
              .side_to_move() == Colour::Black);
}

TEST_CASE("from_fen rejects an invalid side to move") {
  REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 x - - 0 1"), std::invalid_argument);
  REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 white - - 0 1"), std::invalid_argument);
}

TEST_CASE("from_fen reads castling rights") {
  SECTION("all four in the starting position") {
    REQUIRE(Position::from_fen(START_FEN).castling() == CastlingRights{.white_kingside = true,
                                                                       .white_queenside = true,
                                                                       .black_kingside = true,
                                                                       .black_queenside = true});
  }
  SECTION("a subset") {
    REQUIRE(Position::from_fen("8/8/8/8/8/8/8/8 w Kq - 0 1").castling() ==
            CastlingRights{.white_kingside = true, .black_queenside = true});
  }
  SECTION("in any order") {
    REQUIRE(Position::from_fen("8/8/8/8/8/8/8/8 w qK - 0 1").castling() ==
            CastlingRights{.white_kingside = true, .black_queenside = true});
  }
  SECTION("none") {
    REQUIRE(Position::from_fen("8/8/8/8/8/8/8/8 w - - 0 1").castling() == CastlingRights{});
  }
}

TEST_CASE("from_fen rejects invalid castling rights") {
  SECTION("a repeated letter") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 w KK - 0 1"), std::invalid_argument);
  }
  SECTION("an invalid character") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 w Kx - 0 1"), std::invalid_argument);
  }
  SECTION("a dash mixed with letters") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 w -K - 0 1"), std::invalid_argument);
  }
}

TEST_CASE("from_fen reads the en passant square") {
  SECTION("none in the starting position") {
    REQUIRE(Position::from_fen(START_FEN).en_passant() == std::nullopt);
  }
  SECTION("rank 3 after a white double step") {
    REQUIRE(Position::from_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1")
                .en_passant() == square_at(4, 2));
  }
  SECTION("rank 6 after a black double step") {
    REQUIRE(Position::from_fen("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2")
                .en_passant() == square_at(2, 5));
  }
}

TEST_CASE("from_fen rejects an invalid en passant square") {
  SECTION("a square off rank 3 or 6") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 w - e4 0 1"), std::invalid_argument);
  }
  SECTION("an invalid file") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 w - z3 0 1"), std::invalid_argument);
  }
  SECTION("too short") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 w - e 0 1"), std::invalid_argument);
  }
  SECTION("too long") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 w - e33 0 1"), std::invalid_argument);
  }
}

TEST_CASE("from_fen reads the move counters") {
  SECTION("the starting values") {
    const auto position = Position::from_fen(START_FEN);
    REQUIRE(position.halfmove_clock() == 0);
    REQUIRE(position.fullmove_number() == 1);
  }
  SECTION("mid-game values") {
    const auto position = Position::from_fen("8/8/8/8/8/8/8/8 w - - 12 34");
    REQUIRE(position.halfmove_clock() == 12);
    REQUIRE(position.fullmove_number() == 34);
  }
  SECTION("defaults for a four-field EPD") {
    const auto position = Position::from_fen("8/8/8/8/8/8/8/8 w - -");
    REQUIRE(position.halfmove_clock() == 0);
    REQUIRE(position.fullmove_number() == 1);
  }
}

TEST_CASE("from_fen rejects invalid move counters") {
  SECTION("a negative halfmove clock") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 w - - -1 1"), std::invalid_argument);
  }
  SECTION("a halfmove clock that isn't a number") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 w - - abc 1"), std::invalid_argument);
  }
  SECTION("trailing characters after the number") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 w - - 12abc 1"), std::invalid_argument);
  }
  SECTION("a leading plus sign") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 w - - +1 1"), std::invalid_argument);
  }
  SECTION("a fullmove number of zero") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 w - - 0 0"), std::invalid_argument);
  }
  SECTION("a number too large for an int") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 w - - 0 99999999999"),
                      std::invalid_argument);
  }
}

TEST_CASE("from_fen rejects the wrong number of fields") {
  SECTION("five fields") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 w - - 0"), std::invalid_argument);
  }
  SECTION("seven fields") {
    REQUIRE_THROWS_AS(Position::from_fen("8/8/8/8/8/8/8/8 w - - 0 1 x"), std::invalid_argument);
  }
}

TEST_CASE("to_fen round-trips canonical FENs") {
  const std::string fen =
      GENERATE(as<std::string>{}, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
               "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
               "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",
               "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
               "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", "r3k2r/8/8/8/8/8/8/R3K2R b Kq - 3 17");

  REQUIRE(Position::from_fen(fen).to_fen() == fen);
}

TEST_CASE("to_fen writes canonical output") {
  SECTION("castling rights in KQkq order") {
    REQUIRE(Position::from_fen("8/8/8/8/8/8/8/8 w qK - 0 1").to_fen() ==
            "8/8/8/8/8/8/8/8 w Kq - 0 1");
  }
  SECTION("default move counters for a four-field EPD") {
    REQUIRE(Position::from_fen("8/8/8/8/8/8/8/8 w - -").to_fen() == "8/8/8/8/8/8/8/8 w - - 0 1");
  }
}
