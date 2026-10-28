#pragma once

#include <optional>
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

private:
  Position() = default;
  Board board_{};
  Colour side_to_move_{Colour::White};
  CastlingRights castling_{};
};

} // namespace chess
