#include "chess/uci.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

#include "chess/types.hpp"

namespace chess {

namespace {

constexpr std::size_t SQUARE_NAME_LENGTH = 2;
constexpr std::size_t UCI_MOVE_LENGTH = 4;           // "e2e4"
constexpr std::size_t UCI_PROMOTION_MOVE_LENGTH = 5; // "e7e8q"

std::optional<PieceType> promotion_from_uci(char letter) {
  switch(letter) {
    case 'n':
      return PieceType::Knight;
    case 'b':
      return PieceType::Bishop;
    case 'r':
      return PieceType::Rook;
    case 'q':
      return PieceType::Queen;
    default:
      return std::nullopt;
  }
}

} // namespace

std::string move_to_uci(const Move& move) {
  std::string uci = square_name(move.from) + square_name(move.to);
  if(move.promotion) {
    switch(*move.promotion) {
      case PieceType::Knight:
        uci += 'n';
        break;
      case PieceType::Bishop:
        uci += 'b';
        break;
      case PieceType::Rook:
        uci += 'r';
        break;
      case PieceType::Queen:
        uci += 'q';
        break;
      case PieceType::Pawn:
      case PieceType::King:
        break;
    }
  }
  return uci;
}

std::optional<Move> move_from_uci(std::string_view text) {
  if(text.size() != UCI_MOVE_LENGTH && text.size() != UCI_PROMOTION_MOVE_LENGTH)
    return std::nullopt;

  const auto from = square_from_name(text.substr(0, SQUARE_NAME_LENGTH));
  const auto to = square_from_name(text.substr(SQUARE_NAME_LENGTH, SQUARE_NAME_LENGTH));
  if(!from || !to) return std::nullopt;

  std::optional<PieceType> promotion;
  if(text.size() == UCI_PROMOTION_MOVE_LENGTH) {
    promotion = promotion_from_uci(text.back());
    if(!promotion) return std::nullopt;
  }
  return Move{.from = *from, .to = *to, .promotion = promotion};
}

} // namespace chess
