#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <webview/webview.h>

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
    window.set_html(read_file(std::filesystem::path{PCR_WEB_DIR} / "index.html"));
    window.run();
  } catch(const webview::exception& e) {
    std::cerr << "webview error: " << e.what() << '\n';
    return 1;
  } catch(const std::exception& e) {
    std::cerr << "std error: " << e.what() << '\n';
    return 1;
  }
  return 0;
}
