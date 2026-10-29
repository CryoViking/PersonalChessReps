// The board's size is a wish (--board-size on the board area); CSS clamps it to the space
// available. Dragging the handle changes the wish, and it's remembered between sessions.

const STORAGE_KEY = "pcr.boardSize";
const MIN_BOARD_SIZE = 720; // px: small enough to make room, big enough to play on

function applySize(area: HTMLElement, size: number): void {
  area.style.setProperty("--board-size", `${size}px`);
}

// Restores the saved size, if there is one. Storage can be unavailable; then the CSS
// default applies.
export function loadBoardSize(area: HTMLElement): void {
  try {
    const saved = Number(localStorage.getItem(STORAGE_KEY));
    if (Number.isFinite(saved) && saved >= MIN_BOARD_SIZE) applySize(area, saved);
  } catch {
    // Keep the default size.
  }
}

function saveBoardSize(size: number): void {
  try {
    localStorage.setItem(STORAGE_KEY, String(size));
  } catch {
    // Not remembered this  time; nothing else depends on it.
  }
}

// Dragging `handle` resizes `board` from its top-left corner.
export function enableBoardResize(
  handle: HTMLElement,
  board: HTMLElement,
  area: HTMLElement,
): void {
  // Where the drag began, The board is centered, so its corner moves half as far as the
  // board grows: growing by twice the pointer's movement keeps the corner under it.
  let start: { x: number; y: number; size: number } | null = null;

  handle.addEventListener("pointerdown", (event) => {
    start = { x: event.clientX, y: event.clientY, size: board.getBoundingClientRect().width };
    handle.setPointerCapture(event.pointerId);
    event.preventDefault(); // no test selection of focus change while dragging
  });

  handle.addEventListener("pointermove", (event) => {
    if (start == null) return;
    const moved = Math.max(event.clientX - start.x, event.clientY - start.y);
    applySize(area, Math.max(MIN_BOARD_SIZE, start.size + 2 * moved));
  });

  const finish = () => {
    if (start === null) return;
    start = null;
    // Keep what CSS actually allowed, not how far past the edge the pointer went.
    const size = Math.round(board.getBoundingClientRect().width);
    applySize(area, size);
    saveBoardSize(size);
  };

  handle.addEventListener("pointerup", finish);
  handle.addEventListener("pointercancel", finish);
}
