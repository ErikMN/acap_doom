/* Web app main component */
import React, { useState, useEffect, useCallback, useRef } from 'react';
import { AppSettings } from './commonInterfaces';
import { useNavigate } from 'react-router-dom';
import { ThemeProvider, CssBaseline } from '@mui/material';
import { useParameters } from './ParametersContext';
import { CustomStyledIconButton, CustomButton } from './CustomComponents';
import { doomTheme, darkTheme } from '../theme';
import { useLocalStorage, useScreenSizes } from '../helpers/hooks.jsx';
import { playSound } from '../helpers/utils';
import { drawerWidth, drawerHeight, appbarHeight } from './constants';
import { log, enableLogging } from '../helpers/logger';
import { useGlobalContext } from './GlobalContext';
import { jsonRequest } from '../helpers/cgihelper';
import AboutModal from './AboutModal';
import AlertSnackbar from './AlertSnackbar';
import VideoPlayer from './VideoPlayer';
import logo from '../assets/img/doom.png';
import messageSoundUrl from '../assets/audio/message.oga';
import lockSoundUrl from '../assets/audio/lock.oga';
import unlockSoundUrl from '../assets/audio/unlock.oga';

/* Game related stuff */
import KeyPressHandler from './KeyPressHandler';
import LogBox from './LogBox';
import InfoBox from './InfoBox';

import BugReportOutlinedIcon from '@mui/icons-material/BugReportOutlined';
import PlayArrowIcon from '@mui/icons-material/PlayArrow';
import SaveIcon from '@mui/icons-material/Save';
import StopIcon from '@mui/icons-material/Stop';
import VideogameAssetOutlinedIcon from '@mui/icons-material/VideogameAssetOutlined';

/* MUI */
import { styled } from '@mui/material/styles';
import MuiAppBar, { AppBarProps as MuiAppBarProps } from '@mui/material/AppBar';
import Box from '@mui/material/Box';
import ChevronLeftIcon from '@mui/icons-material/ChevronLeft';
import ChevronRightIcon from '@mui/icons-material/ChevronRight';
import ContrastIcon from '@mui/icons-material/Contrast';
import DisplaySettingsIcon from '@mui/icons-material/DisplaySettings';
import Divider from '@mui/material/Divider';
import Drawer from '@mui/material/Drawer';
import Fab from '@mui/material/Fab';
import Fade from '@mui/material/Fade';
import InfoOutlinedIcon from '@mui/icons-material/InfoOutlined';
import KeyboardArrowDownIcon from '@mui/icons-material/KeyboardArrowDown';
import KeyboardArrowUpIcon from '@mui/icons-material/KeyboardArrowUp';
import MenuIcon from '@mui/icons-material/Menu';
import ScienceOutlinedIcon from '@mui/icons-material/ScienceOutlined';
import Toolbar from '@mui/material/Toolbar';
import Tooltip from '@mui/material/Tooltip';
import Typography from '@mui/material/Typography';
import VolumeOffOutlinedIcon from '@mui/icons-material/VolumeOffOutlined';
import VolumeUpOutlinedIcon from '@mui/icons-material/VolumeUpOutlined';

/******************************************************************************/

const Main = styled('main', {
  shouldForwardProp: (prop) => prop !== 'open' && prop !== 'isMobile'
})<{
  open?: boolean;
  isMobile?: boolean;
}>(({ theme, open, isMobile }) => ({
  flexGrow: 1,
  display: 'flex',
  flexDirection: 'column',
  padding: theme.spacing(isMobile ? 0 : '4px'),
  transition: theme.transitions.create('margin', {
    easing: theme.transitions.easing.sharp,
    duration: theme.transitions.duration.leavingScreen
  }),
  ...(isMobile
    ? { marginBottom: open ? drawerHeight : appbarHeight }
    : { marginRight: open ? drawerWidth : 0 }),
  ...(open && {
    transition: theme.transitions.create('margin', {
      easing: theme.transitions.easing.easeOut,
      duration: theme.transitions.duration.enteringScreen
    }),
    position: 'relative'
  })
}));

