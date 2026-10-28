#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace chess {
using Square = int;
constexpr int BOARD_SIZE = 8;
constexpr int SQUARE_COUNT = BOARD_SIZE * BOARD_SIZE; // 64; for array sizes and loops

// Same convention as chess.ts: "row" is the rank and "col" the file, both 0-7.
// row 0 is rank 1 (White's back rank), col 0 is the a-file.
// square = row * 8 + col, so a1 = 0, h1 =7, a8 = 56 and h8 = 63.
constexpr Square square_at(int col, int row) { return (row * BOARD_SIZE) + col; }
constexpr int col_of(Square square) { return square % BOARD_SIZE; }
constexpr int row_of(Square square) { return square / BOARD_SIZE; }

// "e3" -> square_at(4, 2). Returns the nullopt for anything that isn't a1-h8.
constexpr std::optional<Square> square_from_name(std::string_view name) {
  if(name.size() != 2) return std::nullopt;

  const char col = name[0];
  const char row = name[1];
  if(col < 'a' || col > 'h' || row < '1' || row > '8') return std::nullopt;

  return square_at(col - 'a', row - '1');
}

enum class Colour : std::uint8_t { White, Black };
enum class PieceType : std::uint8_t { Pawn, Knight, Bishop, Rook, Queen, King };

struct Piece {
  Colour colour;
  PieceType type;

  bool operator==(const Piece&) const = default;
};

using Board = std::array<std::optional<Piece>, SQUARE_COUNT>;
} // namespace chess
