import React, { useEffect, useState, useRef } from 'react';
import { jsonRequest } from '../helpers/cgihelper.jsx';
import license from '../assets/etc/LICENSE?raw';
import branch from '../assets/etc/branch?raw';
import PropTypes from 'prop-types';
import AppVersion from './AppVersion.jsx';
import KeyPressHandler from './KeyPressHandler.jsx';
import VideoPlayer from './VideoPlayer.jsx';
import PopupBox from './PopupBox.jsx';
import LogBox from './LogBox.jsx';
import InfoBox from './InfoBox.jsx';
import logo from '../assets/img/doom.png';
import '../assets/css/App.css';

/* MUI */
import CircularProgress from '@mui/material/CircularProgress';
import Box from '@mui/material/Container';
import Button from '@mui/material/Button';
import Zoom from '@mui/material/Zoom';
import IconButton from '@mui/material/IconButton';
import BugReportOutlinedIcon from '@mui/icons-material/BugReportOutlined';
import PlayArrowIcon from '@mui/icons-material/PlayArrow';
import StopIcon from '@mui/icons-material/Stop';
import CloseIcon from '@mui/icons-material/Close';
import MenuOpenIcon from '@mui/icons-material/MenuOpen';
import InfoOutlinedIcon from '@mui/icons-material/InfoOutlined';
import SaveIcon from '@mui/icons-material/Save';
import useMediaQuery from '@mui/material/useMediaQuery';

/* CGI endpoints */
const P_CGI = '/axis-cgi/packagemanager.cgi';

/* Websocket endpoint */
const wsPort = 9000;
const wsAddress =
  import.meta.env.MODE === 'development'
    ? `ws://${import.meta.env.VITE_TARGET_IP}:${wsPort}`
    : `ws://${window.location.hostname}:${wsPort}`;

const TIMEOUT = 2000;
const DESKTOP_SIZE = '1350px';

