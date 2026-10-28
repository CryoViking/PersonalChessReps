export type Reply<T> = { ok: true; value: T } | { ok: false; error: string };

export type PositionState = { fen: string; legalMoves: string[]; inCheck: boolean };

declare global {
  interface Window {
    position(fen: string): Promise<Reply<PositionState>>;
    play(fen: string, uci: string): Promise<Reply<PositionState>>;
  }
}
