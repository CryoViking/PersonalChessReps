#include <exception>
#include <iostream>
#include <webview/webview.h>

constexpr int SCREEN_WIDTH = 1024;
constexpr int SCREEN_HEIGHT = 768;

int main() {
  try {
    webview::webview window(true, nullptr);
    window.set_title("Personal Chess Reps");
    window.set_size(SCREEN_WIDTH, SCREEN_HEIGHT, WEBVIEW_HINT_NONE);
    window.set_html("<h1>Hello from Personal Chess Reps</h1>");
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
