#pragma once

#include <webview/webview.h>

namespace pcr {

// Binds every C++ function the page can call (window.position, window.play, ...).
// Each replies with {"ok": true, "value": ...} or {"ok": false, "error": "..."}.
void register_bindings(webview::webview& window);

} // namespace pcr
