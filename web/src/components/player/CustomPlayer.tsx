/**
 * CustomPlayer
 *
 * A custom video player built using the media-stream-player library.
 * https://github.com/AxisCommunications/media-stream-player-js
 */
import React, {
  forwardRef,
  useCallback,
  useEffect,
  useLayoutEffect,
  useMemo,
  useRef,
  useState
} from 'react';
import {
  Stats,
  PlaybackArea,
  PlayerNativeElement,
  Format,
  VideoProperties,
  VapixParameters
} from 'media-stream-player';
import { Feedback } from './Feedback';
import { NoVideoIndicator } from './NoVideoIndicator';
import { Container, Layer } from './PlayerContainer';
import { Limiter } from './Limiter';
import { PlayerControls } from './PlayerControls';
import { getImageURL } from './GetImageURL';

import {
  useSwitch,
  useScreenSizes,
  useLocalStorage
} from '../../helpers/hooks';

const DEFAULT_FORMAT = Format.MP4_H264;

interface CustomPlayerProps {
  /**
   * Callback from Controls to toggle fullscreen mode on the outer player container.
   * Expected to call requestFullscreen() or exitFullscreen on the container element.
   * If omitted, the fullscreen button should be hidden/disabled by the parent.
   */
  readonly onToggleFullscreen?: () => void;
  /**
   * True when the outer player container is currently in fullscreen.
   * Derived by the parent from !!document.fullscreenElement via the
   * 'fullscreenchange' event and passed down for UI state (icon/text).
   */
  readonly isFullscreen?: boolean;

  readonly hostname: string;
  readonly vapixParams?: VapixParameters;
  readonly initialFormat?: Format;
  readonly autoPlay?: boolean;
  /**
   * Set to true if the camera requires a secure
   * connection, "https" and "wss" protocols.
   */
  readonly secure?: boolean;
  readonly aspectRatio?: number;
  readonly className?: string;
  /**
   * When playing a recording, the time the video started
   * (used for labeling with an absolute time) formatted
   * as an ISO time, e.g.: 2021-02-03T12:21:57.465715Z
   */
  readonly startTime?: string;
  /**
   * When playing a recording, the total duration of the video
   * if known by the user (and not reported from backend) in
   * seconds.
   */
  readonly duration?: number;

  /**
   * Activate automatic retries on RTSP errors.
   */
  readonly autoRetry?: boolean;
}

