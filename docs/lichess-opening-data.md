# Acquiring opening data from Lichess

## Purpose

This note describes where Lichess's opening data lives, how to obtain it, and
what the supplied data looks like. It deliberately does **not** prescribe how
Personal Chess Reps should parse, transform, or store that data.

## The source to use for opening names

The appropriate source is the public
[`lichess-org/chess-openings`](https://github.com/lichess-org/chess-openings)
GitHub repository. Lichess describes it as an aggregated, crowdsourced data set
of chess opening names, and links to it from the `Openings` section of its open
database site.

This is the small reference catalogue for questions such as:

- What is this opening or variation called?
- What ECO code does Lichess associate with it?
- Which position does a named line reach?

It is not an opening explorer, a move-recommendation book, a collection of
annotated games, or an engine evaluation database.

## Where the files sit

The source repository's top level contains five tab-separated source files:

```text
a.tsv   # ECO A volume
b.tsv   # ECO B volume
c.tsv   # ECO C volume
d.tsv   # ECO D volume
e.tsv   # ECO E volume
```

Each source row has these fields:

| Field | Description |
| --- | --- |
| `eco` | ECO classification, such as `B90` |
| `name` | English opening name |
| `pgn` | A well-known sequence of moves, or the common master-game route to the opening |

Lichess also documents a generated `dist/` form of the same data. In addition
to the three fields above, it contains:

| Field | Description |
| --- | --- |
| `uci` | The move sequence in UCI notation |
| `epd` | The reached position in EPD form: FEN without move counters, with en-passant retained only when legal |

The upstream project documents two ways to obtain this generated form: generate
it from a checkout using its documented Python/`make` process, or download it
from an upstream GitHub Actions workflow artifact. It also offers the data in
Apache Parquet format.

## Acquisition options

### 1. Pin a Git revision — recommended

For a reproducible project dependency, clone or download a specific commit of
the repository and record its full commit SHA.

```text
https://github.com/lichess-org/chess-openings
```

This is the most useful approach for a project repository or release pipeline:
the exact input can be recovered later, updates are deliberate, and a new
upstream change cannot silently alter the application's behaviour.

### 2. Download the top-level TSV files

The five `a.tsv`–`e.tsv` files are the authoritative editable source material.
They are useful for inspection, a small manual experiment, or a tool that wants
the original ECO/name/PGN representation.

### 3. Obtain the generated distribution

If UCI moves, resulting EPD positions, or Parquet are wanted directly, use the
upstream-generated distribution rather than trying to infer what it contains
from the source file names. The upstream README identifies the workflow
artifacts as a distribution route and explains the local generation command.

When recording such a download, retain both the upstream commit SHA and the
artifact/download date: an artifact is an output of a particular repository
state, not a permanent version identifier on its own.

## Licence and attribution

`lichess-org/chess-openings` is released under the CC0 Public Domain
Dedication. Lichess states that the collection is public-domain data and that
its curation is also released under CC0 insofar as copyright applies. It is
therefore suitable for a personal application and does not require a commercial
licence or a request for permission.

Even though CC0 does not require attribution, retain a small provenance record:
the upstream URL, commit SHA, acquisition date, and a credit to Lichess. That
will make updates and future data questions far less mysterious.

## What not to confuse with the catalogue

### Live Opening Explorer

The Lichess Opening Explorer is backed by the separate
[`lila-openingexplorer`](https://github.com/lichess-org/lila-openingexplorer)
service. It can answer statistics-oriented questions from master games, rated
Lichess games, or a player's games. Its README describes a database designed to
handle trillions of positions and import compressed PGN dumps; this is not a
small download intended to be bundled in a desktop app.

Lichess's changelog also records that Opening Explorer access now requires
authentication. Treat it as a possible future, authenticated online feature,
not as the way to acquire the static opening-name catalogue.

### Open game databases

[database.lichess.org](https://database.lichess.org/) offers monthly compressed
PGN dumps, checksums, puzzles, evaluations, and links back to the opening-name
catalogue. These datasets are useful only if the project later wants game
statistics or model games. They are dramatically larger than the opening-name
data: the site currently lists billions of standard games and multi-terabyte
storage for that corpus.

The general database exports are CC0, but do not assume every collection shares
the same terms. For example, Lichess states that its official broadcast PGNs
are CC BY-SA 4.0. Check the page and record the licence for each separately
acquired dataset.

## Information to retain whenever data is acquired

- upstream URL;
- full Git commit SHA, when applicable;
- acquisition date;
- original file names and checksums, where published;
- declared licence; and
- whether the files are source TSV, generated distribution, or a separate game
  dataset.

That is enough provenance for the feasibility phase. Decisions about parsing,
SQLite representation, runtime lookup, and update tooling can be made once the
application's chess core has begun to take shape.

## Sources

- [Lichess chess-openings repository](https://github.com/lichess-org/chess-openings)
- [Lichess open database](https://database.lichess.org/)
- [Lichess opening-explorer implementation](https://github.com/lichess-org/lila-openingexplorer)
- [Lichess changelog](https://lichess.org/page/changelog)
