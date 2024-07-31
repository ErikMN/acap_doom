import { defineConfig } from 'vite';
import { compression } from 'vite-plugin-compression2';
import react from '@vitejs/plugin-react';

// https://vitejs.dev/config/
export default defineConfig({
  base: '/local/acap_doom',
  build: {
    outDir: 'build',
    assetsDir: 'static',
    sourcemap: false
  },
  server: {
    port: 3000,
    open: true,
    proxy: {
      '/rtsp-over-websocket': {
        target: `ws://${process.env.TARGET_IP}`,
        ws: true
      },
      '/axis-cgi/': {
        target: `http://${process.env.TARGET_IP}`,
        changeOrigin: true
      }
    }
  },
  plugins: [
    react(),
    // https://www.npmjs.com/package/vite-plugin-compression2
    compression({ deleteOriginalAssets: true, exclude: [/\.html$/] })
  ]
});
