#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "chess/types.hpp"

namespace chess {

struct CastlingRights {
  bool white_kingside = false;
  bool white_queenside = false;
  bool black_kingside = false;
  bool black_queenside = false;

  bool operator==(const CastlingRights&) const = default;
};

class Position {
public:
  // Parses a FEN string. Throws std::invalid_argument describing what is wrong.
  static Position from_fen(std::string_view fen);

  [[nodiscard]] std::optional<Piece> piece_at(Square square) const;
  [[nodiscard]] Colour side_to_move() const;
  [[nodiscard]] CastlingRights castling() const;
  [[nodiscard]] std::optional<Square> en_passant() const;
  [[nodiscard]] int halfmove_clock() const;
  [[nodiscard]] int fullmove_number() const;
  [[nodiscard]] std::string to_fen() const;

private:
  Position() = default;
  Board board_{};
  Colour side_to_move_{Colour::White};
  CastlingRights castling_{};
  std::optional<Square> en_passant_;
  int halfmove_clock_ = 0;
  int fullmove_number_ = 1;
};

} // namespace chess
