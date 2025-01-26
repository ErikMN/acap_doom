import React, { useEffect, useState } from 'react';
import { useGlobalContext } from './GlobalContext';
import { useParameters } from './ParametersContext';
import { jsonRequest } from '../helpers/cgihelper';
import { doomTheme, darkTheme } from '../theme';
import { SR_CGI } from './constants';
import bg from '../assets/img/doom_bg.jpg';
/* MUI */
import { ThemeProvider, CssBaseline } from '@mui/material';
import { Box, CircularProgress, Fade, Typography } from '@mui/material';

interface LoadingScreenProps {
  Component: React.ComponentType;
}

const LoadingScreen: React.FC<LoadingScreenProps> = ({ Component }) => {
  /* Local state */
  const [systemReady, setSystemReady] = useState<string>('no');
  const [message, setMessage] = useState<string>('');

  /* Global context */
  const { currentTheme, handleOpenAlert, appLoading, setAppLoading } =
    useGlobalContext();
  const { paramsLoading } = useParameters();

  /* Theme */
  const theme = currentTheme === 'dark' ? darkTheme : doomTheme;

  /* App mount calls */
  useEffect(() => {
    /* Check system state */
    const fetchSystemReady = async () => {
      setAppLoading(true);
      /* Check protocol for HTTPS */
      // const protocol = window.location.protocol;
      // if (protocol === 'https:') {
      //   setMessage('PLEASE USE HTTP AND NOT HTTPS');
      //   return;
      // }
      const payload = {
        apiVersion: '1.0',
        method: 'systemready',
        params: {
          timeout: 10
        }
      };
      try {
        const resp = await jsonRequest(SR_CGI, payload);
        const systemReadyState = resp.data.systemready;
        /* If the system is not ready, wait a couple of seconds and retry */
        if (systemReadyState !== 'yes') {
          setTimeout(() => {
            fetchSystemReady();
          }, 2000); /* Wait before retrying */
        } else {
          setSystemReady(systemReadyState);
        }
      } catch (error) {
        console.error(error);
        handleOpenAlert('Failed to check system status', 'error');
      } finally {
        setAppLoading(false);
      }
    };
    fetchSystemReady();
  }, []);

  if (!appLoading && !paramsLoading && systemReady === 'yes') {
    return <Component />;
  }

  return (
    <ThemeProvider theme={theme}>
      <Box
        sx={{
          display: 'flex',
          flexDirection: 'column',
          justifyContent: 'center',
          alignItems: 'center',
          height: '100vh',
          width: '100%',
          position: 'relative',
          overflow: 'hidden',
          '&::before': {
            content: '""',
            position: 'absolute',
            top: 0,
            left: 0,
            width: '100%',
            height: '100%',
            backgroundImage: `url("${bg}")`,
            backgroundSize: 'cover',
            backgroundRepeat: 'no-repeat',
            backgroundPosition: 'center',
            zIndex: -1,
            animation: 'fadeEffect 2s ease-in-out',
            opacity: 0,
            animationFillMode: 'forwards'
          },
          '@keyframes fadeEffect': {
            from: {
              opacity: 0
            },
            to: {
              opacity: 1
            }
          }
        }}
      >
        <CssBaseline />
        <Fade in={true} timeout={1000} mountOnEnter unmountOnExit>
          <div
            style={{
              display: 'flex',
              flexDirection: 'column',
              justifyContent: 'center',
              alignItems: 'center',
              textAlign: 'center'
            }}
          >
            <Typography
              variant="h6"
              sx={{ fontFamily: 'Doom', marginBottom: 2 }}
            >
              {import.meta.env.VITE_WEBSITE_NAME} is getting ready
            </Typography>
            <Typography
              variant="h5"
              sx={{ fontFamily: 'Doom', marginBottom: 2 }}
            >
              {message}
            </Typography>
          </div>
        </Fade>
        {message === '' && <CircularProgress size={30} />}
      </Box>
    </ThemeProvider>
  );
};

export default LoadingScreen;
