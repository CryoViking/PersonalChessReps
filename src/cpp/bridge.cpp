#include "bridge.hpp"

#include <algorithm>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>
#include <webview/webview.h>

#include "chess/position.hpp"
#include "chess/types.hpp"
#include "chess/uci.hpp"

namespace pcr {

namespace {

using Json = nlohmann::json;

// `replace` swaps invalid UTF-8 for U+FFFD instead of throwing, so building an error
// reply can't itself fail.
std::string dump(const Json& json) {
  return json.dump(-1, ' ', false, Json::error_handler_t::replace);
}

// Wraps a handler (JSON argumemnts in, JSON value out, may throw) as a webview binding.
// Every reply is {"ok": true, "value"" ...} or {"ok": false, "error": "..."}; no
// exception ever escapes into webview
auto json_binding(std::string name, auto handler) {
  return [name = std::move(name), handler = std::move(handler)](const std::string& args) {
    try {
      return dump(Json{{"ok", true}, {"value", handler(Json::parse(args))}});
    } catch(const std::exception& e) {
      std::cerr << name << " error: " << e.what() << "\n";
      return dump(Json{{"ok", false}, {"error", e.what()}});
    } catch(...) {
      std::cerr << name << " error: unknown error\n";
      return std::string{R"({"ok": false, "error": "unknown error"})"};
    }
  };
}

// The shape every position reply shares.
Json position_state(const chess::Position& position) {
  std::vector<std::string> legal_moves;
  for(const chess::Move& move : position.legal_moves())
    legal_moves.push_back(chess::move_to_uci(move));
  return Json{
      {"fen", position.to_fen()},
      {"legalMoves", legal_moves},
      {"inCheck", position.in_check()},
  };
}

// position(fen)
Json handle_position(const Json& args) {
  return position_state(chess::Position::from_fen(args.at(0).get<std::string>()));
}

// play(fen, uci): only accepts a move that is legal in that position.
Json handle_play(const Json& args) {
  const auto fen = args.at(0).get<std::string>();
  const auto uci = args.at(1).get<std::string>();

  const auto move = chess::move_from_uci(uci);
  if(!move) throw std::invalid_argument("malformed move '" + uci + "'");

  const auto position = chess::Position::from_fen(fen);
  const auto legal = position.legal_moves();
  if(std::ranges::find(legal, *move) == legal.end())
    throw std::invalid_argument("illegal move " + uci + " in " + fen);
  return position_state(position.make_move(*move));
}

} // namespace

void register_bindings(webview::webview& window) {
  window.bind("position", json_binding("position", handle_position));
  window.bind("play", json_binding("play", handle_play));
}

} // namespace pcr
