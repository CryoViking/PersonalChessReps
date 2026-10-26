import { defineConfig } from "vite";

export default defineConfig({
  root: "src/web",
  build: {
    outDir: "../../build/web",
    emptyOutDir: true,
  },
});
