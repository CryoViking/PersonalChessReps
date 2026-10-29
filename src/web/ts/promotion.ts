import { pieceImage, squareCenter } from "./board";
import type { Piece, Square } from "./chess";

export type PromotionLetter = "q" | "r" | "b" | "n";

// Queen first: it sits on the promotion square itself, nearest the pawn.
const CHOICES: readonly PromotionLetter[] = ["q", "r", "b", "n"];

const WHITE_PIECES: Record<PromotionLetter, Piece> = { q: "Q", r: "R", b: "B", n: "N" };
const BLACK_PIECES: Record<PromotionLetter, Piece> = { q: "q", r: "r", b: "b", n: "n" };

// Shows the four promotion pieces in a column over `square` and waits for a choice.
// Resolves to the chosen letter, or null if the user clicks elsewhere or presses Escape.
export function choosePromotion(
  layer: HTMLElement,
  square: Square,
  white: boolean,
): Promise<PromotionLetter | null> {
  return new Promise((resolve) => {
    // One abort() removes every listener added with this signal.
    const controller = new AbortController();
    const { signal } = controller;

    const finish = (choice: PromotionLetter | null) => {
      controller.abort();
      layer.replaceChildren();
      layer.classList.remove("open");
      resolve(choice);
    };

    // A one-square-wide column on the promotion square's file, reaching into the board:
    // down from the top edge for White, up from the bottom edge for Black.
    const center = squareCenter(square);
    const column = document.createElement("div");
    column.classList.add("promotion-column", white ? "from-top" : "from-bottom");
    column.style.left = `${(center.x - 0.5) * 12.5}%`;

    const pieces = white ? WHITE_PIECES : BLACK_PIECES;
    for (const letter of CHOICES) {
      const button = document.createElement("button");
      button.type = "button";
      button.append(pieceImage(pieces[letter]));
      button.addEventListener("click", () => finish(letter), { signal });
      column.append(button);
    }

    // Keep presses inside the picker away from the board, and treat any press outside
    // the column as a cancel.
    layer.addEventListener(
      "pointerdown",
      (event) => {
        event.stopPropagation();
        if (event.target instanceof Node && column.contains(event.target)) return;
        finish(null);
      },
      { signal },
    );
    document.addEventListener(
      "keydown",
      (event) => {
        if (event.key === "Escape") finish(null);
      },
      { signal },
    );

    layer.replaceChildren(column);
    layer.classList.add("open");
    column.querySelector("button")?.focus();
  });
}
