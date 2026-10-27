import "../css/board.css";
import { type Arrow, renderArrows, renderHighlights, renderPieces, renderSquares } from "./board";
import { type Square, START_BOARD } from "./chess";
import { squareFromPointer } from "./input";

const LEFT_CLICK = 0;
const RIGHT_CLICK = 2;

function requireElement<T extends Element>(selector: string) {
  const element = document.querySelector<T>(selector);
  if (element === null) throw new Error(`missing ${selector}`);
  return element;
}

const markedSquares = new Set<Square>();
let selectedSquare: Square | null = null;
const arrows: Arrow[] = [];
let rightDragStart: Square | null = null;

const squaresLayer = requireElement<HTMLElement>(".board-squares");
const piecesLayer = requireElement<HTMLElement>(".board-pieces");
const highlightsLayer = requireElement<HTMLElement>(".board-highlights");
const arrowsLayer = requireElement<SVGGElement>(".arrow-lines");
const board = requireElement<HTMLElement>("#board");

renderSquares(squaresLayer);
renderPieces(piecesLayer, START_BOARD);
renderHighlights(highlightsLayer, selectedSquare, markedSquares);

function toggleMark(square: Square): void {
  if (markedSquares.has(square)) markedSquares.delete(square);
  else markedSquares.add(square);
}

function toggleArrow(from: Square, to: Square): void {
  const index = arrows.findIndex((arrow) => arrow.from === from && arrow.to === to);
  if (index === -1) arrows.push({ from, to });
  else arrows.splice(index, 1);
}

board.addEventListener("pointerdown", (event) => {
  const square = squareFromPointer(board, event);
  if (event.button === LEFT_CLICK) {
    markedSquares.clear();
    arrows.length = 0;
    selectedSquare = square === selectedSquare ? null : square;
    renderHighlights(highlightsLayer, selectedSquare, markedSquares);
    renderArrows(arrowsLayer, arrows);
  } else if (event.button === RIGHT_CLICK) {
    rightDragStart = square;
    board.setPointerCapture(event.pointerId);
  }
});

board.addEventListener("pointerup", (event) => {
  if (event.button !== RIGHT_CLICK || rightDragStart === null) return;

  const end = squareFromPointer(board, event);
  if (end === rightDragStart) toggleMark(end);
  else if (end !== null) toggleArrow(rightDragStart, end);

  rightDragStart = null;
  renderHighlights(highlightsLayer, selectedSquare, markedSquares);
  renderArrows(arrowsLayer, arrows);
});

board.addEventListener("pointercancel", () => {
  rightDragStart = null;
});

board.addEventListener("contextmenu", (event) => {
  event.preventDefault();
});