interface AppBarProps extends MuiAppBarProps {
  open?: boolean;
  isMobile?: boolean;
}

const AppBar = styled(MuiAppBar, {
  shouldForwardProp: (prop) => prop !== 'open' && prop !== 'isMobile'
})<AppBarProps>(({ theme, open, isMobile }) => ({
  overflowX: 'auto',
  backgroundColor: theme.palette.background.paper,
  backgroundImage: 'none',
  whiteSpace: 'nowrap',
  transition: theme.transitions.create(
    isMobile ? 'margin' : ['margin', 'width'],
    {
      easing: theme.transitions.easing.sharp,
      duration: theme.transitions.duration.leavingScreen
    }
  ),
  ...(open &&
    !isMobile && {
      width: `calc(100% - ${drawerWidth}px)`,
      marginRight: `${drawerWidth}px`,
      transition: theme.transitions.create(['margin', 'width'], {
        easing: theme.transitions.easing.easeOut,
        duration: theme.transitions.duration.enteringScreen
      })
    }),
  /* Horizontal scrollbar style */
  '&::-webkit-scrollbar': {
    height: '8px',
    backgroundColor: 'transparent'
  },
  '&::-webkit-scrollbar-thumb': {
    backgroundColor:
      theme.palette.mode === 'dark'
        ? theme.palette.grey[600]
        : theme.palette.grey[400],
    borderRadius: '6px'
  },
  '&::-webkit-scrollbar-track': {
    backgroundColor:
      theme.palette.mode === 'dark'
        ? theme.palette.grey[800]
        : theme.palette.grey[200]
  },
  '& .MuiToolbar-root': {
    minHeight: appbarHeight
  }
}));

const DrawerHeader = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  padding: theme.spacing(0, 1),
  height: appbarHeight,
  justifyContent: 'flex-end',
  flexShrink: 0
}));

/******************************************************************************/

