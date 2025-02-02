import React, { useEffect, useState, useRef } from 'react';
import { Player, Format } from 'media-stream-player';
import { useGlobalContext } from './GlobalContext';
import { Dimensions } from './commonInterfaces';

interface VapixConfig {
  compression: string;
  resolution: string;
}

/* Force a login by fetching usergroup */
const authorize = async (): Promise<void> => {
  try {
    await window.fetch('/axis-cgi/usergroup.cgi', {
      credentials: 'include',
      mode: 'no-cors'
    });
  } catch (err) {
    console.error(err);
  }
};

/* Set default Vapix params if not already set */
const setDefaultParams = (): void => {
  const existingVapixJSON = localStorage.getItem('vapix');
  if (!existingVapixJSON) {
    const vapixConfig: Partial<VapixConfig> = {
      compression: '20',
      resolution: '1920x1080'
    };
    const vapixJSON = JSON.stringify(vapixConfig);
    localStorage.setItem('vapix', vapixJSON);
    console.log('Setting Vapix params:', vapixJSON);
  }
};

const VideoPlayer: React.FC = () => {
  /* Local state */
  const [authorized, setAuthorized] = useState<boolean>(false);
  const [retryCount, setRetryCount] = useState<number>(0);
  const [dimensions, setDimensions] = useState<Dimensions>({
    videoWidth: 0,
    videoHeight: 0,
    pixelWidth: 0,
    pixelHeight: 0,
    offsetX: 0,
    offsetY: 0
  });

  /* Global context */
  const { appSettings, currentTheme } = useGlobalContext();

  /* Refs */
  const playerContainerRef = useRef<HTMLDivElement | null>(null);
  const videoElementRef = useRef<HTMLVideoElement | null>(null);
  const resizeObserverRef = useRef<ResizeObserver | null>(null);

  let vapixParams: Partial<VapixConfig> = {};
  const vapixData = window.localStorage.getItem('vapix');
  if (vapixData) {
    try {
      vapixParams = JSON.parse(vapixData);
    } catch (err) {
      console.warn('Failed to parse VAPIX parameters:', err);
      window.localStorage.removeItem('vapix');
    }
  }

  /* Function to log and send video element's dimensions */
  const logVideoDimensions = () => {
    if (videoElementRef.current && playerContainerRef.current) {
      const { videoWidth, videoHeight } = videoElementRef.current;

      /* Video element pixel dimensions */
      const videoRect = videoElementRef.current.getBoundingClientRect();
      const containerRect = playerContainerRef.current.getBoundingClientRect();

      const offsetX = videoRect.left - containerRect.left;
      const offsetY = videoRect.top - containerRect.top;

      /* Check if any of the values are 0, and retry after a short delay */
      if (
        videoWidth === 0 ||
        videoHeight === 0 ||
        videoRect.width === 0 ||
        videoRect.height === 0
      ) {
        if (appSettings.debug) {
          console.warn('Video dimensions not ready, retrying...');
        }
        /* Retry after a short delay (e.g., 500ms) */
        setTimeout(() => {
          setRetryCount((count) => count + 1);
          logVideoDimensions();
        }, 500);

        return;
      }
      /* Set stream and pixel dimensions */
      setDimensions({
        videoWidth, // Stream width
        videoHeight, // Stream height
        pixelWidth: videoRect.width, // Pixel width
        pixelHeight: videoRect.height, // Pixel height
        offsetX, // Offset X (left margin of the video in the container)
        offsetY // Offset Y (top margin of the video in the container)
      });
    }
  };

  useEffect(() => {
    authorize()
      .then(() => {
        setAuthorized(true);
      })
      .catch((err) => {
        console.error(err);
      });
    setDefaultParams();

    return () => {
      /* Reset dimensions to 0 on unmount */
      // console.log('VideoPlayer unmounted: Set dimensions to 0');
      setDimensions({
        videoWidth: 0,
        videoHeight: 0,
        pixelWidth: 0,
        pixelHeight: 0,
        offsetX: 0,
        offsetY: 0
      });
    };
  }, []);

  useEffect(() => {
    if (authorized && playerContainerRef.current) {
      const videoElement = playerContainerRef.current.querySelector(
        'video'
      ) as HTMLVideoElement | null;
      videoElementRef.current = videoElement;

      if (videoElement) {
        /* Log initial dimensions */
        logVideoDimensions();

        /* Set up ResizeObserver to monitor changes in video container or video element size */
        resizeObserverRef.current = new ResizeObserver(() => {
          logVideoDimensions();
        });

        /* Observe both the player container and the video element */
        resizeObserverRef.current.observe(playerContainerRef.current);
        resizeObserverRef.current.observe(videoElement);

        /* Log video dimensions once metadata (e.g., width/height) is loaded */
        videoElement.addEventListener('loadedmetadata', logVideoDimensions);

        return () => {
          /* Cleanup ResizeObserver and event listeners */
          if (resizeObserverRef.current) {
            resizeObserverRef.current.disconnect();
          }
          videoElement.removeEventListener(
            'loadedmetadata',
            logVideoDimensions
          );
        };
      }
    }
  }, [authorized, retryCount]);

  if (!authorized) {
    return null;
  }

  return (
    <div
      ref={playerContainerRef}
      style={{
        flexGrow: 1,
        backgroundColor:
          currentTheme === 'dark' ? 'rgb(61, 61, 61)' : 'rgb(0, 0, 0)',
        padding: '3px',
        position: 'relative',
        display: 'flex',
        flexDirection: 'column',
        overflow: 'hidden'
      }}
    >
      <Player
        hostname={window.location.host}
        initialFormat={
          appSettings.wsDefault ? Format.RTP_JPEG : Format.MP4_H264
        }
        autoPlay
        autoRetry
        vapixParams={vapixParams}
      />
    </div>
  );
};

export default VideoPlayer;
