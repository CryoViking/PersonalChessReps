#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <webview/webview.h>

using Json = nlohmann::json;

constexpr int SCREEN_WIDTH = 1024;
constexpr int SCREEN_HEIGHT = 768;

namespace {
[[nodiscard]] std::string read_file(const std::filesystem::path& path) {
  const std::ifstream file(path, std::ios::binary);
  if(!file) {
    throw std::runtime_error("could not open " + path.string());
  }

  std::ostringstream contents;
  contents << file.rdbuf(); // read the buffer
  return contents.str();
}
} // namespace

int main() {
  try {
    webview::webview window(true, nullptr);
    window.set_title("Personal Chess Reps");
    window.set_size(SCREEN_WIDTH, SCREEN_HEIGHT, WEBVIEW_HINT_NONE);

    window.bind("ping", [](const std::string& args) -> std::string {
      try {
        const auto json = Json::parse(args);
        const auto message = json.at(0).get<std::string>();
        if(message == "error") {
          throw std::runtime_error("ping was told to fail");
        }
        return Json{{"ok", true}, {"value", "pong: " + message}}.dump();
      } catch(const std::exception& e) {
        std::cerr << "ping error: " << e.what() << '\n';
        const Json reply{{"ok", false}, {"error", e.what()}};
        return reply.dump(-1, ' ', false, Json::error_handler_t::replace);
      } catch(...) {
        std::cerr << "ping error: unknown error\n";
        return R"({"ok": false, "error": "unknown error"})";
      }
    });

    // NOLINTNEXTLINE(concurrency-mt-unsafe): This is to be expected at the moment
    const auto* const url = std::getenv("PCR_DEV_URL");
    if(url != nullptr) window.navigate(url);
    else window.set_html(read_file(std::filesystem::path{PCR_WEB_DIST} / "index.html"));

    window.run();
  } catch(const webview::exception& e) {
    std::cerr << "webview error: " << e.what() << '\n';
    return 1;
  } catch(const std::exception& e) {
    std::cerr << "std error: " << e.what() << '\n';
    return 1;
  } catch(...) {
    std::cerr << "unknown error\n";
    return 1;
  }
  return 0;
}
