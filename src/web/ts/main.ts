import "../css/board.css";
import { renderPieces, renderSquares } from "./board";
import { START_BOARD } from "./chess";

function requireElement<T extends HTMLElement>(selector: string) {
  const element = document.querySelector<T>(selector);
  if (element === null) throw new Error(`missing ${selector}`);
  return element;
}

const squaresLayer = requireElement(".board-squares");
renderSquares(squaresLayer);
const piecesLayer = requireElement(".board-pieces");
renderPieces(piecesLayer, START_BOARD);
