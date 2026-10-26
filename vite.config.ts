import { defineConfig } from "vite";
import { viteSingleFile } from "vite-plugin-singlefile";

export default defineConfig({
  root: "src/web",
  build: {
    outDir: "../../build/web",
    emptyOutDir: true,
  },
  plugins: [viteSingleFile()],
});
