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
import { applyMove, type Board, isWhite, type Square, START_BOARD } from "./chess";
import { pointerToBoard, squareFromPointer } from "./input";

const LEFT_CLICK = 0;
const RIGHT_CLICK = 2;

function requireElement<T extends Element>(selector: string) {
  const element = document.querySelector<T>(selector);
  if (element === null) throw new Error(`missing ${selector}`);
  return element;
}

// Board state
let position: Board = START_BOARD;

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
redraw();

// No legality yet; just stops a piece capturing its own side.
function canMoveTo(from: Square, to: Square): boolean {
  const mover = position[from];
  const target = position[to];
  return !(mover && target && isWhite(mover) === isWhite(target));
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

  renderPieces(piecesLayer, position, leftDrag?.from ?? null);
  renderHighlights(highlightsLayer, selectedSquare, markedSquares);
  renderArrows(arrowsLayer, shown);
}

board.addEventListener("pointerdown", (event) => {
  const square = squareFromPointer(board, event);
  if (event.button === LEFT_CLICK) {
    markedSquares.clear();
    arrows.length = 0;

    const piece = square === null ? undefined : position[square];

    if (square === null) selectedSquare = null;
    else if (
      selectedSquare !== null &&
      square !== selectedSquare &&
      canMoveTo(selectedSquare, square)
    ) {
      position = applyMove(position, selectedSquare, square);
      selectedSquare = null;
    } else if (piece) {
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
      position = applyMove(position, leftDrag.from, end);
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
