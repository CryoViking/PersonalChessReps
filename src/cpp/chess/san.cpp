#include "chess/san.hpp"

#include <cstdlib>
#include <format>
#include <stdexcept>
#include <string>

#include "chess/position.hpp"
#include "chess/types.hpp"

namespace chess {

namespace {

constexpr int CASTLING_KING_STEP = 2; // columns the king moves when castling

char piece_letter(PieceType type) {
  switch(type) {
    case PieceType::Knight:
      return 'N';
    case PieceType::Bishop:
      return 'B';
    case PieceType::Rook:
      return 'R';
    case PieceType::Queen:
      return 'Q';
    case PieceType::King:
      return 'K';
    case PieceType::Pawn:
      break;
  }
  return '?'; // pawns have no letter; callers never ask for one
}

// Just enough of the origin square to tell `move` apart from any other legal move by the
// same kind of piece to the same square: nothing, the file, the rank, or both.
std::string disambiguation(const Position& position, const Move& move, PieceType type) {
  bool ambiguous = false;
  bool same_file = false;
  bool same_rank = false;

  for(const Move& other : position.legal_moves()) {
    if(other.to != move.to || other.from == move.from) continue;
    const auto other_piece = position.piece_at(other.from);
    if(!other_piece || other_piece->type != type) continue;

    ambiguous = true;
    if(col_of(other.from) == col_of(move.from)) same_file = true;
    if(row_of(other.from) == row_of(move.from)) same_rank = true;
  }

  std::string name = square_name(move.from); // "a1": file letter, then rank digit
  if(!ambiguous) return "";
  if(!same_file) return name.substr(0, 1);
  if(!same_rank) return name.substr(1, 1);
  return name;
}

} // namespace

std::string move_to_san(const Position& position, const Move& move) {
  const auto moving = position.piece_at(move.from);
  if(!moving) {
    throw std::invalid_argument(std::format("no piece on {} to move", square_name(move.from)));
  }
  const PieceType type = moving->type;

  std::string san;
  const int col_change = col_of(move.to) - col_of(move.from);
  if(type == PieceType::King && std::abs(col_change) == CASTLING_KING_STEP) {
    san = col_change > 0 ? "O-O" : "O-O-O";
  } else {
    const bool is_capture = position.piece_at(move.to).has_value() ||
                            (type == PieceType::Pawn && position.en_passant() == move.to);

    if(type == PieceType::Pawn) {
      if(is_capture) san += square_name(move.from).front(); // the pawn's file, as in "exd5"
    } else {
      san += piece_letter(type);
      san += disambiguation(position, move, type);
    }
    if(is_capture) san += 'x';
    san += square_name(move.to);
    if(move.promotion) {
      san += '=';
      san += piece_letter(*move.promotion);
    }
  }

  const Position next = position.make_move(move);
  if(next.in_check()) san += next.legal_moves().empty() ? '#' : '+';
  return san;
}

} // namespace chess
