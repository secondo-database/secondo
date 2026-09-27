import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

// Proxy /api to the FastAPI bridge so cookies stay same-origin in dev.
export default defineConfig({
  plugins: [react()],
  build: {
    // deck.gl and React get their own chunks, so the app chunk stays small and
    // an app-only change does not invalidate the vendor code in the cache.
    // (@luma.gl and friends are left out: grouping them pulls luma's lazily
    // loaded webgl chunk into the eager one.)
    rolldownOptions: {
      output: {
        codeSplitting: {
          groups: [
            { name: "deck", test: /node_modules[\\/]@deck\.gl[\\/]/ },
            { name: "react", test: /node_modules[\\/](react|react-dom|scheduler)[\\/]/ },
          ],
        },
      },
    },
    // Two chunks are a single library each and cannot be split further:
    // maplibre-gl (~1.06 MB, a prebuilt bundle that react-map-gl imports
    // lazily) and deck.gl (~0.92 MB). The limit sits just above maplibre.
    chunkSizeWarningLimit: 1100,
  },
  server: {
    port: 5173,
    proxy: {
      "/api": {
        target: "http://127.0.0.1:8000",
        changeOrigin: true,
      },
    },
  },
});
