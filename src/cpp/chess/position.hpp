#pragma once

#include <optional>
#include <string_view>

#include "chess/types.hpp"

namespace chess {

class Position {
public:
  // Parses a FEN string. Throws std::invalid_argument describing what is wrong.
  static Position from_fen(std::string_view fen);

  [[nodiscard]] std::optional<Piece> piece_at(Square square) const;

private:
  Position() = default;

  Board board_{};
};

} // namespace chess