export const CustomPlayer = forwardRef<PlayerNativeElement, CustomPlayerProps>(
  (
    {
      hostname,
      vapixParams = {},
      initialFormat = DEFAULT_FORMAT,
      autoPlay = false,
      secure,
      className,
      startTime,
      duration,
      autoRetry = false,
      onToggleFullscreen,
      isFullscreen
    },
    ref
  ) => {
    const [play, setPlay] = useState(autoPlay);
    const [offset, setOffset] = useState(0);
    const [refresh, setRefresh] = useState(0);
    const [host, setHost] = useState(hostname);
    const [waiting, setWaiting] = useState(autoPlay);
    const [volume, setVolume] = useState<number>();
    const [expanded, setExpanded] = useState(true);

    const [format, setFormat] = useLocalStorage(
      'streamFormat',
      initialFormat,
      false
    ) as [Format, (v: Format) => void];

    /**
     * VAPIX parameters
     */
    const [vapixParameters, setVapixParameters] = useState(vapixParams);

    useEffect(() => {
      /**
       * Check if localStorage actually exists, since if you
       * server side render, localStorage won't be available.
       */
      if (window?.localStorage !== undefined) {
        window.localStorage.setItem('vapix', JSON.stringify(vapixParameters));
      }
    }, [vapixParameters]);

    /**
     * Stats overlay
     */
    const [showStatsOverlay, toggleStatsOverlay] = useSwitch(
      window?.localStorage !== undefined
        ? window.localStorage.getItem('stats-overlay') === 'on'
        : false
    ) as [boolean, (state?: boolean) => void];

    useEffect(() => {
      if (window?.localStorage !== undefined) {
        window.localStorage.setItem(
          'stats-overlay',
          showStatsOverlay ? 'on' : 'off'
        );
      }
    }, [showStatsOverlay]);

    /**
     * Controls
     */
    const [videoProperties, setVideoProperties] = useState<VideoProperties>();

    /* Screen size */
    const { isMobile } = useScreenSizes();

    const onPlaying = useCallback(
      (props: VideoProperties) => {
        setVideoProperties(props);
        setWaiting(false);
        setVolume(props.volume);
      },
      [setWaiting]
    );

    const onPlayStop = useCallback(() => {
      if (host) {
        setPlay(false);
        setHost('');
        setWaiting(false);
      } else {
        setWaiting(true);
        setHost(hostname);
        setPlay(true);
      }
    }, [host, hostname]);

    const onRefresh = useCallback(() => {
      setPlay(true);
      setRefresh((value) => value + 1);
      setWaiting(true);
    }, []);

    const onScreenshot = useCallback(() => {
      if (videoProperties === undefined) {
        return undefined;
      }

      const { el, width, height } = videoProperties;
      const imageURL = getImageURL(el, { width, height });
      const link = document.createElement('a');
      const event = new window.MouseEvent('click');

      link.download = `snapshot_${Date.now()}.jpg`;
      link.href = imageURL;
      link.dispatchEvent(event);
    }, [videoProperties]);

    /* NOTE: This callback is never used, stop is handled in onPlayStop */
    const onStop = useCallback(() => {
      setPlay(false);
      setHost('');
      setWaiting(false);
    }, []);

    const onVapix = useCallback((key: string, value: string) => {
      setVapixParameters((p: typeof vapixParams) => {
        const newParams = { ...p, [key]: value };
        if (value === '') {
          delete newParams[key];
        }
        return newParams;
      });
      /* NOTE: Clear video properties if setting default resolution */
      if (key === 'resolution' && value === '') {
        setVideoProperties(undefined);
      }
      setRefresh((refreshCount) => refreshCount + 1);
    }, []);

    /**
     * Refresh when changing visibility (e.g. when you leave a tab the
     * video will halt, so when you return we need to play again).
     */
    useEffect(() => {
      const cb = () => {
        if (document.visibilityState === 'visible') {
          setPlay(true);
          setHost(hostname);
        } else if (document.visibilityState === 'hidden') {
          setPlay(false);
          setWaiting(false);
          setHost('');
        }
      };

      document.addEventListener('visibilitychange', cb);

      return () => document.removeEventListener('visibilitychange', cb);
    }, [hostname]);

    /**
     * Aspect ratio
     *
     * This needs to be set so make the Container (and Layers) match the size of
     * the visible image of the video or still image.
     */
    const naturalAspectRatio = useMemo(() => {
      if (videoProperties === undefined) {
        return undefined;
      }

      const { width, height } = videoProperties;

      return width / height;
    }, [videoProperties]);

    /**
     * Limit video size.
     *
     * The video size should not expand outside the available container, and
     * should be recomputed on resize.
     */
    const limiterRef = useRef<HTMLDivElement>(null);
    useLayoutEffect(() => {
      if (naturalAspectRatio === undefined || limiterRef.current === null) {
        return;
      }

      const observer = new window.ResizeObserver(([entry]) => {
        const element = entry.target as HTMLElement;
        if (!isMobile) {
          const maxWidth = element.clientHeight * naturalAspectRatio;
          element.style.maxWidth = `${maxWidth}px`;
        } else {
          /* Mobile mode: no width limit */
          element.style.maxWidth = '100%';
        }
      });
      observer.observe(limiterRef.current);

      return () => observer.disconnect();
    }, [naturalAspectRatio, isMobile]);

    /**
     * Volume control on the VideoElement (h264 only)
     */
    useEffect(() => {
      if (videoProperties?.volume !== undefined && volume !== undefined) {
        const videoEl = videoProperties.el as HTMLVideoElement;
        videoEl.muted = volume === 0;
        videoEl.volume = volume;
      }
    }, [videoProperties, volume]);

    /**
     * Refresh on stream end
     */
    const onEnded = useCallback(() => {
      if (autoRetry) {
        onRefresh();
      }
    }, [autoRetry, onRefresh]);

    /**
     * Toggle expand stats
     */
    const handleExpandStats = useCallback((_value: boolean) => {
      setExpanded((prev) => !prev);
    }, []);

    /**
     * Render
     *
     * Each layer is positioned exactly on top of the visible image, since the
     * aspect ratio is carried over to the container, and the layers match the
     * container size.
     *
     * There is a layer for the spinner (feedback), a statistics overlay, and a
     * control bar with play/pause/stop/refresh and a settings menu.
     */
    return (
      <div
        style={{
          position: 'relative',
          width: '100%',
          height: '100%',
          display: 'flex',
          flexDirection: 'column'
        }}
        className={className}
      >
        {/* Client stream data */}
        {showStatsOverlay && videoProperties && (
          <div style={{ position: 'static', zIndex: 10 }}>
            <Stats
              format={format}
              videoProperties={videoProperties}
              refresh={refresh}
              volume={volume}
              expanded={expanded}
              onToggleExpanded={handleExpandStats}
            />
          </div>
        )}
        <div style={{ position: 'relative', flex: 1, minHeight: 0 }}>
          <Limiter ref={limiterRef}>
            <Container aspectRatio={naturalAspectRatio}>
              <Layer>
                <PlaybackArea
                  forwardedRef={ref}
                  refresh={refresh}
                  play={play}
                  offset={offset}
                  host={host}
                  format={format}
                  parameters={vapixParameters}
                  onPlaying={onPlaying}
                  onEnded={onEnded}
                  secure={secure}
                  autoRetry={autoRetry}
                />
              </Layer>
              <Layer>
                {/*
                  Show Feedback only while the stream is starting:
                  - host: a real backend endpoint is set
                  - play: the user has requested playback
                  - waiting: PlaybackArea has not yet emitted onPlaying
                  This prevents the spinner from appearing after Stop or during teardown.
                */}
                <Feedback waiting={!!host && play && waiting} />
              </Layer>
              {/* No video indicator */}
              <Layer>{!host && <NoVideoIndicator />}</Layer>
            </Container>
          </Limiter>
        </div>
        <div style={{ position: 'relative', zIndex: 10 }}>
          <PlayerControls
            play={play}
            videoProperties={videoProperties}
            src={host}
            parameters={vapixParameters}
            onPlay={onPlayStop}
            onStop={onStop}
            onRefresh={onRefresh}
            onScreenshot={onScreenshot}
            onFormat={(fmt) => {
              setFormat(fmt);
            }}
            onVapix={onVapix}
            onSeek={setOffset}
            labels={{
              play: 'Play',
              pause: 'Pause',
              stop: 'Stop',
              refresh: 'Refresh',
              settings: 'Settings',
              screenshot: 'Take a snapshot',
              volume: 'Volume'
            }}
            showStatsOverlay={showStatsOverlay}
            toggleStats={toggleStatsOverlay}
            format={format}
            volume={volume}
            setVolume={setVolume}
            startTime={startTime}
            duration={duration}
            onToggleFullscreen={onToggleFullscreen}
            isFullscreen={isFullscreen}
            waiting={waiting}
          />
        </div>
      </div>
    );
  }
);

CustomPlayer.displayName = 'Player';
