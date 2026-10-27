import blackBishop from "../pieces/bB.svg";
import blackKing from "../pieces/bK.svg";
import blackKnight from "../pieces/bN.svg";
import blackPawn from "../pieces/bP.svg";
import blackQueen from "../pieces/bQ.svg";
import blackRook from "../pieces/bR.svg";
import whiteBishop from "../pieces/wB.svg";
import whiteKing from "../pieces/wK.svg";
import whiteKnight from "../pieces/wN.svg";
import whitePawn from "../pieces/wP.svg";
import whiteQueen from "../pieces/wQ.svg";
import whiteRook from "../pieces/wR.svg";

import { type Board, type Piece, squareAt } from "./chess";

const PIECE_IMAGES: Record<Piece, string> = {
  P: whitePawn,
  R: whiteRook,
  N: whiteKnight,
  B: whiteBishop,
  Q: whiteQueen,
  K: whiteKing,
  p: blackPawn,
  r: blackRook,
  n: blackKnight,
  b: blackBishop,
  q: blackQueen,
  k: blackKing,
};

const PIECE_NAMES: Record<Piece, string> = {
  P: "white pawn",
  R: "white rook",
  N: "white knight",
  B: "white bishop",
  Q: "white queen",
  K: "white king",
  p: "black pawn",
  r: "black rook",
  n: "black knight",
  b: "black bishop",
  q: "black queen",
  k: "black king",
};

export function renderSquares(container: HTMLElement): void {
  const squares: HTMLDivElement[] = [];

  for (let displayRow = 0; displayRow < 8; displayRow++) {
    for (let displayCol = 0; displayCol < 8; displayCol++) {
      const row = 7 - displayRow;
      const col = displayCol;

      const square = document.createElement("div");
      if ((row + col) % 2 === 0) square.classList.add("square-dark");
      else square.classList.add("square-light");

      square.dataset.square = String(squareAt(col, row));
      squares.push(square);
    }
  }

  container.replaceChildren(...squares);
}

export function renderPieces(container: HTMLElement, board: Board): void {
  const cells: HTMLDivElement[] = [];

  for (let displayRow = 0; displayRow < 8; displayRow++) {
    for (let displayCol = 0; displayCol < 8; displayCol++) {
      const row = 7 - displayRow;
      const col = displayCol;

      const cell = document.createElement("div");
      const piece = board[squareAt(col, row)];
      if (piece) {
        const image = document.createElement("img");
        image.src = PIECE_IMAGES[piece];
        image.alt = PIECE_NAMES[piece];
        cell.append(image);
      }

      cells.push(cell);
    }
  }

  container.replaceChildren(...cells);
}
