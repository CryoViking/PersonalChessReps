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

import { type Board, colOf, type Move, type Piece, rowOf, type Square, squareAt } from "./chess";

export type Arrow = { from: Square; to: Square };
export type Point = { x: number; y: number };

const START_OFFSET = 0.35;
const HEAD_LENGTH = 0.35;
const SHAFT_HALF_WIDTH = 0.11;
const HEAD_HALF_WIDTH = 0.26;
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

// Which way around the board is drawn, False: White at the bottom. True: Black at the
// bottom (rotated 180 degrees). A view setting only; the position itself never changes.
let flipped = false;
export function setFlipped(value: boolean) {
  flipped = value;
}
export function isFlipped(): boolean {
  return flipped;
}

// Screen position -> board square.
export function squareAtDisplay(displayCol: number, displayRow: number): Square {
  const col = flipped ? 7 - displayCol : displayCol;
  const row = flipped ? displayRow : 7 - displayRow;
  return squareAt(col, row);
}

export function squareCenter(square: Square): Point {
  const displayCol = flipped ? 7 - colOf(square) : colOf(square);
  const displayRow = flipped ? rowOf(square) : 7 - rowOf(square);
  return { x: displayCol + 0.5, y: displayRow + 0.5 };
}

function offset(p: Point, direction: Point, distance: number): Point {
  return { x: p.x + direction.x * distance, y: p.y + direction.y * distance };
}

// Unit vector (length 1) pointing `from` towards `to`;
function unit(from: Point, to: Point): Point {
  const length = Math.hypot(to.x - from.x, to.y - from.y);
  return { x: (to.x - from.x) / length, y: (to.y - from.y) / length };
}

// The direction rotated 90 degress: points sideways, used to give a leg its width.
function normal(direction: Point): Point {
  return { x: -direction.y, y: direction.x };
}

const FILES = "abcdefgh";

export function renderSquares(container: HTMLElement): void {
  const cells: HTMLDivElement[] = [];

  for (let displayRow = 0; displayRow < 8; displayRow++) {
    for (let displayCol = 0; displayCol < 8; displayCol++) {
      const square = squareAtDisplay(displayCol, displayRow);
      const cell = document.createElement("div");

      if ((displayRow + displayCol) % 2 === 1) cell.classList.add("square-dark");
      else cell.classList.add("square-light");
      cell.dataset.square = String(square);

      // Labels sit on the display edges; their text comes from whichever square is there.
      if (displayCol === 0) {
        const label = document.createElement("span");
        label.classList.add("coord", "coord-rank");
        label.textContent = String(rowOf(square) + 1);
        cell.append(label);
      }
      if (displayRow === 7) {
        const label = document.createElement("span");
        label.classList.add("coord", "coord-file");
        label.textContent = FILES.charAt(colOf(square));
        cell.append(label);
      }

      cells.push(cell);
    }
  }

  container.replaceChildren(...cells);
}

// An <img> of `piece`, with alt text.
export function pieceImage(piece: Piece): HTMLImageElement {
  const image = document.createElement("img");
  image.src = PIECE_IMAGES[piece];
  image.alt = PIECE_NAMES[piece];
  return image;
}

export function renderPieces(container: HTMLElement, board: Board, hidden: Square | null): void {
  const cells: HTMLDivElement[] = [];

  for (let displayRow = 0; displayRow < 8; displayRow++) {
    for (let displayCol = 0; displayCol < 8; displayCol++) {
      const cell = document.createElement("div");
      const square = squareAtDisplay(displayCol, displayRow);
      const piece = square === hidden ? null : board[square];

      if (piece) cell.append(pieceImage(piece));
      cells.push(cell);
    }
  }

  container.replaceChildren(...cells);
}

// The piece "in hand" during a drag; null removes it.
export function renderDragPiece(layer: HTMLElement, piece: Piece | null): void {
  if (piece === null) {
    layer.replaceChildren();
    return;
  }

  const image = pieceImage(piece);
  image.classList.add("drag-piece");
  layer.replaceChildren(image);
}

