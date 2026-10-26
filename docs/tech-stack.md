# Proposed technology stack

This document records the proposed stack for Personal Chess Reps. It is a
design baseline, not a promise that every library named here is already a
project dependency. Each non-trivial dependency should be evaluated in a small
spike before it becomes part of the shipped application.

## Constraints and goals

- The initial device is an Apple-silicon macOS laptop, with Linux as a future
  supported platform.
- The application is personal and local-first. A server, login system, and
  cloud account are explicitly out of scope for the first release.
- SQLite is the authoritative structured store. The filesystem holds immutable
  imports, optional analysis artefacts, exports, and backups.
- The user interface should be readily restyled with HTML, CSS, and
  TypeScript, while the chess and persistence logic remains C++.
- PGN and FEN are interoperability formats; the app must not invent a private
  format for data that chess tools already exchange successfully.

## Selected baseline

| Concern | Choice | Why |
| --- | --- | --- |
| Core language | C++20 | It is the language in which the application will primarily be authored and keeps the chess domain, storage, and engine integration in one native layer. |
| Build | CMake + Ninja | Cross-platform, IDE-friendly, and already present on the development machine. |
| Desktop host | [`webview/webview`](https://github.com/webview/webview) | A small C/C++ host with two-way JavaScript bindings. It uses the platform webview: WebKit on macOS and WebKitGTK on Linux, rather than bundling Chromium. |
| Frontend | TypeScript + Svelte, standard HTML and CSS | A compact component model with direct CSS control. The frontend is compiled to static assets and can be replaced without changing the chess core. |
| Database | SQLite | Excellent fit for a local, relational, query-heavy repertoire and review history. |
| Database access | Narrow C++ repository layer over SQLite | Keeps SQL, transactions, migrations, and schema invariants out of JavaScript. |
| Chess engine | Stockfish via UCI subprocess | Mature analysis without embedding engine code in the application process. |
| API documentation | Doxygen + Markdown | Generates C++ reference pages beside architecture and operational documentation. |
| Tests | Catch2 or GoogleTest, selected during the first implementation spike | Either supports portable CMake-based unit and integration tests; the choice should follow a short evaluation rather than habit. |

Svelte is a recommendation, not an architectural dependency. React, Vue, or a
small framework-free TypeScript UI would leave the native boundary unchanged.

## Application boundary

```text
HTML / CSS / TypeScript frontend
              |
       typed JSON command bridge
              |
C++ application services and chess core
  |-- SQLite repositories and migrations
  |-- PGN, FEN, SAN, move legality, position graph
  |-- spaced-repetition scheduling
  |-- import/export and backups
  `-- Stockfish UCI process controller
```

The webview is a presentation client, not a trusted operating-system shell.
JavaScript receives only named application commands such as `submitMove`,
`importPgn`, or `getPracticePrompt`. It must not receive arbitrary SQL,
filesystem access, shell execution, or an unrestricted native-object bridge.
Inputs crossing the bridge are validated in C++ and responses are serialised as
explicit data-transfer objects.

## Data layout

```text
Application Support/PersonalChessReps/
  user.sqlite             # migrations, repertoire, reviews, annotations
  library/
    imports/              # original PGNs, content-addressed where practical
    analysis/             # optional large engine-output artefacts
    exports/
    backups/
```

Board positions are small, relational, and frequently queried; they belong in
SQLite, not as individual files. Store a canonical position key based on piece
placement, side to move, castling rights, and a legally meaningful en-passant
state. Store move edges separately so transpositions converge on a single
position. Repertoire choices and review cards remain separate layers over that
general chess graph.

## Frontend strategy

The board, move tree, filters, keyboard shortcuts, drill feedback, animation,
and theme belong in the frontend. Use SVG pieces, CSS custom properties, and a
small design-token set rather than native rendering APIs. This is more than
adequate for a chess board and preserves the freedom to reshape the visual
language later.

Frontend assets should be bundled into the application, loaded from an
application-controlled origin, and treated as static local content. Do not load
remote pages into the privileged webview.

## Platform considerations

The macOS build uses the system WebKit framework supplied by Xcode/macOS. A
future Linux build requires GTK and WebKitGTK development/runtime packages and
should be tested on an actual supported Linux distribution or CI runner.

Neither Metal nor a bundled Chromium runtime is warranted for the first
release. The former creates a platform-specific rendering path for a trivial
graphics workload; the latter imposes Electron-like distribution weight without
advancing the chess domain.

## Documentation workflow

Doxygen is configured through `Doxyfile.in`, which CMake expands into the build
directory. Public C++ declarations will use Doxygen comments; this `docs/`
directory supplies the long-form Markdown pages.

```sh
cmake --preset dev
cmake --build --preset dev --target docs
open build/dev/docs/html/index.html
```

The generated documentation includes HTML for local viewing and XML as an
escape hatch for a future Sphinx/Breathe or other publishing pipeline. Generated
output stays under `build/` and is not committed.