const App: React.FC = () => {
  /* Local state */
  const [aboutModalOpen, setAboutModalOpen] = useState<boolean>(false);

  /* Local storage state */
  const [drawerOpen, setDrawerOpen] = useLocalStorage('drawerOpen', true);
  const [isMuted, setIsMuted] = useLocalStorage('mute', false);

  /* Global context */
  const {
    openAlert,
    setOpenAlert,
    alertContent,
    alertSeverity,
    currentTheme,
    setCurrentTheme,
    appSettings,
    setAppSettings,
    handleOpenAlert
  } = useGlobalContext();

  /* Refs */
  const drawerRef = useRef<HTMLDivElement>(null);

  /* Global parameter list */
  const { parameters } = useParameters();
  const ProdFullName = parameters?.['root.Brand.ProdFullName'];
  const ProdShortName = parameters?.['root.Brand.ProdShortName'];

  /* Theme */
  const theme = currentTheme === 'dark' ? darkTheme : doomTheme;

  /* Screen size */
  const { isMobile } = useScreenSizes();

  /* Navigation */
  const navigate = useNavigate();

  enableLogging(true);

  const handleDrawerClose = useCallback(() => {
    setDrawerOpen(false);
  }, [setDrawerOpen]);

  const toggleDrawerOpen = useCallback(() => {
    setDrawerOpen(!drawerOpen);
    /* Scroll drawer to top if closed in mobile mode */
    if (drawerOpen && drawerRef.current) {
      drawerRef.current.scrollTo({ top: 0 });
    }
  }, [drawerOpen, setDrawerOpen]);

  const toggleTheme = useCallback(() => {
    const newTheme = currentTheme === 'light' ? 'dark' : 'light';
    playSound(newTheme === 'dark' ? unlockSoundUrl : lockSoundUrl);
    setCurrentTheme(newTheme);
    handleOpenAlert(
      `Use theme: ${newTheme === 'light' ? 'Doom' : 'Dark'}`,
      'success'
    );
  }, [currentTheme, setCurrentTheme]);

  /* Modal open/close handlers */
  const handleOpenAboutModal = () => {
    setAboutModalOpen(true);
    playSound(messageSoundUrl);
  };
  const handleCloseAboutModal = () => setAboutModalOpen(false);

  /* Toggle debug features of the site */
  const toggleDebug = () => {
    setAppSettings((prevSettings: AppSettings) => {
      const newDebugState = !prevSettings.debug;
      playSound(newDebugState ? unlockSoundUrl : lockSoundUrl);
      return {
        ...prevSettings,
        debug: newDebugState
      };
    });
    handleOpenAlert(`Debug mode: ${!appSettings.debug}`, 'success');
  };

  /* Toggle WebSocket stream default */
  const toggleWSDefault = () => {
    setAppSettings((prevSettings: AppSettings) => {
      playSound(prevSettings.wsDefault ? lockSoundUrl : unlockSoundUrl);
      const newSettings = {
        ...prevSettings,
        wsDefault: !prevSettings.wsDefault
      };
      handleOpenAlert(
        `WebSocket stream default: ${newSettings.wsDefault}`,
        'success'
      );
      return newSettings;
    });
    /* Wait one second then reload the page */
    setTimeout(() => {
      window.location.reload();
    }, 1000);
  };

  /* App routes navigation handlers */

  /* Alert handler */
  const handleCloseAlert = (
    event?: React.SyntheticEvent | Event,
    reason?: string
  ) => {
    if (reason === 'clickaway') {
      return;
    }
    setOpenAlert(false);
  };

  const handleToggleMute = () => {
    setIsMuted((prevIsMuted: boolean) => {
      const newMuteState = !prevIsMuted;
      localStorage.setItem('mute', newMuteState.toString());
      return newMuteState;
    });
  };

  /****************************************************************************/
  /* GAME */
  const P_CGI = '/axis-cgi/packagemanager.cgi';
  const TIMEOUT = 2000;

  /* FIXME: Game state */
  const [isLoading, setLoading] = useState<boolean>(false);
  const [isRunning, setRunning] = useState<boolean>(false);
  const [loadingMessage, setLoadingMessage] = useState<string>('');
  const [response, setResponse] = useState<string>('');
  const [errorResp, setErrorResp] = useState<string>('');
  const [connectionError, setConnectionError] = useState<string>('');

  const socketRef = useRef<WebSocket | null>(null);

  /* Websocket endpoint */
  const wsHttpPort = 9000;
  const wsHttpsPort = 9001;
  const wsProtocol = window.location.protocol === 'https:' ? 'wss' : 'ws';
  const wsPort = wsProtocol === 'wss' ? wsHttpsPort : wsHttpPort;
  const wsAddress =
    import.meta.env.MODE === 'development'
      ? `${wsProtocol}://${import.meta.env.VITE_TARGET_IP}:${wsPort}`
      : `${wsProtocol}://${window.location.hostname}:${wsPort}`;

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
      socketRef.current.onmessage = (event: MessageEvent) => {
        setResponse(event.data);
      };
      /* WS onclose */
      socketRef.current.onclose = () => {
        setConnectionError('WebSocket connection closed. Reconnecting...');
        setRunning(false);
        setTimeout(connectWebSocket, TIMEOUT);
      };
      /* WS onerror */
      socketRef.current.onerror = (error: Event) => {
        console.error(
          'Error: Could not establish WebSocket connection:',
          error
        );
        setConnectionError('Error: Could not establish WebSocket connection.');
        setRunning(false);
      };
    };
    connectWebSocket();

    return () => {
      if (socketRef.current) {
        socketRef.current.close();
      }
    };
  }, []);

  /* WebSocket status indicator */
  const WsStatus = () => {
    return (
      <div
        style={{
          display: 'flex',
          flexDirection: 'column',
          justifyContent: 'center',
          alignItems: 'center'
        }}
      >
        {connectionError ? (
          <div>
            <p style={{ fontFamily: 'Doom' }}>
              <span style={{ color: 'red', fontSize: '24px' }}>&#9679;</span>{' '}
              Controls Disconnected
            </p>
            {appSettings.debug === true && (
              <p style={{ color: 'lightgrey' }}>{connectionError}</p>
            )}
          </div>
        ) : (
          <div>
            <p style={{ fontFamily: 'Doom' }}>
              <span style={{ color: '#65e765', fontSize: '24px' }}>
                &#9679;
              </span>{' '}
              Controls Connected
            </p>
            {appSettings.debug === true && (
              <Box
                sx={{
                  textAlign: 'center',
                  fontFamily: 'monospace',
                  backgroundColor: 'background.default',
                  color: 'text.primary',
                  border: '1px solid',
                  borderColor: 'divider',
                  borderRadius: '4px',
                  padding: '10px',
                  margin: '10px auto',
                  width: '100%',
                  maxWidth: '600px',
                  overflowX: 'auto',
                  boxShadow: 3
                }}
              >
                {response}
              </Box>
            )}
          </div>
        )}
      </div>
    );
  };

  /* Start or stop the app */
  const startApp = (key: 'start' | 'stop'): void => {
    const setData = async () => {
      setLoading(true);
      setLoadingMessage(
        key === 'start' ? 'Starting DOOM ...' : 'Stopping DOOM ...'
      );
      if (key === 'start') {
        playSound(lockSoundUrl);
      } else {
        playSound(unlockSoundUrl);
      }
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
        setErrorResp(error as string);
        console.error('Request failed:', error);
      }
      setErrorResp('');
      setLoading(false);
    };

    setData();
  };

  const StartStop = () => {
    return (
      <div
        style={{
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-between',
          width: '100%',
          marginTop: '20px',
          marginBottom: '20px'
        }}
      >
        <CustomButton
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
        </CustomButton>
        <div style={{ marginLeft: '20px' }} />
        <CustomButton
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
        </CustomButton>
      </div>
    );
  };

  const postKey = (key: string): void => {
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
  const handleKeyPress = (key: string): void => {
    // console.log(`Key Pressed: ${key}`);
    postKey(key);
  };

  /* Key release callback function */
  const handleKeyRelease = (key: string): void => {
    // console.log(`Key Released: ${key}`);
    postKey(key);
  };

  /****************************************************************************/
  /* Joystick */

  /* Track whether WebHID is available, and store the joystick device once connected. */
  const [joystickAvailable, setJoystickAvailable] = useState<boolean>(false);
  const [joystickDevice, setJoystickDevice] = useState<HIDDevice | null>(null);

  /* Check for WebHID support on mount */
  useEffect(() => {
    if ('hid' in navigator) {
      setJoystickAvailable(true);
    }
  }, []);

  /* Prompt the user to connect the AXIS joystick */
  const connectJoystick = async () => {
    playSound(messageSoundUrl);
    try {
      const devices = await navigator.hid.requestDevice({
        /* AXIS T8311 joystick */
        filters: [{ vendorId: 0x07c0, productId: 0x1131 }]
      });
      if (devices.length > 0) {
        const device = devices[0];
        await device.open();
        setJoystickDevice(device);
        handleOpenAlert(`Joystick connected: ${device.productName}`, 'success');
      }
    } catch (err) {
      console.error('Failed to connect joystick:', err);
      handleOpenAlert('Failed to connect joystick.', 'error');
    }
  };

  /* Disconnect the joystick if currently connected */
  const disconnectJoystick = async () => {
    playSound(unlockSoundUrl);
    if (joystickDevice) {
      try {
        await joystickDevice.close();
        setJoystickDevice(null);
        handleOpenAlert('Joystick disconnected', 'success');
      } catch (err) {
        console.error('Failed to disconnect joystick:', err);
        handleOpenAlert('Failed to disconnect joystick.', 'error');
      }
    }
  };

  /****************************************************************************/

  const contentMain = () => {
    return (
      <>
        {/* Application header bar */}
        <AppBar position="fixed" open={drawerOpen} isMobile={isMobile}>
          <Toolbar
            sx={{
              display: 'flex',
              justifyContent: 'space-between'
            }}
          >
            {/* Mute button */}
            <Tooltip title={isMuted ? 'Unmute audio' : 'Mute audio'} arrow>
              <div>
                <CustomStyledIconButton
                  color="inherit"
                  aria-label="mute/unmute audio"
                  onClick={handleToggleMute}
                  edge="end"
                  sx={{ marginRight: '0px' }}
                >
                  {isMuted ? (
                    <VolumeOffOutlinedIcon
                      sx={{
                        width: '20px',
                        height: '20px',
                        color: 'text.secondary'
                      }}
                    />
                  ) : (
                    <VolumeUpOutlinedIcon
                      sx={{
                        width: '20px',
                        height: '20px',
                        color: 'text.secondary'
                      }}
                    />
                  )}
                </CustomStyledIconButton>
              </div>
            </Tooltip>

            {/* Info Button (left of theme icon) */}
            <Tooltip title="About info" arrow>
              <div>
                <CustomStyledIconButton
                  color="inherit"
                  aria-label="about info"
                  onClick={handleOpenAboutModal}
                  edge="end"
                  sx={{ marginRight: '0px' }}
                >
                  <InfoOutlinedIcon
                    sx={{
                      width: '20px',
                      height: '20px',
                      color: 'text.secondary'
                    }}
                  />
                </CustomStyledIconButton>
              </div>
            </Tooltip>

            {/* Theme Toggle Button */}
            <Tooltip title="Toggle theme" arrow>
              <div>
                <CustomStyledIconButton
                  color="inherit"
                  aria-label="toggle theme"
                  onClick={toggleTheme}
                  edge="end"
                  sx={{ marginRight: '0px' }}
                >
                  <ContrastIcon
                    sx={{
                      width: '20px',
                      height: '20px',
                      color: 'text.secondary'
                    }}
                  />
                </CustomStyledIconButton>
              </div>
            </Tooltip>

            {/* Debug Toggle Button */}
            <Tooltip title="Toggle debug" arrow>
              <div>
                <CustomStyledIconButton
                  color="inherit"
                  aria-label="toggle debug"
                  onClick={toggleDebug}
                  edge="end"
                  sx={{ marginRight: '0px', position: 'relative' }}
                >
                  <BugReportOutlinedIcon
                    sx={{
                      width: '20px',
                      height: '20px',
                      color: 'text.secondary'
                    }}
                  />
                  {/* Cross line overlay */}
                  {!appSettings.debug && (
                    <Box
                      sx={{
                        position: 'absolute',
                        top: '50%',
                        left: '50%',
                        width: '24px',
                        height: '2px',
                        backgroundColor: 'error.main',
                        transform: 'translate(-50%, -50%) rotate(45deg)',
                        zIndex: 1
                      }}
                    />
                  )}
                </CustomStyledIconButton>
              </div>
            </Tooltip>

            {/* WS streaming default toggle */}

            {appSettings.debug && (
              <Tooltip title="Toggle WebSocket stream" arrow>
                <div>
                  <CustomStyledIconButton
                    color="inherit"
                    aria-label="toggle ws"
                    onClick={toggleWSDefault}
                    edge="end"
                    sx={{ marginRight: '0px', position: 'relative' }}
                  >
                    <DisplaySettingsIcon
                      sx={{
                        width: '20px',
                        height: '20px',
                        color: 'text.secondary'
                      }}
                    />
                    {/* Cross line overlay */}
                    {!appSettings.wsDefault && (
                      <Box
                        sx={{
                          position: 'absolute',
                          top: '50%',
                          left: '50%',
                          width: '24px',
                          height: '2px',
                          backgroundColor: 'error.main',
                          transform: 'translate(-50%, -50%) rotate(45deg)',
                          zIndex: 1
                        }}
                      />
                    )}
                  </CustomStyledIconButton>
                </div>
              </Tooltip>
            )}

            {/* Joystick Connect Button (only if WebHID is supported) */}
            {joystickAvailable && (
              <Tooltip title="Connect Joystick" arrow>
                <div>
                  <CustomStyledIconButton
                    color="inherit"
                    aria-label="connect joystick"
                    onClick={connectJoystick}
                    edge="end"
                    sx={{ marginRight: '0px' }}
                  >
                    <VideogameAssetOutlinedIcon
                      sx={{
                        width: '20px',
                        height: '20px',
                        color: 'text.secondary'
                      }}
                    />
                  </CustomStyledIconButton>
                </div>
              </Tooltip>
            )}

            {/* Joystick Disconnect Button (only if a joystick is connected) */}
            {joystickDevice && (
              <Tooltip title="Disconnect Joystick" arrow>
                <div>
                  <CustomStyledIconButton
                    color="inherit"
                    aria-label="disconnect joystick"
                    onClick={disconnectJoystick}
                    edge="end"
                    sx={{ marginRight: '0px', position: 'relative' }}
                  >
                    <VideogameAssetOutlinedIcon
                      sx={{
                        width: '20px',
                        height: '20px',
                        color: 'text.secondary',
                        transform: 'scaleX(-1)'
                      }}
                    />
                    {/* Cross line overlay */}
                    <Box
                      sx={{
                        position: 'absolute',
                        top: '50%',
                        left: '50%',
                        width: '24px',
                        height: '2px',
                        backgroundColor: 'error.main',
                        transform: 'translate(-50%, -50%) rotate(45deg)',
                        zIndex: 1
                      }}
                    />
                  </CustomStyledIconButton>
                </div>
              </Tooltip>
            )}

            {/* Title and Logo */}
            <Box
              sx={{
                flexGrow: 1,
                display: 'flex',
                justifyContent: 'center',
                alignItems: 'center'
              }}
            >
              {/* Title */}
              <Fade in={true} timeout={1000} mountOnEnter unmountOnExit>
                <Typography
                  variant={isMobile ? 'h6' : 'h5'}
                  noWrap
                  component="div"
                  style={{
                    display: 'flex',
                    alignItems: 'center',
                    fontFamily: 'Doom'
                  }}
                >
                  {/* Website Name and Product Full Name */}
                  {import.meta.env.VITE_WEBSITE_NAME} @{' '}
                  {isMobile ? ProdShortName : ProdFullName}
                  {/* Debug Icon */}
                  {appSettings.debug && (
                    <ScienceOutlinedIcon
                      sx={{
                        marginLeft: '8px',
                        marginTop: '5px',
                        width: '20px',
                        height: '20px',
                        color: 'text.secondary'
                      }}
                    />
                  )}
                </Typography>
              </Fade>
            </Box>

            {/* Menu button (left-aligned) */}
            <Tooltip
              title={drawerOpen ? 'Close the menu' : 'Open the menu'}
              arrow
              placement="right"
            >
              <div>
                <CustomStyledIconButton
                  color="inherit"
                  aria-label="open drawer"
                  onClick={toggleDrawerOpen}
                  edge="start"
                  sx={{
                    ...(!isMobile && drawerOpen
                      ? { display: 'none' }
                      : { marginLeft: '10px' })
                  }}
                >
                  <MenuIcon
                    sx={{
                      width: '20px',
                      height: '20px',
                      color: 'text.secondary'
                    }}
                  />
                </CustomStyledIconButton>
              </div>
            </Tooltip>
          </Toolbar>
        </AppBar>

        {/* Drawer menu */}
        <Drawer
          PaperProps={{ ref: drawerRef }}
          sx={{
            flexShrink: 0,
            '& .MuiDrawer-paper': {
              boxSizing: 'border-box',
              overflow: isMobile && !drawerOpen ? 'hidden' : 'auto',
              /* Prevent horizontal overflow */
              overflowX: 'hidden',
              position: 'fixed',
              '&::-webkit-scrollbar': {
                width: '8px',
                backgroundColor: 'transparent'
              },
              '&::-webkit-scrollbar-thumb': {
                backgroundColor: (theme) =>
                  theme.palette.mode === 'dark'
                    ? theme.palette.grey[600]
                    : theme.palette.grey[400],
                borderRadius: '6px'
              },
              '&::-webkit-scrollbar-track': {
                backgroundColor: (theme) =>
                  theme.palette.mode === 'dark'
                    ? theme.palette.grey[800]
                    : theme.palette.grey[200]
              },
              ...(isMobile
                ? {
                    height: drawerOpen ? drawerHeight : appbarHeight,
                    bottom: 0,
                    width: '100%'
                  }
                : {
                    width: drawerWidth,
                    right: 0,
                    top: 0,
                    height: '100%'
                  })
            }
          }}
          variant="persistent"
          anchor={isMobile ? 'bottom' : 'right'}
          /* Menu always open to show drawer header in mobile mode */
          open={isMobile ? true : drawerOpen}
        >
          <DrawerHeader
            sx={{
              display: 'flex',
              justifyContent: 'flex-end',
              alignItems: 'center',
              position: 'relative',
              width: '100%'
            }}
          >
            <Box
              sx={{
                display: 'flex',
                justifyContent: 'center',
                alignItems: 'center',
                flexGrow: 1
              }}
            >
              <img
                src={logo}
                style={{ width: '96px', height: '34px' }}
                alt="logo"
              />
            </Box>
            {/* Menu toggle button */}
            <Tooltip
              title={drawerOpen ? 'Close the menu' : 'Open the menu'}
              arrow
              placement={'right'}
            >
              <div>
                <CustomStyledIconButton
                  onClick={isMobile ? toggleDrawerOpen : handleDrawerClose}
                >
                  {isMobile ? (
                    drawerOpen ? (
                      <KeyboardArrowDownIcon />
                    ) : (
                      <KeyboardArrowUpIcon />
                    )
                  ) : theme.direction === 'ltr' ? (
                    <ChevronRightIcon />
                  ) : (
                    <ChevronLeftIcon />
                  )}
                </CustomStyledIconButton>
              </div>
            </Tooltip>
          </DrawerHeader>
          <Divider />
          {/* NOTE: Drawer content here */}
          <Box sx={{ paddingBottom: 1, pl: 1, pr: 1 }}>
            <StartStop />
            {appSettings.debug ? <LogBox /> : <InfoBox />}
            <WsStatus />
            {!isRunning && !isLoading && (
              <Fade in={true} timeout={1000} mountOnEnter unmountOnExit>
                <h3
                  style={{
                    display: 'flex',
                    justifyContent: 'center',
                    paddingTop: '1em',
                    color: '#ffcc00',
                    fontFamily: 'Doom'
                  }}
                >
                  Press START to play!
                </h3>
              </Fade>
            )}
            <h3 style={{ color: 'white' }}>
              {!isRunning && isLoading ? (
                <Fade in={true} timeout={1000} mountOnEnter unmountOnExit>
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
                </Fade>
              ) : (
                ''
              )}
            </h3>
            <h3 style={{ color: 'white' }}>{errorResp}</h3>
          </Box>
        </Drawer>

        {/* Main content */}
        <Main open={drawerOpen} isMobile={isMobile}>
          <DrawerHeader />
          {/* Video Player */}
          <VideoPlayer />
        </Main>

        {/* Alert Snackbar */}
        <AlertSnackbar
          openAlert={openAlert}
          alertSeverity={alertSeverity}
          alertContent={alertContent}
          handleCloseAlert={handleCloseAlert}
        />

        {/* About Modal */}
        <AboutModal open={aboutModalOpen} handleClose={handleCloseAboutModal} />

        {/* Scroll-to-Top Button for mobile */}
        {isMobile && drawerOpen && (
          <Fab
            color="primary"
            size="small"
            onClick={() => {
              if (drawerRef.current) {
                drawerRef.current.scrollTo({ top: 0, behavior: 'smooth' });
              }
            }}
            sx={{
              position: 'fixed',
              bottom: '16px',
              right: '16px',
              zIndex: 2000
            }}
          >
            <KeyboardArrowUpIcon />
          </Fab>
        )}
        <KeyPressHandler
          onPressCallback={handleKeyPress}
          onReleaseCallback={handleKeyRelease}
          joystickDevice={joystickDevice}
        />
      </>
    );
  };

  return (
    <ThemeProvider theme={theme}>
      <Box sx={{ display: 'flex', flexDirection: 'column', height: '100vh' }}>
        <CssBaseline />
        {contentMain()}
      </Box>
    </ThemeProvider>
  );
};

export default App;
