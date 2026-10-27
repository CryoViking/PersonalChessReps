import "../css/board.css";
import { renderHighlights, renderPieces, renderSquares } from "./board";
import { type Square, START_BOARD } from "./chess";
import { squareFromPointer } from "./input";

const LEFT_CLICK = 0;
const RIGHT_CLICK = 2;

function requireElement<T extends HTMLElement>(selector: string) {
  const element = document.querySelector<T>(selector);
  if (element === null) throw new Error(`missing ${selector}`);
  return element;
}

const markedSquares = new Set<Square>();
let selectedSquare: Square | null = null;

const squaresLayer = requireElement(".board-squares");
const piecesLayer = requireElement(".board-pieces");
const highlightsLayer = requireElement(".board-highlights");

renderSquares(squaresLayer);
renderPieces(piecesLayer, START_BOARD);
renderHighlights(highlightsLayer, selectedSquare, markedSquares);

const board = requireElement("#board");
board.addEventListener("pointerdown", (event) => {
  const square = squareFromPointer(board, event);
  if (event.button === LEFT_CLICK) {
    markedSquares.clear();
    selectedSquare = square === selectedSquare ? null : square;
    renderHighlights(highlightsLayer, selectedSquare, markedSquares);
  }
  if (event.button === RIGHT_CLICK) {
    if (square !== null) {
      // explicit because 0 "a1" would be false
      if (markedSquares.has(square)) markedSquares.delete(square);
      else markedSquares.add(square);
    }
    renderHighlights(highlightsLayer, selectedSquare, markedSquares);
  }
});

board.addEventListener("contextmenu", (event) => {
  event.preventDefault();
});
