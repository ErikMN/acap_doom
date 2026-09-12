/**
 * VideoPlayer
 *
 * Main video player component that handles authorization, fullscreen toggling,
 * and video playback through CustomPlayer.
 */
import React, { useEffect, useState, useRef, useCallback } from 'react';
import { useGlobalContext } from './GlobalContext';
import { CustomPlayer } from './player/CustomPlayer';
import { Format } from 'media-stream-player';

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

const VideoPlayer: React.FC = () => {
  /* Local state */
  const [isFullscreen, setIsFullscreen] = useState(false);
  const [authorized, setAuthorized] = useState<boolean>(false);
  /* Global context */
  const { appSettings, currentTheme } = useGlobalContext();

  /* Refs */
  const playerContainerRef = useRef<HTMLDivElement | null>(null);

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

  /* Toggle fullscreen
   *
   * Requests fullscreen on the player container when not already in fullscreen,
   * otherwise exits fullscreen. Uses the standard Fullscreen API and guards for
   * missing methods (older browsers) with optional chaining "?.()".
   */
  const toggleFullscreen = useCallback(() => {
    const element = playerContainerRef.current;
    if (!element) return;

    if (!document.fullscreenElement) {
      /* Enter fullscreen on the container element */
      element.requestFullscreen?.().catch((err) => {
        console.error('Failed to enter fullscreen:', err);
      });
    } else {
      /* Exit fullscreen */
      document.exitFullscreen?.().catch((err) => {
        console.error('Failed to exit fullscreen:', err);
      });
    }
  }, []);

  /* Listen for fullscreen changes
   *
   * Updates local state (isFullscreen) whenever the document enters or exits
   * fullscreen. The state is driven by the presence of document.fullscreenElement.
   * Cleanup removes the event listener on unmount.
   */
  useEffect(() => {
    const handleFullscreenChange = () => {
      setIsFullscreen(!!document.fullscreenElement);
    };
    document.addEventListener('fullscreenchange', handleFullscreenChange);

    return () => {
      document.removeEventListener('fullscreenchange', handleFullscreenChange);
    };
  }, []);

  useEffect(() => {
    authorize()
      .then(() => {
        setAuthorized(true);
      })
      .catch((err) => {
        console.error(err);
      });
  }, []);

  /* Not authorized: return */
  if (!authorized) {
    return null;
  }

  return (
    <div
      ref={playerContainerRef}
      style={{
        flexGrow: 1,
        minHeight: 0,
        backgroundColor:
          currentTheme === 'dark' ? 'rgb(31, 31, 31)' : 'rgb(0, 0, 0)',
        position: 'relative',
        display: 'flex',
        flexDirection: 'column',
        overflow: 'hidden'
      }}
    >
      <CustomPlayer
        hostname={window.location.host}
        initialFormat={
          appSettings.wsDefault ? Format.RTP_H264 : Format.MP4_H264
        }
        autoPlay
        autoRetry
        vapixParams={vapixParams}
        onToggleFullscreen={toggleFullscreen}
        isFullscreen={isFullscreen}
      />
    </div>
  );
};

export default VideoPlayer;
