import "../css/board.css";
import {
  type Arrow,
  isFlipped,
  positionDragPiece,
  renderArrows,
  renderDragPiece,
  renderHighlights,
  renderPieces,
  renderSquares,
  setFlipped,
} from "./board";
import { enableBoardResize, loadBoardSize } from "./boardSize";
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
import { renderMoveList } from "./moveList";
import { choosePromotion } from "./promotion";
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
type HistoryEntry = { state: PositionState; move: Move | null; san: string | null };

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
const promotionLayer = requireElement<HTMLElement>(".board-promotion");

const board = requireElement<HTMLElement>("#board");
const boardArea = requireElement<HTMLElement>(".board-area");
loadBoardSize(boardArea);
enableBoardResize(requireElement<HTMLElement>("#resize-handle"), board, boardArea);
const moveListElement = requireElement<HTMLElement>(".move-list");

renderSquares(squaresLayer);
history.push({ state: await getPosition(START_FEN), move: null, san: null });
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

function whiteToMove(): boolean {
  return currentEntry().state.fen.split(" ")[1] === "w";
}

// Can this piece be picked up? Only the side to move's pieces can.
function isMovable(piece: Piece): boolean {
  return isWhite(piece) === whiteToMove();
}

// Whether from -> to is legal, and if so whether it needs a promotion piece chosen.
function moveKind(from: Square, to: Square): "move" | "promotion" | null {
  const uci = squareName(from) + squareName(to);
  const legal = currentEntry().state.legalMoves;
  if (legal.includes(uci)) return "move";
  if (legal.includes(`${uci}q`)) return "promotion";
  return null;
}

function canMoveTo(from: Square, to: Square): boolean {
  return moveKind(from, to) !== null;
}

// Asks C++ to play the move, then shows the new position. Anything after the current
// position is dropped first, so playing from an earlier position starts a new line.
async function makeMove(from: Square, to: Square): Promise<void> {
  const kind = moveKind(from, to);
  if (kind === null || busy) return;

  busy = true;
  try {
    let uci = squareName(from) + squareName(to);
    if (kind === "promotion") {
      const letter = await choosePromotion(promotionLayer, to, whiteToMove());
      if (letter === null) return; // cancelled; `finally` still runs.
      uci += letter;
    }

    const state = await playMove(currentEntry().state.fen, uci);
    history.splice(current + 1);
    history.push({ state, move: { from, to }, san: state.san });
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
  renderMoveList(
    moveListElement,
    history[0]?.state.fen ?? START_FEN,
    history.slice(1).map((entry) => entry.san ?? ""),
    current,
    goTo,
  );
}

// Shows the position at `index` in the history, clamped ot the ends. Selection and
// annotations belong to the position being left, so they're cleared.
function goTo(index: number): void {
  if (busy || leftDrag !== null) return;
  current = Math.min(Math.max(index, 0), history.length - 1);
  selectedSquare = null;
  markedSquares.clear();
  arrows.length = 0;
  redraw();
}

// Back to the starting position, discarding every move played. Orientation and board size
// are view settings, so they stay as they are.
function resetBoard(): void {
  if (busy || leftDrag !== null) return;
  history.length = 1; // keep only the starting position
  goTo(0);
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

    if (end !== null && end !== leftDrag.from && canMoveTo(leftDrag.from, end))
      void makeMove(leftDrag.from, end);

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

function flipBoard(): void {
  if (busy) return;
  setFlipped(!isFlipped());
  renderSquares(squaresLayer); // the coordinate labels change sides.
  redraw();
}

// "f" flips the board, as on Lichess. It's fine to listen on the entire document
// because it isn't a browser, it's a standalone application.
document.addEventListener("keydown", (event) => {
  if (event.metaKey || event.ctrlKey || event.altKey) return;

  switch (event.key) {
    case "f": // as on Lichess
      if (!event.repeat) flipBoard();
      break;
    case "ArrowLeft":
      event.preventDefault(); // stop the page scrolling as well
      goTo(current - 1);
      break;
    case "ArrowRight":
      event.preventDefault();
      goTo(current + 1);
      break;
    case "Home":
      event.preventDefault();
      goTo(0);
      break;
    case "End":
      event.preventDefault();
      goTo(history.length - 1);
      break;
  }
});

requireElement<HTMLButtonElement>("#flip-button").addEventListener("click", flipBoard);
requireElement<HTMLButtonElement>("#nav-first").addEventListener("click", () => goTo(0));
requireElement<HTMLButtonElement>("#nav-prev").addEventListener("click", () => goTo(current - 1));
requireElement<HTMLButtonElement>("#nav-next").addEventListener("click", () => goTo(current + 1));
requireElement<HTMLButtonElement>("#nav-last").addEventListener("click", () =>
  goTo(history.length - 1),
);
requireElement<HTMLButtonElement>("#reset-board").addEventListener("click", resetBoard);
