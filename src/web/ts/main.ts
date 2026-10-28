import "../css/board.css";
import {
  type Arrow,
  positionDragPiece,
  renderArrows,
  renderDragPiece,
  renderHighlights,
  renderPieces,
  renderSquares,
} from "./board";
import type { PositionState } from "./bridge";
import {
  type Board,
  boardFromFen,
  isWhite,
  type Move,
  type Piece,
  type Square,
  START_FEN,
  squareName,
} from "./chess";
import { pointerToBoard, squareFromPointer } from "./input";
import { getPosition, playMove } from "./rules";

const LEFT_CLICK = 0;
const RIGHT_CLICK = 2;

function requireElement<T extends Element>(selector: string) {
  const element = document.querySelector<T>(selector);
  if (element === null) throw new Error(`missing ${selector}`);
  return element;
}

// Game state: every position reached so far, and which one is on the board. Each entry
// keeps the move that led to it, for the last-move highlight.
type HistoryEntry = { state: PositionState; move: Move | null };

const history: HistoryEntry[] = [];
let current = 0;
let busy = false; // true while a move is waiting for C++

const markedSquares = new Set<Square>();
const arrows: Arrow[] = [];
let selectedSquare: Square | null = null;
let rightDragStart: Square | null = null;
let rightDragCurrent: Square | null = null;
let leftDrag: { from: Square; wasSelected: boolean } | null = null;

const squaresLayer = requireElement<HTMLElement>(".board-squares");
const piecesLayer = requireElement<HTMLElement>(".board-pieces");
const highlightsLayer = requireElement<HTMLElement>(".board-highlights");
const arrowsLayer = requireElement<SVGGElement>(".arrow-lines");
const dragLayer = requireElement<HTMLElement>(".board-drag");
const board = requireElement<HTMLElement>("#board");

renderSquares(squaresLayer);
history.push({ state: await getPosition(START_FEN), move: null });
redraw();

function currentEntry(): HistoryEntry {
  const entry = history[current];
  if (entry === undefined) throw new Error("no position loaded");
  return entry;
}

function currentBoard(): Board {
  const fen = currentEntry().state.fen;
  return boardFromFen(fen.slice(0, fen.indexOf(" ")));
}

// Can this piece be picked up? Only the side to move's pieces can.
function isMovable(piece: Piece): boolean {
  const whiteToMove = currentEntry().state.fen.split(" ")[1] === "w";
  return isWhite(piece) === whiteToMove;
}

// The UCI text for from -> to if it's legal, else null. Promotions become a queen for
// now; a picker comes later.
function legalUci(from: Square, to: Square): string | null {
  const uci = squareName(from) + squareName(to);
  const legal = currentEntry().state.legalMoves;
  if (legal.includes(uci)) return uci;
  if (legal.includes(`${uci}q`)) return `${uci}q`;
  return null;
}

function canMoveTo(from: Square, to: Square): boolean {
  return legalUci(from, to) !== null;
}

// Asks C++ to play the move, then shows the new position. Anything after the current
// position is dropped first, so playing from an earlier position starts a new line.
async function makeMove(from: Square, to: Square): Promise<void> {
  const uci = legalUci(from, to);
  if (uci === null || busy) return;

  busy = true;
  try {
    const state = await playMove(currentEntry().state.fen, uci);
    history.splice(current + 1);
    history.push({ state, move: { from, to } });
    current = history.length - 1;
  } catch (error) {
    console.error(error);
  } finally {
    busy = false;
    redraw();
  }
}

function toggleMark(square: Square): void {
  if (markedSquares.has(square)) markedSquares.delete(square);
  else markedSquares.add(square);
}

function toggleArrow(from: Square, to: Square): void {
  const index = arrows.findIndex((arrow) => arrow.from === from && arrow.to === to);
  if (index === -1) arrows.push({ from, to });
  else arrows.splice(index, 1);
}

function redraw(): void {
  let shown = arrows;
  if (rightDragStart !== null && rightDragCurrent !== null && rightDragStart !== rightDragCurrent) {
    shown = [...arrows, { from: rightDragStart, to: rightDragCurrent }];
  }

  renderPieces(piecesLayer, currentBoard(), leftDrag?.from ?? null);
  renderHighlights(highlightsLayer, selectedSquare, markedSquares, currentEntry().move);
  renderArrows(arrowsLayer, shown);
}

board.addEventListener("pointerdown", (event) => {
  const square = squareFromPointer(board, event);
  if (event.button === LEFT_CLICK) {
    markedSquares.clear();
    arrows.length = 0;

    const piece = square === null ? undefined : currentBoard()[square];

    if (square === null) selectedSquare = null;
    else if (
      selectedSquare !== null &&
      square !== selectedSquare &&
      canMoveTo(selectedSquare, square)
    ) {
      void makeMove(selectedSquare, square);
      selectedSquare = null;
    } else if (piece && isMovable(piece)) {
      leftDrag = { from: square, wasSelected: square === selectedSquare };
      selectedSquare = square;
      board.setPointerCapture(event.pointerId);
      renderDragPiece(dragLayer, piece);
      positionDragPiece(dragLayer, pointerToBoard(board, event));
    } else selectedSquare = null;

    redraw();
  } else if (event.button === RIGHT_CLICK) {
    rightDragStart = square;
    rightDragCurrent = square;
    board.setPointerCapture(event.pointerId);
  }
});

board.addEventListener("pointermove", (event) => {
  if (leftDrag !== null) positionDragPiece(dragLayer, pointerToBoard(board, event));
  if (rightDragStart === null) return;

  const square = squareFromPointer(board, event);
  if (square === rightDragCurrent) return;

  rightDragCurrent = square;
  redraw();
});

board.addEventListener("pointerup", (event) => {
  if (event.button === LEFT_CLICK && leftDrag !== null) {
    const end = squareFromPointer(board, event);

    if (end !== null && end !== leftDrag.from && canMoveTo(leftDrag.from, end)) {
      void makeMove(leftDrag.from, end);
    }

    // Only a first click (released where it was pressed) keeps the piece selected.
    const isFirstClicked = end === leftDrag.from && !leftDrag.wasSelected;
    if (!isFirstClicked) selectedSquare = null;

    leftDrag = null;
    renderDragPiece(dragLayer, null);
    redraw();
  } else if (event.button === RIGHT_CLICK && rightDragStart !== null) {
    const end = squareFromPointer(board, event);
    if (end === rightDragStart) toggleMark(end);
    else if (end !== null) toggleArrow(rightDragStart, end);

    rightDragStart = null;
    rightDragCurrent = null;
    redraw();
  }
});

board.addEventListener("pointercancel", () => {
  leftDrag = null;
  renderDragPiece(dragLayer, null);
  rightDragStart = null;
  rightDragCurrent = null;
  redraw();
});

board.addEventListener("contextmenu", (event) => {
  event.preventDefault();
});
