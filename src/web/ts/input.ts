import { type Point, squareAtDisplay } from "./board";
import type { Square } from "./chess";

// Pointer position in board units: (0,0) is the top-left corner, (8,8) the bottom-right.
// Not clamped, so it can fall outside 0..8 when dragging off the board.
export function pointerToBoard(board: HTMLElement, event: PointerEvent): Point {
  const rect = board.getBoundingClientRect();
  return {
    x: ((event.clientX - rect.left) / rect.width) * 8,
    y: ((event.clientY - rect.top) / rect.height) * 8,
  };
}

export function squareFromPointer(board: HTMLElement, event: PointerEvent): Square | null {
  const p = pointerToBoard(board, event);
  const displayCol = Math.floor(p.x);
  const displayRow = Math.floor(p.y);

  if (displayCol < 0 || displayCol > 7 || displayRow < 0 || displayRow > 7) return null;

  return squareAtDisplay(displayCol, displayRow);
}
