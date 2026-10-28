export type Piece = "P" | "N" | "B" | "R" | "Q" | "K" | "p" | "n" | "b" | "r" | "q" | "k";

// NOTE: We make the choice to use "row" to mean "rank" and "col" to mean "file"
// because "file" can be ambiguous in a coding context. So when displaying information to
// the screen, context is easier to understand so "rank" and "file" as chess terms will
// be acceptable. In the code itself we'll use "row" and "col"
// THEREFORE:
// col: 0-7, where 0 is the a-file and 7 the h-file.
// row: 0-7, where 0 is rank 1 (white's back rank) and 7 is rank 8. This is the board's own
// coordinate system: it doesn't depend on how the board is displayed.
// square = row * 8 + col, so a1 = 0, h1 = 7, a8 = 56, h8 = 63, as before.
export type Square = number;

export type Board = readonly (Piece | null)[];

export type Move = { from: Square; to: Square };

export function isPiece(c: string): c is Piece {
  return c.length === 1 && "PNBRQKpnbrqk".includes(c);
}

export function squareAt(col: number, row: number): Square {
  return row * 8 + col;
}

export function colOf(square: Square): number {
  return square % 8;
}

export function rowOf(square: Square): number {
  return Math.floor(square / 8);
}

// The algebraic name of a square: squareAt(4, 1) is "e2". Matches C++'s square_name.
export function squareName(square: Square): string {
  return "abcdefgh".charAt(colOf(square)) + String(rowOf(square) + 1);
}

export function isWhite(piece: Piece): boolean {
  return piece === piece.toUpperCase();
}

export function boardFromFen(placement: string): Board {
  const board: (Piece | null)[] = new Array(64).fill(null);

  const rows = placement.split("/");
  if (rows.length !== 8) throw new Error(`invalid FEN string: ${placement}`);

  for (const [i, rowString] of rows.entries()) {
    const row = 7 - i;
    let col = 0;

    for (const ch of rowString) {
      if (/^[1-8]$/.test(ch)) {
        col += Number(ch);
        if (col > 8) throw new Error(`rank "${rowString}" has more than 8 files`);
      } else if (isPiece(ch)) {
        if (col >= 8) throw new Error(`rank "${rowString}" has more than 8 files`);
        board[squareAt(col, row)] = ch;
        col += 1;
      } else throw new Error(`invalid character: ${ch} in FEN: ${placement}`);
    }

    if (col !== 8) throw new Error(`incomplete FEN string: ${placement}`);
  }
  return board;
}

export const START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