// Moves the ghost to a point in board units (0..8). One unit is 12.5% of the board.
export function positionDragPiece(layer: HTMLElement, point: Point): void {
  const image = layer.firstElementChild;
  if (!(image instanceof HTMLElement)) return;
  image.style.left = `${point.x * 12.5}%`;
  image.style.top = `${point.y * 12.5}%`;
}

export function renderHighlights(
  container: HTMLElement,
  selected: Square | null,
  marked: Set<Square>,
  lastMove: Move | null,
): void {
  const cells: HTMLDivElement[] = [];

  for (let displayRow = 0; displayRow < 8; displayRow++) {
    for (let displayCol = 0; displayCol < 8; displayCol++) {
      const square = squareAtDisplay(displayCol, displayRow);
      const cell = document.createElement("div");
      if (square === selected) cell.classList.add("highlight-selected");
      if (marked.has(square)) cell.classList.add("highlight-marked");
      if (lastMove !== null && (square === lastMove.from || square === lastMove.to)) {
        cell.classList.add("highlight-last-move");
      }

      cells.push(cell);
    }
  }

  container.replaceChildren(...cells);
}

function straightArrowPoints(a: Point, b: Point): Point[] {
  const u = unit(a, b);
  const n = normal(u);

  const tail = offset(a, u, START_OFFSET);
  const base = offset(b, u, -HEAD_LENGTH);

  return [
    offset(tail, n, SHAFT_HALF_WIDTH),
    offset(base, n, SHAFT_HALF_WIDTH),
    offset(base, n, HEAD_HALF_WIDTH),
    b,
    offset(base, n, -HEAD_HALF_WIDTH),
    offset(base, n, -SHAFT_HALF_WIDTH),
    offset(tail, n, -SHAFT_HALF_WIDTH),
  ];
}

function isKnightShaped(a: Point, b: Point): boolean {
  const dx = Math.abs(b.x - a.x);
  const dy = Math.abs(b.y - a.y);
  return (dx === 1 && dy === 2) || (dx === 2 && dy === 1);
}

// An L: the long leg first, then a 90 degree turn into the short leg, which carries the head.
function knightArrowPoints(a: Point, b: Point): Point[] {
  const corner: Point = Math.abs(b.y - a.y) === 2 ? { x: a.x, y: b.y } : { x: b.x, y: a.y };

  const n1 = normal(unit(a, corner));
  const u2 = unit(corner, b);
  const n2 = normal(u2);

  const tail = offset(a, unit(a, corner), START_OFFSET);
  const base = offset(b, u2, -HEAD_LENGTH);

  // Moving sideways along both normals land on where the two legs' edges meet.
  const outerCorner = offset(offset(corner, n1, SHAFT_HALF_WIDTH), n2, SHAFT_HALF_WIDTH);
  const innerCorner = offset(offset(corner, n1, -SHAFT_HALF_WIDTH), n2, -SHAFT_HALF_WIDTH);

  return [
    offset(tail, n1, SHAFT_HALF_WIDTH),
    outerCorner,
    offset(base, n2, SHAFT_HALF_WIDTH),
    offset(base, n2, HEAD_HALF_WIDTH),
    b,
    offset(base, n2, -HEAD_HALF_WIDTH),
    offset(base, n2, -SHAFT_HALF_WIDTH),
    innerCorner,
    offset(tail, n1, -SHAFT_HALF_WIDTH),
  ];
}

export function renderArrows(group: SVGGElement, arrows: Arrow[]): void {
  const polygons: SVGPolygonElement[] = [];

  for (const arrow of arrows) {
    const a = squareCenter(arrow.from);
    const b = squareCenter(arrow.to);
    const points = isKnightShaped(a, b) ? knightArrowPoints(a, b) : straightArrowPoints(a, b);

    const polygon = document.createElementNS(SVG_NS, "polygon");
    polygon.setAttribute("points", points.map((p) => `${p.x}, ${p.y}`).join(" "));
    polygons.push(polygon);
  }

  group.replaceChildren(...polygons);
}
