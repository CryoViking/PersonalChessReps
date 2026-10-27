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

import { type Board, type Piece, type Square, squareAt } from "./chess";

export type Arrow = { from: Square; to: Square };
export type Point = { x: number; y: number };

const START_OFFSET = 0.35;
const HEAD_LENGTH = 0.3;
const SHAFT_HALF_WIDTH = 0.1;
const HEAD_HALF_WIDTH = 0.25;
const SVG_NS = "http://www.w3.org/2000/svg";

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

export function squareAtDisplay(displayCol: number, displayRow: number): Square {
  const row = 7 - displayRow;
  const col = displayCol;
  return squareAt(col, row);
}

function offset(p: Point, direction: Point, distance: number): Point {
  return { x: p.x + direction.x * distance, y: p.y + direction.y * distance };
}

export function squareCenter(square: Square): Point {
  const col: number = square % 8;
  const row: number = Math.floor(square / 8);
  const displayRow = 7 - row;
  return { x: col + 0.5, y: displayRow + 0.5 };
}

export function renderSquares(container: HTMLElement): void {
  const squares: HTMLDivElement[] = [];

  for (let displayRow = 0; displayRow < 8; displayRow++) {
    for (let displayCol = 0; displayCol < 8; displayCol++) {
      const square = document.createElement("div");

      if ((displayRow + displayCol) % 2 === 1) square.classList.add("square-dark");
      else square.classList.add("square-light");

      square.dataset.square = String(squareAtDisplay(displayCol, displayRow));
      squares.push(square);
    }
  }

  container.replaceChildren(...squares);
}

export function renderPieces(container: HTMLElement, board: Board): void {
  const cells: HTMLDivElement[] = [];

  for (let displayRow = 0; displayRow < 8; displayRow++) {
    for (let displayCol = 0; displayCol < 8; displayCol++) {
      const cell = document.createElement("div");
      const piece = board[squareAtDisplay(displayCol, displayRow)];

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

export function renderHighlights(
  container: HTMLElement,
  selected: Square | null,
  marked: Set<Square>,
): void {
  const cells: HTMLDivElement[] = [];

  for (let displayRow = 0; displayRow < 8; displayRow++) {
    for (let displayCol = 0; displayCol < 8; displayCol++) {
      const square = squareAtDisplay(displayCol, displayRow);
      const cell = document.createElement("div");
      if (square === selected) cell.classList.add("highlight-selected");
      if (marked.has(square)) cell.classList.add("highlight-marked");

      cells.push(cell);
    }
  }

  container.replaceChildren(...cells);
}

export function renderArrows(group: SVGGElement, arrows: Arrow[]): void {
  const polygons: SVGPolygonElement[] = [];

  for (const arrow of arrows) {
    const a = squareCenter(arrow.from);
    const b = squareCenter(arrow.to);

    // from !== to is guaranteed by toggleArrow, so length is never 0
    const length = Math.hypot(b.x - a.x, b.y - a.y);
    const u: Point = { x: (b.x - a.x) / length, y: (b.y - a.y) / length };
    const n: Point = { x: -u.y, y: u.x };

    const tail = offset(a, u, START_OFFSET);
    const base = offset(b, u, -HEAD_LENGTH);

    const points = [
      offset(tail, n, SHAFT_HALF_WIDTH),
      offset(base, n, SHAFT_HALF_WIDTH),
      offset(base, n, HEAD_HALF_WIDTH),
      b,
      offset(base, n, -HEAD_HALF_WIDTH),
      offset(base, n, -SHAFT_HALF_WIDTH),
      offset(tail, n, -SHAFT_HALF_WIDTH),
    ];

    const polygon = document.createElementNS(SVG_NS, "polygon");
    polygon.setAttribute("points", points.map((p) => `${p.x}, ${p.y}`).join(" "));
    polygons.push(polygon);
  }

  group.replaceChildren(...polygons);
}
