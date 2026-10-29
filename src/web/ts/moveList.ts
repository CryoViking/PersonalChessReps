// The move list: numbered rows of SAN, the current move highlighted. It only shows the
// moves and reports clicks; the game state lives in main.ts.

// `sans[i[` is the move that led to history index i + 1 (index 0 is the start).
export function renderMoveList(
  container: HTMLElement,
  startFen: string,
  sans: readonly string[],
  current: number,
  onSelect: (index: number) => void,
): void {
  const fields = startFen.split(" ");
  const blackStarts = fields[1] === "b";
  const firstNumber = Number(fields[5] ?? "1") || 1;

  const rows: HTMLDivElement[] = [];
  let row: HTMLDivElement | null = null;
  let currentButton: HTMLButtonElement | null = null;

  for (const [i, san] of sans.entries()) {
    // Counting half-moves from White's first move in this numbering.
    const halfMove = i + (blackStarts ? 1 : 0);
    const whiteMove = halfMove % 2 === 0;

    if (whiteMove || row === null) {
      row = document.createElement("div");
      row.classList.add("move-row");

      const number = document.createElement("span");
      number.classList.add("move-number");
      number.textContent = `${firstNumber + Math.floor(halfMove / 2)}.`;
      row.append(number);

      // A game starting with Black to move leaves White's first cell empty: "1. ...e5".
      // Think a chess puzzle instead of a real game.
      if (!whiteMove) {
        const gap = document.createElement("span");
        gap.classList.add("move-gap");
        gap.textContent = "_";
        row.append(gap);
      }
      rows.push(row);
    }

    const index = i + 1;
    const button = document.createElement("button");
    button.type = "button";
    button.classList.add("move");
    button.textContent = san;
    button.addEventListener("click", () => onSelect(index));
    if (index === current) {
      button.classList.add("current");
      button.setAttribute("aria-current", "step");
      currentButton = button;
    }
    row.append(button);
  }

  container.replaceChildren(...rows);
  currentButton?.scrollIntoView({ block: "nearest" });
}
