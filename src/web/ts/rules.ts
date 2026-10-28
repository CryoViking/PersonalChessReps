// Calls into the C++ rules engine. Bridge replies carry errors as values; these turn
// them back into exceptions so callers can just await a PositionState.
import type { PositionState, Reply } from "./bridge";

function unwrap<T>(reply: Reply<T>): T {
  if (!reply.ok) throw new Error(reply.error);
  return reply.value;
}

export async function getPosition(fen: string): Promise<PositionState> {
  return unwrap(await window.position(fen));
}

export async function playMove(fen: string, uci: string): Promise<PositionState> {
  return unwrap(await window.play(fen, uci));
}
