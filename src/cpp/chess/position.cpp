#include "chess/position.hpp"

#include <cstddef>
#include <format>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "chess/types.hpp"

namespace chess {

namespace {

constexpr std::size_t FEN_FIELD_COUNT = 6;
constexpr std::size_t EPD_FIELD_COUNT = 4;

constexpr std::size_t PLACEMENT = 0;
constexpr std::size_t SIDE_TO_MOVE = 1;
constexpr std::size_t CASTLING = 2;
constexpr std::size_t EN_PASSANT = 3;
constexpr std::size_t HALFMOVE_CLOCK = 4;
constexpr std::size_t FULLMOVE_NUMBER = 5;

std::optional<PieceType> type_from_fen_char(char c) {
  switch(c) {
    case 'p':
    case 'P':
      return PieceType::Pawn;
    case 'n':
    case 'N':
      return PieceType::Knight;
    case 'b':
    case 'B':
      return PieceType::Bishop;
    case 'r':
    case 'R':
      return PieceType::Rook;
    case 'q':
    case 'Q':
      return PieceType::Queen;
    case 'k':
    case 'K':
      return PieceType::King;
    default:
      return std::nullopt;
  }
}

// Uppercase is White, lowercase is Black.
std::optional<Piece> piece_from_fen_char(char c) {
  const auto type = type_from_fen_char(c);
  if(!type) return std::nullopt;

  const Colour colour = (c >= 'A' && c <= 'Z') ? Colour::White : Colour::Black;
  return Piece{.colour = colour, .type = *type};
}

// Splits on single spaces. The views point into `text`, so they are only valid while it is.
std::vector<std::string_view> split_fields(std::string_view text) {
  std::vector<std::string_view> fields;
  std::size_t start = 0;

  while(true) {
    const std::size_t end = text.find(' ', start);
    if(end == std::string_view::npos) {
      fields.push_back(text.substr(start));
      return fields;
    }
    fields.push_back(text.substr(start, end - start));
    start = end + 1;
  }
}

// Parses the first FEN field (piece placement) into a board.
Board parse_placement(std::string_view placement) {
  Board board{};
  int row = BOARD_SIZE - 1;
  int col = 0;

  for(const char c : placement) {
    if(c == '/') {
      if(col != BOARD_SIZE) {
        throw std::invalid_argument(
            std::format("FEN rank {} has {} files, expected 8", row + 1, col));
      }
      --row;
      col = 0;
      if(row < 0) throw std::invalid_argument("FEN has more than 8 ranks");
    } else if(c >= '1' && c <= '8') {
      col += c - '0';
      if(col > BOARD_SIZE)
        throw std::invalid_argument(std::format("FEN rank {} has more than 8 files", row + 1));
    } else if(const auto piece = piece_from_fen_char(c)) {
      if(col >= BOARD_SIZE)
        throw std::invalid_argument(std::format("FEN rank {} has more than 8 files", row + 1));
      board.at(static_cast<std::size_t>(square_at(col, row))) = piece;
      ++col;
    } else {
      throw std::invalid_argument(std::format("invalid character '{}' in FEN", c));
    }
  }
  if(row != 0 || col != BOARD_SIZE)
    throw std::invalid_argument("FEN placement is incomplete: expected 8 ranks of 8 files");
  return board;
}

// Parses the second FEN field: "w" or "b"
Colour parse_side(std::string_view field) {
  if(field == "w") return Colour::White;
  if(field == "b") return Colour::Black;
  throw std::invalid_argument(std::format("FEN side to move must be 'w' or 'b', got '{}'", field));
}
} // namespace

Position Position::from_fen(std::string_view fen) {
  const auto fields = split_fields(fen);
  if(fields.size() != FEN_FIELD_COUNT && fields.size() != EPD_FIELD_COUNT) {
    throw std::invalid_argument(std::format("FEN has {} fields, expected {} or {}", fields.size(),
                                            EPD_FIELD_COUNT, FEN_FIELD_COUNT));
  }
  Position position;
  position.board_ = parse_placement(fields[PLACEMENT]);
  position.side_to_move_ = parse_side(fields[SIDE_TO_MOVE]);
  return position;
}

std::optional<Piece> Position::piece_at(Square square) const {
  return board_.at(static_cast<std::size_t>(square));
}

Colour Position::side_to_move() const { return side_to_move_; }

} // namespace chess
