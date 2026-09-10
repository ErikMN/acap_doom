import { defineConfig } from 'vite';
import { compression } from 'vite-plugin-compression2';
import react from '@vitejs/plugin-react';
import viteTsconfigPaths from 'vite-tsconfig-paths';
import svgr from 'vite-plugin-svgr';

const targetProtocol = process.env.TARGET_PROTOCOL ?? 'https';

if (targetProtocol !== 'http' && targetProtocol !== 'https') {
  throw new Error('TARGET_PROTOCOL must be either http or https');
}

const websocketProtocol = targetProtocol === 'https' ? 'wss' : 'ws';

const targetPort =
  targetProtocol === 'https'
    ? (process.env.TARGET_HTTPS_PORT ?? '443')
    : (process.env.TARGET_HTTP_PORT ?? '80');

const target = `${targetProtocol}://${process.env.TARGET_IP}:${targetPort}`;
const websocketTarget = `${websocketProtocol}://${process.env.TARGET_IP}:${targetPort}`;

// https://vitejs.dev/config/
export default defineConfig({
  base: '/local/acap_doom',
  build: {
    outDir: 'build',
    assetsDir: 'static',
    sourcemap: false,
    rollupOptions: {
      output: {
        manualChunks: (id: string): string | undefined => {
          if (id.indexOf('node_modules') !== -1) {
            // MUI in own chunk:
            if (id.indexOf('@mui') !== -1) {
              return 'vendor_mui';
            }
            // Rest of vendors in node_modules:
            return 'vendor';
          }
        }
      }
    }
  },
  server: {
    host: '0.0.0.0',
    port: 8080,
    open: true,
    /* Proxy endpoints (use HTTP or HTTPS) */
    proxy: {
      '/local/acap_doom/control': {
        target,
        ws: true,
        changeOrigin: true,
        secure: false
      },
      '/rtsp-over-websocket': {
        target: websocketTarget,
        ws: true,
        secure: false
      },
      '/axis-cgi/': {
        target,
        changeOrigin: true,
        secure: false
      },
      '/mjpg/': {
        target,
        changeOrigin: true,
        secure: false
      },
      '/img/': {
        target,
        changeOrigin: true,
        secure: false
      },
      '/config/rest/': {
        target,
        changeOrigin: true,
        secure: false
      }
    }
  },
  assetsInclude: ['**/*.oga'],
  plugins: [
    react(),
    viteTsconfigPaths(),
    svgr({
      include: '**/*.svg?react'
    }),
    // https://www.npmjs.com/package/vite-plugin-compression2
    compression({ deleteOriginalAssets: true, exclude: [/\.html$/] })
  ]
});
