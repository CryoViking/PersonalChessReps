export {};

type PingReply = { ok: true; value: string } | { ok: false; error: string };

declare global {
  interface Window {
    ping(message: string): Promise<PingReply>;
  }
}
