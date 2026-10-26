# Personal Chess Reps

Personal Chess Reps is a local-first desktop application for studying chess
opening repertoires. It is being designed first for macOS, with Linux
portability as a deliberate constraint.

The intended application has an HTML/CSS/TypeScript interface inside a small
native webview host. C++ owns the chess rules, PGN/FEN handling, SQLite,
training state, filesystem operations, and local engine integration. The
project is currently in its foundation and research phase: there is no
application executable yet.

## Project status

This repository currently provides:

- a CMake C++20 foundation;
- a development CMake preset;
- a Doxygen documentation target; and
- Markdown design research, including the proposed stack and Lichess opening
  data acquisition notes.

The `include/` and `src/` directories are intentionally empty placeholders for
the first implementation slice.

## Prerequisites

Required for the initial macOS development workflow:

- a C++20-capable compiler, such as Apple Clang from Xcode;
- [CMake](https://cmake.org/) 3.25 or later;
- [Ninja](https://ninja-build.org/); and
- [Doxygen](https://www.doxygen.nl/) for generated documentation.

The frontend will later also require a supported Node.js/npm installation. A
future Linux build will require the platform's GTK and WebKitGTK development
packages for the system-webview host.

On macOS, the documentation dependency can be installed with:

```sh
brew install doxygen
```

## Configure and build

Run all commands from the repository root.

```sh
cmake --preset dev
cmake --build --preset dev
```

The `dev` preset creates a Debug Ninja build in `build/dev/`. Once executable
and test targets exist, the second command will build them. At the present
foundation stage, it is expected to configure successfully without producing
an application binary.

To reconfigure after changing CMake files or installing a new dependency, run
the first command again.

## Generate documentation

The documentation combines Doxygen-generated C++ API reference with the
Markdown documents in [`docs/`](docs/).

```sh
cmake --preset dev
cmake --build --preset dev --target docs
open build/dev/docs/html/index.html
```

Generated HTML and XML remain under `build/dev/docs/` and are not committed.
The important source documents are:

- [Documentation overview](docs/overview.md)
- [Proposed technology stack](docs/tech-stack.md)
- [Lichess opening-data acquisition](docs/lichess-opening-data.md)

## Intended layout

```text
include/                 Public C++ interfaces
src/                     C++ implementation and application host
docs/                    Doxygen-visible Markdown and design notes
frontend/                Future TypeScript, HTML, and CSS application
build/                   Generated CMake, binaries, and documentation output
```

## Architectural boundary

The web frontend controls presentation: board rendering, move trees, filters,
keyboard interaction, themes, and drill feedback. The C++ side remains the
authority for every durable or chess-sensitive operation:

```text
HTML / CSS / TypeScript
          |
    narrow typed bridge
          |
C++ chess and application services
  |-- legal moves, FEN, SAN, PGN
  |-- SQLite and migrations
  |-- repertoire and review scheduling
  |-- import/export and backups
  `-- local Stockfish UCI integration
```

Frontend code must not have arbitrary SQL, filesystem, or process access. It
should call a small, explicit native API whose inputs are validated in C++.

## Documentation convention

Document public C++ interfaces in headers using Doxygen comments (`///` or
`/** ... */`). Explain contracts, invariants, ownership, errors, and threading
expectations close to the declaration. Put longer-lived design decisions,
research, workflows, and rationale in Markdown under `docs/`.

When adding a new Markdown page, link it from [`docs/overview.md`](docs/overview.md)
so it appears in the generated documentation navigation.

## Data source note

The planned opening-name source is Lichess's public `chess-openings` catalogue.
The acquisition routes and formats are documented in
[the Lichess note](docs/lichess-opening-data.md). The live Lichess Opening
Explorer and enormous game dumps are separate future considerations, not
foundation dependencies.
