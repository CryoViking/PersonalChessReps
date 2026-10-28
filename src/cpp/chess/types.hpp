#pragma once

#include <cstdint>

namespace chess {
enum class Colour : std::uint8_t { White, Black };
enum class PieceType : std::uint8_t { Pawn, Knight, Bishop, Rook, Queen, King };

struct Piece {
  Colour colour;
  PieceType type;

  bool operator==(const Piece&) const = default;
};

// Same convention as chess.ts: "row" is the rank and "col" the file, both 0-7.
// row 0 is rank 1 (White's back ranke), col 0 is the a-file.
// square = row * 8 + col, so a1 = 0, h1 =7, a8 = 56 and h8 = 63.
using Square = int;

constexpr int BOARD_SIZE = 8;

constexpr Square square_at(int col, int row) { return (row * BOARD_SIZE) + col; }
constexpr int col_of(Square square) { return square % BOARD_SIZE; }
constexpr int row_of(Square square) { return square / BOARD_SIZE; }
} // namespace chess
