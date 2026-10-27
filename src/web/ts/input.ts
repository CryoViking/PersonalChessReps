import { squareAtDisplay } from "./board";
import type { Square } from "./chess";

export function squareFromPointer(board: HTMLElement, event: PointerEvent): Square | null {
  const rect = board.getBoundingClientRect();

  const x = event.clientX - rect.left;
  const y = event.clientY - rect.top;

  const displayCol = Math.floor(x / (rect.width / 8));
  const displayRow = Math.floor(y / (rect.height / 8));

  if (displayCol < 0 || displayCol > 7 || displayRow < 0 || displayRow > 7) return null;

  return squareAtDisplay(displayCol, displayRow);
}