function App({ paramData }) {
  const isDesktopScreen = useMediaQuery(`(min-width:${DESKTOP_SIZE})`);

  /* Local states */
  const [isLoading, setLoading] = useState(false);
  const [isRunning, setRunning] = useState(false);
  const [screenWidth, setScreenWidth] = useState(window.innerWidth);
  const [screenHeight, setScreenHeight] = useState(window.innerHeight);
  const [loadingMessage, setLoadingMessage] = useState('');
  const [response, setResponse] = useState('');
  const [errorResp, setErrorResp] = useState('');
  const [connectionError, setConnectionError] = useState('');
  const [debug, setDebug] = useState(false);
  const [openLicense, setOpenLicense] = useState(false);
  const [sidePanelOpen, setSidePanelOpen] = useState(isDesktopScreen);
  const [loadApp, setLoadApp] = useState(true);

  /* Refs */
  const socketRef = useRef(null);

  /* Various mount functions */
  useEffect(() => {
    /* Check debug flag */
    const debugSetting = localStorage.getItem('debug');
    if (debugSetting !== null) {
      const isDebug = debugSetting === 'true';
      setDebug(isDebug);
    } else {
      setDebug(false);
    }
    /* Check side panel flag */
    const showSidePanel = localStorage.getItem('showSidePanel');
    if (showSidePanel !== null) {
      const show = showSidePanel === 'true';
      if (isDesktopScreen) {
        setSidePanelOpen(show);
      } else {
        setSidePanelOpen(isDesktopScreen);
      }
    } else {
      setSidePanelOpen(isDesktopScreen);
    }
  }, [isDesktopScreen]);

  /* Update screen size for videoplayer whenever the window is resized */
  useEffect(() => {
    const handleResize = () => {
      setScreenWidth(window.innerWidth);
      setScreenHeight(window.innerHeight);
    };
    window.addEventListener('resize', handleResize);
    return () => {
      window.removeEventListener('resize', handleResize);
    };
  }, []);

  /* Websocket setup */
  useEffect(() => {
    const connectWebSocket = () => {
      socketRef.current = new WebSocket(wsAddress);
      /* WS onopen */
      socketRef.current.onopen = () => {
        setConnectionError('');
        setRunning(true);
      };
      /* WS onmessage */
      socketRef.current.onmessage = (event) => {
        setResponse(event.data);
      };
      /* WS onclose */
      socketRef.current.onclose = () => {
        setConnectionError('WebSocket connection closed. Reconnecting...');
        setRunning(false);
        setTimeout(connectWebSocket, TIMEOUT);
      };
      /* WS onerror */
      socketRef.current.onerror = (error) => {
        setConnectionError(
          'Error: Could not establish WebSocket connection:',
          error
        );
        setRunning(false);
      };
    };
    /* Check protocol for HTTPS */
    const protocol = window.location.protocol;
    if (protocol === 'https:') {
      setLoadApp(false);
    } else {
      connectWebSocket();
    }

    return () => {
      if (socketRef.current) {
        socketRef.current.close();
      }
    };
  }, []);

  /* WebSocket status indicator */
  const WsStatus = () => {
    return (
      <div>
        {connectionError ? (
          <div>
            <p style={{ color: 'white', fontFamily: 'Doom' }}>
              <span style={{ color: 'red', fontSize: '24px' }}>&#9679;</span>{' '}
              Controls Disconnected
            </p>
            {debug === true && (
              <p style={{ color: 'lightgrey' }}>{connectionError}</p>
            )}
          </div>
        ) : (
          <div>
            <p style={{ color: 'white', fontFamily: 'Doom' }}>
              <span style={{ color: '#65e765', fontSize: '24px' }}>
                &#9679;
              </span>{' '}
              Controls Connected
            </p>
            {debug === true && <h4 style={{ color: 'white' }}>{response}</h4>}
          </div>
        )}
      </div>
    );
  };

  /* Start or stop the app */
  const startApp = (key) => {
    const setData = async () => {
      setLoading(true);
      setLoadingMessage(
        key === 'start' ? 'Starting DOOM ...' : 'Stopping DOOM ...'
      );
      const appReq = {
        apiVersion: '1.4',
        method: key,
        params: {
          id: 'acap_doom',
          generation: 2
        }
      };
      try {
        const resp = await jsonRequest(P_CGI, appReq);
        if ('error' in resp) {
          console.error(resp.error);
          setErrorResp(resp.error);
          setLoading(false);
          return;
        }
        setLoading(false);
        if (key === 'start') {
          setRunning(true);
        } else {
          setRunning(false);
        }
      } catch (error) {
        setErrorResp(error);
        console.error('Request failed:', error);
      }
      setErrorResp('');
      setLoading(false);
    };
    setData();
  };

  /* Toggle debug features of the site */
  const toggleDebug = () => {
    setDebug(!debug);
    localStorage.setItem('debug', !debug);
  };

  /* Toggle license info box */
  const toggleOpenLicense = () => {
    setOpenLicense(!openLicense);
  };

  /* Close license info box */
  const handleCloseLicense = () => {
    setOpenLicense(false);
  };

  const StartStop = () => {
    return (
      <div
        style={{
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-evenly'
        }}
      >
        <Button
          onClick={() => startApp('start')}
          variant="contained"
          type="button"
          disabled={isLoading || isRunning}
          sx={{
            fontFamily: 'Doom',
            backgroundColor: '#b30000',
            color: '#ffcc00',
            '&:hover': {
              backgroundColor: '#6d0000'
            }
          }}
        >
          <PlayArrowIcon sx={{ paddingRight: '5px' }} />
          START
        </Button>
        <div style={{ marginLeft: '20px' }} />
        <Button
          onClick={() => startApp('stop')}
          variant="contained"
          color="warning"
          type="button"
          disabled={isLoading || !isRunning}
          sx={{
            fontFamily: 'Doom',
            backgroundColor: '#b30000',
            color: '#ffcc00',
            '&:hover': {
              backgroundColor: 'darkred'
            }
          }}
        >
          <StopIcon sx={{ paddingRight: '5px' }} />
          STOP
        </Button>
        <IconButton
          sx={{
            display: 'flex',
            alignItems: 'center',
            marginLeft: '16px',
            color: 'red'
          }}
          onClick={toggleOpenLicense}
        >
          <InfoOutlinedIcon sx={{ color: 'white' }} />
        </IconButton>
      </div>
    );
  };

  const postKey = (key) => {
    if (!socketRef.current) {
      console.error('WebSocket is not connected.');
      return;
    }
    const keyPressReq = JSON.stringify({
      keyPress: key
    });
    try {
      socketRef.current.send(keyPressReq);
      // console.log(`Key event sent: ${key}`);
    } catch (error) {
      console.error('Failed to send key event:', error);
    }
  };

  /* Key press callback function */
  const handleKeyPress = (key) => {
    // console.log(`Key Pressed: ${key}`);
    postKey(key);
  };

  /* Key release callback function */
  const handleKeyRelease = (key) => {
    // console.log(`Key Released: ${key}`);
    postKey(key);
  };

  const toggleSidePanel = () => {
    setSidePanelOpen(!sidePanelOpen);
    localStorage.setItem('showSidePanel', !sidePanelOpen);
  };

  /* Control SidePanel */
  const sidePanel = () => {
    return (
      <Zoom in={sidePanelOpen}>
        <div className="controls">
          <div className="settingsbox">
            <div
              style={{
                display: 'flex',
                alignItems: 'center',
                marginLeft: '16px'
              }}
            >
              {/* App start stop controls */}
              <StartStop />
              <IconButton
                sx={{
                  display: 'flex',
                  alignItems: 'center',
                  marginLeft: '20px',
                  color: 'red'
                }}
                onClick={toggleSidePanel}
              >
                <CloseIcon style={{ color: '#ffcc00' }} />
              </IconButton>
            </div>

            <div style={{ marginTop: '1.4em' }} />
            {/* Log box or Info box */}
            {debug ? <LogBox /> : <InfoBox />}
            <WsStatus />
            {!isRunning && !isLoading && (
              <h3
                style={{
                  paddingTop: '2em',
                  color: '#ffcc00',
                  fontFamily: 'Doom'
                }}
              >
                Press START to play!
              </h3>
            )}
            <h3 style={{ color: 'white' }}>
              {!isRunning && isLoading ? (
                <div style={{ display: 'flex', justifyContent: 'center' }}>
                  <SaveIcon
                    className="spinner"
                    style={{
                      color: '#b30000',
                      width: '24px',
                      height: '24px',
                      marginTop: '20px'
                    }}
                  />
                  <div style={{ marginLeft: '10px' }} />
                  <div
                    style={{
                      marginTop: '22px',
                      fontFamily: 'Doom',
                      color: '#ffcc00'
                    }}
                  >
                    {loadingMessage}
                  </div>
                </div>
              ) : (
                ''
              )}
            </h3>
            <h3 style={{ color: 'white' }}>{errorResp}</h3>
            <div style={{ marginTop: '2em' }} />
          </div>
        </div>
      </Zoom>
    );
  };

  /* Web app inner contents */
  const contentInner = () => {
    return (
      <div className="App">
        <div className="main">
          <VideoPlayer width={screenWidth} height={screenHeight} />
          {sidePanelOpen ? (
            sidePanel()
          ) : (
            <IconButton
              sx={{
                display: 'flex',
                alignItems: 'start',
                width: '40px',
                height: '40px',
                color: 'red'
              }}
              onClick={toggleSidePanel}
            >
              <MenuOpenIcon style={{ color: 'white' }} />
            </IconButton>
          )}
        </div>
      </div>
    );
  };

  const Header = () => {
    const headerColor =
      branch === undefined || branch === null || typeof branch !== 'string'
        ? '#ffcc00'
        : branch.trim() !== 'main'
          ? '#0080ff'
          : '#ffcc00';

    return (
      <header className="header">
        <img
          src={logo}
          style={{ width: '160px', marginTop: '1em' }}
          alt="logo"
        />
        <h2 style={{ color: headerColor, paddingTop: '40px' }}>
          {import.meta.env.VITE_WEBSITE_NAME} @ {paramData}
        </h2>
        <div className="header-title">
          {/* Loading the game */}
          {!isRunning && isLoading && (
            <CircularProgress
              size={50}
              sx={{ color: 'red' }}
              style={{ marginTop: '40px' }}
            />
          )}
        </div>
      </header>
    );
  };

  const wrongUrl = () => {
    const currentUrl = window.location.href;
    const updatedUrl = currentUrl.replace('https', 'http');
    return (
      <div
        style={{
          marginTop: '6em',
          marginBottom: '8em',
          display: 'flex',
          flexDirection: 'column',
          justifyContent: 'center',
          alignItems: 'center',
          color: 'white',
          fontFamily: 'Doom'
        }}
      >
        <h1>PLEASE USE HTTP AND NOT HTTPS</h1>
        <h2>
          <a
            href={updatedUrl}
            style={{
              color: 'white',
              textDecoration: 'none',
              fontFamily: 'monospace'
            }}
          >
            {updatedUrl}
          </a>
        </h2>
      </div>
    );
  };

  const contentMain = () => {
    return (
      <>
        <Box maxWidth={false} sx={{ backgroundColor: 'none', maxWidth: 2000 }}>
          <Header />
          {loadApp ? contentInner() : wrongUrl()}
          <footer style={{ color: 'white' }}>
            <span style={{ marginRight: '0px' }}>
              Version: {import.meta.env.VITE_VERSION}{' '}
            </span>
            {loadApp && (
              <IconButton sx={{ color: 'white' }} onClick={toggleDebug}>
                <BugReportOutlinedIcon />
              </IconButton>
            )}
          </footer>
          {debug && <AppVersion />}
          <PopupBox
            title={'LICENSE'}
            open={openLicense}
            handleClose={handleCloseLicense}
            text={license}
          />
          <KeyPressHandler
            onPressCallback={handleKeyPress}
            onReleaseCallback={handleKeyRelease}
          />
        </Box>
      </>
    );
  };

  return <>{contentMain()}</>;
}

App.propTypes = {
  paramData: PropTypes.string
};

export default App;
