#include "chess/position.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdlib>
#include <format>
#include <initializer_list>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "chess/types.hpp"

namespace chess {

namespace {

struct Offset {
  int dc;
  int dr;
};

struct CastlingSide {
  int rook_col;                    // Where the rook must be
  std::span<const int> empty_cols; // squares between king and rook
  std::span<const int> safe_cols;  // squares the king stands on, crosses or lands on
  int king_to_col;
};

constexpr std::size_t FEN_FIELD_COUNT = 6;
constexpr std::size_t EPD_FIELD_COUNT = 4;

constexpr std::size_t PLACEMENT = 0;
constexpr std::size_t SIDE_TO_MOVE = 1;
constexpr std::size_t CASTLING = 2;
constexpr std::size_t EN_PASSANT = 3;
constexpr std::size_t HALFMOVE_CLOCK = 4;
constexpr std::size_t FULLMOVE_NUMBER = 5;

constexpr int WHITE_EN_PASSANT_ROW = 2;
constexpr int BLACK_EN_PASSANT_ROW = 5;
constexpr int MIN_HALFMOVE_CLOCK = 0;
constexpr int MIN_FULLMOVE_NUMBER = 1;

// Rook home squares: anything moving from or to one of these loses that castling again.
constexpr Square WHITE_KINGSIDE_ROOK = square_at(7, 0);  // h1
constexpr Square WHITE_QUEENSIDE_ROOK = square_at(0, 0); // a1
constexpr Square BLACK_KINGSIDE_ROOK = square_at(7, 7);  // h8
constexpr Square BLACK_QUEENSIDE_ROOK = square_at(0, 7); // a8

// Where the rook goes when castling: h-file to f-file, or a-file to d-file.
constexpr int KINGSIDE_ROOK_FROM_COL = 7;
constexpr int KINGSIDE_ROOK_TO_COL = 5;
constexpr int QUEENSIDE_ROOK_FROM_COL = 0;
constexpr int QUEENSIDE_ROOK_TO_COL = 3;

constexpr std::array<int, 2> KINGSIDE_EMPTY_COLS{
    5,
    6,
}; // f, g
constexpr std::array<int, 3> KINGSIDE_SAFE_COLS{
    4,
    5,
    6,
}; // e, f, g
constexpr std::array<int, 3> QUEENSIDE_EMPTY_COLS{
    3,
    2,
    1,
}; // d, c, b
constexpr std::array<int, 3> QUEENSIDE_SAFE_COLS{
    4,
    3,
    2,
}; // e, d, c (not b: the king never crosses it)

constexpr CastlingSide KINGSIDE{
    .rook_col = 7,
    .empty_cols = KINGSIDE_EMPTY_COLS,
    .safe_cols = KINGSIDE_SAFE_COLS,
    .king_to_col = 6,
};
constexpr CastlingSide QUEENSIDE{
    .rook_col = 0,
    .empty_cols = QUEENSIDE_EMPTY_COLS,
    .safe_cols = QUEENSIDE_SAFE_COLS,
    .king_to_col = 2,
};

constexpr int CASTLING_KING_STEP = 2; // columns the king moves when castling
constexpr int DOUBLE_PUSH_STEP = 2;   // rows a pawn moves on its first double step

constexpr int WHITE_PAWN_START_ROW = 1; // rank 2
constexpr int BLACK_PAWN_START_ROW = 6; // rank 7
constexpr int WHITE_PROMOTION_ROW = 7;  // rank 8
constexpr int BLACK_PROMOTION_ROW = 0;  // rank 9

constexpr int KING_HOME_COL = 4; // e-file
constexpr int WHITE_HOME_ROW = 0;
constexpr int BLACK_HOME_ROW = 7;

constexpr std::array<PieceType, 4> PROMOTION_TYPES{
    PieceType::Queen,
    PieceType::Rook,
    PieceType::Bishop,
    PieceType::Knight,
};

constexpr std::array<Offset, 8> KNIGHT_OFFSETS{
    {
        {.dc = 1, .dr = 2},
        {.dc = 2, .dr = 1},
        {.dc = 2, .dr = -1},
        {.dc = 1, .dr = -2},
        {.dc = -1, .dr = -2},
        {.dc = -2, .dr = -1},
        {.dc = -2, .dr = 1},
        {.dc = -1, .dr = 2},
    },
};

constexpr std::array<Offset, 8> KING_OFFSETS{
    {
        {.dc = -1, .dr = -1},
        {.dc = 0, .dr = -1},
        {.dc = 1, .dr = -1},
        {.dc = -1, .dr = 0},
        {.dc = 1, .dr = 0},
        {.dc = -1, .dr = 1},
        {.dc = 0, .dr = 1},
        {.dc = 1, .dr = 1},
    },
};

constexpr std::array<Offset, 4> DIAGONALS{
    {
        {.dc = 1, .dr = 1},
        {.dc = 1, .dr = -1},
        {.dc = -1, .dr = -1},
        {.dc = -1, .dr = 1},
    },
};

constexpr std::array<Offset, 4> STRAIGHTS{
    {
        {.dc = 1, .dr = 0},
        {.dc = -1, .dr = 0},
        {.dc = 0, .dr = 1},
        {.dc = 0, .dr = -1},
    },
};

// One step from `square`, or nullopt if that would leave the board.
std::optional<Square> offset_square(Square square, Offset offset) {
  const int col = col_of(square) + offset.dc;
  const int row = row_of(square) + offset.dr;
  if(col < 0 || col >= BOARD_SIZE || row < 0 || row >= BOARD_SIZE) return std::nullopt;
  return square_at(col, row);
}

// Does a piece equal to `attacker` sit one offset away from `square`?
bool leaper_attacks(const Position& position, Square square, std::span<const Offset> offsets,
                    Piece attacker) {
  return std::ranges::any_of(offsets, [&](Offset offset) {
    const auto target = offset_square(square, offset);
    return target && position.piece_at(*target) == attacker;
  });
}

// Walks each direction until the first piece; true if it's `by`'s `type` or queen.
bool slider_attacks(const Position& position, Square square, std::span<const Offset> directions,
                    Colour by, PieceType type) {
  for(const Offset direction : directions) {
    auto current = offset_square(square, direction);
    while(current) {
      if(const auto piece = position.piece_at(*current)) {
        if(piece->colour == by && (piece->type == type || piece->type == PieceType::Queen)) {
          return true;
        }
        break;
      }
      current = offset_square(*current, direction);
    }
  }
  return false;
}

void add_leaper_moves(const Position& position, Square from, std::span<const Offset> offsets,
                      Colour us, std::vector<Move>& moves) {
  for(const Offset offset : offsets) {
    const auto target = offset_square(from, offset);
    if(!target) continue;
    const auto occupant = position.piece_at(*target);
    if(occupant && occupant->colour == us) continue;
    moves.push_back(Move{.from = from, .to = *target, .promotion = std::nullopt});
  }
}

// Walks each direction, adding every empty square and the first enemy piece.
// Stops at the board edge of the first piece either way.
void add_slider_moves(const Position& position, Square from, std::span<const Offset> directions,
                      Colour us, std::vector<Move>& moves) {
  for(const Offset direction : directions) {
    auto current = offset_square(from, direction);
    while(current) {
      const auto occupant = position.piece_at(*current);
      if(occupant && occupant->colour == us) break;

      moves.push_back(Move{.from = from, .to = *current, .promotion = std::nullopt});
      if(occupant) break;
      current = offset_square(*current, direction);
    }
  }
}

// Adds `move`, or one move per promotion piece if it lands on the promotion row.
void add_pawn_move(const Move& move, int promotion_row, std::vector<Move>& moves) {
  if(row_of(move.to) != promotion_row) {
    moves.push_back(move);
    return;
  }
  for(const PieceType type : PROMOTION_TYPES) {
    moves.push_back(Move{.from = move.from, .to = move.to, .promotion = type});
  }
}

// Pushes (one or two squares straight ahead onot empty squares) and diagonal captures
// (onto) an enemy piece or the en passant square).
void add_pawn_moves(const Position& position, Square from, Colour us, std::vector<Move>& moves) {
  const bool white = us == Colour::White;
  const int forward = white ? 1 : -1;
  const int start_row = white ? WHITE_PAWN_START_ROW : BLACK_PAWN_START_ROW;
  const int promotion_row = white ? WHITE_PROMOTION_ROW : BLACK_PROMOTION_ROW;

  const auto one_step = offset_square(from, Offset{.dc = 0, .dr = forward});
  if(one_step && !position.piece_at(*one_step)) {
    add_pawn_move(Move{.from = from, .to = *one_step, .promotion = std::nullopt}, promotion_row,
                  moves);

    const auto two_steps = offset_square(*one_step, Offset{.dc = 0, .dr = forward});
    if(row_of(from) == start_row && two_steps && !position.piece_at(*two_steps))
      moves.push_back(Move{.from = from, .to = *two_steps, .promotion = std::nullopt});
  }

  for(const int dc : {-1, 1}) {
    const auto target = offset_square(from, Offset{.dc = dc, .dr = forward});
    if(!target) continue;

    const auto occupant = position.piece_at(*target);
    const bool captures_enemy = occupant && occupant->colour != us;
    if(captures_enemy || position.en_passant() == *target) {
      add_pawn_move(Move{.from = from, .to = *target, .promotion = std::nullopt}, promotion_row,
                    moves);
    }
  }
}

// Adds the castling move for one side if every condition holds. The right itself is
// checked by the caller.
void add_castling_move(const Position& position, const CastlingSide& side,
                       std::vector<Move>& moves) {
  const Colour us = position.side_to_move();
  const Colour them = opposite(us);
  const int row = us == Colour::White ? WHITE_HOME_ROW : BLACK_HOME_ROW;
  const Square king_from = square_at(KING_HOME_COL, row);

  // Guard against FENs whose rights don't match the board.
  if(position.piece_at(king_from) != Piece{.colour = us, .type = PieceType::King}) return;
  if(position.piece_at(square_at(side.rook_col, row)) !=
     Piece{.colour = us, .type = PieceType::Rook}) {
    return;
  }

  const auto occupied = [&](int col) { return position.piece_at(square_at(col, row)).has_value(); };
  const auto attacked = [&](int col) {
    return position.is_square_attacked(square_at(col, row), them);
  };
  if(std::ranges::any_of(side.empty_cols, occupied)) return;
  if(std::ranges::any_of(side.safe_cols, attacked)) return;

  moves.push_back(
      Move{.from = king_from, .to = square_at(side.king_to_col, row), .promotion = std::nullopt});
}

void add_castling_moves(const Position& position, std::vector<Move>& moves) {
  const CastlingRights rights = position.castling();
  const bool white = position.side_to_move() == Colour::White;

  if(white ? rights.white_kingside : rights.black_kingside)
    add_castling_move(position, KINGSIDE, moves);

  if(white ? rights.white_queenside : rights.black_queenside)
    add_castling_move(position, QUEENSIDE, moves);
}

std::size_t index_of(Square square) { return static_cast<std::size_t>(square); }

void clear_corner_right(CastlingRights& rights, Square square) {
  if(square == WHITE_KINGSIDE_ROOK) rights.white_kingside = false;
  if(square == WHITE_QUEENSIDE_ROOK) rights.white_queenside = false;
  if(square == BLACK_KINGSIDE_ROOK) rights.black_kingside = false;
  if(square == BLACK_QUEENSIDE_ROOK) rights.black_queenside = false;
}

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

// The inverse of piece_from_fen_char: uppercase for White, lowercase for Black.
char fen_char(Piece piece) {
  char letter = 'p';
  switch(piece.type) {
    case PieceType::Pawn:
      letter = 'p';
      break;
    case PieceType::Knight:
      letter = 'n';
      break;
    case PieceType::Bishop:
      letter = 'b';
      break;
    case PieceType::Rook:
      letter = 'r';
      break;
    case PieceType::Queen:
      letter = 'q';
      break;
    case PieceType::King:
      letter = 'k';
      break;
  }
  return piece.colour == Colour::White ? static_cast<char>(letter - 'a' + 'A') : letter;
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

// The inverse of parse_placement: rank 8 first, runs of empty squares as digits.
std::string placement_to_fen(const Board& board) {
  std::string result;

  for(int row = BOARD_SIZE - 1; row >= 0; --row) {
    int empty = 0;
    for(int col = 0; col < BOARD_SIZE; ++col) {
      const auto& piece = board.at(static_cast<std::size_t>(square_at(col, row)));
      if(piece) {
        if(empty > 0) {
          result += static_cast<char>('0' + empty);
          empty = 0;
        }
        result += fen_char(*piece);
      } else {
        ++empty;
      }
    }
    if(empty > 0) result += static_cast<char>('0' + empty);
    if(row > 0) result += '/';
  }

  return result;
}

// The inverse of parse_castling. Always KQkq order, so the output is canonical.
std::string castling_to_fen(const CastlingRights& rights) {
  std::string result;
  if(rights.white_kingside) result += 'K';
  if(rights.white_queenside) result += 'Q';
  if(rights.black_kingside) result += 'k';
  if(rights.black_queenside) result += 'q';
  return result.empty() ? "-" : result;
}

// Parses the second FEN field: "w" or "b"
Colour parse_side(std::string_view field) {
  if(field == "w") return Colour::White;
  if(field == "b") return Colour::Black;
  throw std::invalid_argument(std::format("FEN side to move must be 'w' or 'b', got '{}'", field));
}

// Parses the third FEN field: "-" or any of K, Q, k, q (each at most once).
CastlingRights parse_castling(std::string_view field) {
  CastlingRights rights;
  if(field == "-") return rights;
  if(field.empty()) throw std::invalid_argument("FEN castling field is empty");

  for(const char c : field) {
    bool* flag = nullptr;
    switch(c) {
      case 'K':
        flag = &rights.white_kingside;
        break;
      case 'k':
        flag = &rights.black_kingside;
        break;
      case 'Q':
        flag = &rights.white_queenside;
        break;
      case 'q':
        flag = &rights.black_queenside;
        break;
      default:
        throw std::invalid_argument(std::format("invalid castling character '{}' in FEN", c));
    }

    if(*flag)
      throw std::invalid_argument(std::format("repeated castling character '{}' in FEN", c));
    *flag = true;
  }
  return rights;
}

std::optional<Square> parse_en_passant(std::string_view field) {
  if(field == "-") return std::nullopt;

  const auto square = square_from_name(field);
  if(!square)
    throw std::invalid_argument(std::format("invalid en passant square '{}' in FEND", field));

  const int row = row_of(*square);
  if(row != WHITE_EN_PASSANT_ROW && row != BLACK_EN_PASSANT_ROW) {
    throw std::invalid_argument(
        std::format("en passant square '{}' must be on rank 3 or 6", field));
  }
  return square;
}

// Parses a FEN move counter: a whole number, with no sign or trailing characters.
// `name` is only for the error message.
int parse_counter(std::string_view field, std::string_view name, int minimum) {
  int value = 0;
  const char* const end = field.data() + field.size();
  // NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage): bounded by end
  const auto [ptr, ec] = std::from_chars(field.data(), end, value);

  if(ec != std::errc{} || ptr != end)
    throw std::invalid_argument(std::format("FEN {} '{}' is not a whole number", name, field));
  if(value < minimum) {
    throw std::invalid_argument(
        std::format("FEN {} must be at least {}, got {}", name, minimum, value));
  }
  return value;
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
  position.castling_ = parse_castling(fields[CASTLING]);
  position.en_passant_ = parse_en_passant(fields[EN_PASSANT]);
  if(fields.size() == FEN_FIELD_COUNT) {
    position.halfmove_clock_ =
        parse_counter(fields[HALFMOVE_CLOCK], "halfmove clock", MIN_HALFMOVE_CLOCK);
    position.fullmove_number_ =
        parse_counter(fields[FULLMOVE_NUMBER], "fullmove number", MIN_FULLMOVE_NUMBER);
  }
  return position;
}

std::string Position::to_fen() const {
  return std::format("{} {} {} {} {} {}", placement_to_fen(board_),
                     side_to_move_ == Colour::White ? "w" : "b", castling_to_fen(castling_),
                     en_passant_ ? square_name(*en_passant_) : "-", halfmove_clock_,
                     fullmove_number_);
}

std::optional<Piece> Position::piece_at(Square square) const {
  return board_.at(static_cast<std::size_t>(square));
}
Colour Position::side_to_move() const { return side_to_move_; }
CastlingRights Position::castling() const { return castling_; }
std::optional<Square> Position::en_passant() const { return en_passant_; }
int Position::halfmove_clock() const { return halfmove_clock_; }
int Position::fullmove_number() const { return fullmove_number_; }

Position Position::make_move(const Move& move) const {
  const auto moving = board_.at(index_of(move.from));
  if(!moving)
    throw std::invalid_argument(std::format("no piece on {} to move", square_name(move.from)));

  const Piece piece = *moving;
  const bool is_pawn = piece.type == PieceType::Pawn;
  const bool is_capture = board_.at(index_of(move.to)).has_value();

  Position next = *this;

  // En passant: the captured pawn is beside the destination, not on it.
  if(is_pawn && en_passant_ == move.to)
    next.board_.at(index_of(square_at(col_of(move.to), row_of(move.from)))) = std::nullopt;

  next.board_.at(index_of(move.to)) =
      move.promotion ? Piece{.colour = piece.colour, .type = *move.promotion} : piece;
  next.board_.at(index_of(move.from)) = std::nullopt;

  // Castling: the king moves two columns, and the rook jumps to its other side.
  const int col_change = col_of(move.to) - col_of(move.from);
  if(piece.type == PieceType::King && std::abs(col_change) == CASTLING_KING_STEP) {
    const int row = row_of(move.from);
    const bool kingside = col_change > 0;
    const Square rook_from =
        square_at(kingside ? KINGSIDE_ROOK_FROM_COL : QUEENSIDE_ROOK_FROM_COL, row);
    const Square rook_to = square_at(kingside ? KINGSIDE_ROOK_TO_COL : QUEENSIDE_ROOK_TO_COL, row);
    next.board_.at(index_of(rook_to)) = next.board_.at(index_of(rook_from));
    next.board_.at(index_of(rook_from)) = std::nullopt;
  }

  if(piece.type == PieceType::King) {
    if(piece.colour == Colour::White) {
      next.castling_.white_kingside = false;
      next.castling_.white_queenside = false;
    } else {
      next.castling_.black_kingside = false;
      next.castling_.black_queenside = false;
    }
  }
  clear_corner_right(next.castling_, move.from);
  clear_corner_right(next.castling_, move.to);

  next.en_passant_ = std::nullopt;
  const int row_change = row_of(move.to) - row_of(move.from);
  if(is_pawn && std::abs(row_change) == DOUBLE_PUSH_STEP)
    next.en_passant_ = square_at(col_of(move.from), (row_of(move.from) + row_of(move.to)) / 2);

  next.halfmove_clock_ = (is_pawn || is_capture) ? 0 : halfmove_clock_ + 1;
  if(side_to_move_ == Colour::Black) ++next.fullmove_number_;
  next.side_to_move_ = opposite(side_to_move_);
  return next;
}

bool Position::is_square_attacked(Square square, Colour by) const {
  if(leaper_attacks(*this, square, KNIGHT_OFFSETS, Piece{.colour = by, .type = PieceType::Knight}))
    return true;
  if(leaper_attacks(*this, square, KING_OFFSETS, Piece{.colour = by, .type = PieceType::King}))
    return true;

  // Look back the way an attacking a pawn would have come: White pawns attack upwards,
  // so a White attacker sites one row below the square.
  const int pawn_dr = by == Colour::White ? -1 : 1;
  const std::array<Offset, 2> pawn_offsets{
      {
          {.dc = -1, .dr = pawn_dr},
          {.dc = 1, .dr = pawn_dr},
      },
  };
  if(leaper_attacks(*this, square, pawn_offsets, Piece{.colour = by, .type = PieceType::Pawn}))
    return true;

  return slider_attacks(*this, square, DIAGONALS, by, PieceType::Bishop) ||
         slider_attacks(*this, square, STRAIGHTS, by, PieceType::Rook);
}

std::vector<Move> Position::legal_moves() const {
  std::vector<Move> moves;
  for(Square from = 0; from < SQUARE_COUNT; ++from) {
    const auto piece = piece_at(from);
    if(!piece || piece->colour != side_to_move_) continue;

    switch(piece->type) {
      case PieceType::Pawn:
        add_pawn_moves(*this, from, side_to_move_, moves);
        break;
      case PieceType::Knight:
        add_leaper_moves(*this, from, KNIGHT_OFFSETS, side_to_move_, moves);
        break;
      case PieceType::Bishop:
        add_slider_moves(*this, from, DIAGONALS, side_to_move_, moves);
        break;
      case PieceType::Rook:
        add_slider_moves(*this, from, STRAIGHTS, side_to_move_, moves);
        break;
      case PieceType::Queen:
        add_slider_moves(*this, from, DIAGONALS, side_to_move_, moves);
        add_slider_moves(*this, from, STRAIGHTS, side_to_move_, moves);
        break;
      case PieceType::King:
        add_leaper_moves(*this, from, KING_OFFSETS, side_to_move_, moves);
        add_castling_moves(*this, moves);
        break;
    }
  }
  // Keep only moves that don't leave our king attacked. That one rule covers pins,
  // moving into check and failing to get out of check.
  std::erase_if(moves, [&](const Move& move) {
    const Position next = make_move(move);
    const auto king = next.king_square(side_to_move_);
    return king && next.is_square_attacked(*king, opposite(side_to_move_));
  });
  return moves;
}

std::optional<Square> Position::king_square(Colour colour) const {
  for(Square square = 0; square < SQUARE_COUNT; ++square)
    if(piece_at(square) == Piece{.colour = colour, .type = PieceType::King}) return square;
  return std::nullopt;
}

bool Position::in_check() const {
  const auto king = king_square(side_to_move_);
  return king && is_square_attacked(*king, opposite(side_to_move_));
}

} // namespace chess
