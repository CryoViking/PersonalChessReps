# Personal Chess Reps

Personal Chess Reps is a local-first desktop application for learning and
maintaining chess opening repertoires. It will import and export PGN, represent
positions and variations as a transposition-aware chess graph, and schedule
practice prompts from the user's chosen lines.

The application is intended to run without an account, server, or permanent
network connection. macOS is the initial target; Linux portability is a design
constraint rather than an afterthought.

## Design documents

- [Proposed technology stack](tech-stack.md)
- [Lichess opening catalogue acquisition and processing](lichess-opening-data.md)

## Documentation policy

Generated API documentation describes the public C++ interfaces. This Markdown
area records decisions that cannot be inferred from a type signature: why a
component exists, the invariants it protects, and the trade-offs that shaped
it.
