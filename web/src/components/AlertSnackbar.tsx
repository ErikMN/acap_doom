/**
 * AlertSnackbar
 *
 * This component displays a customizable alert snackbar using MUI components.
 */
import React, { useState, useEffect } from 'react';
import { playSound } from '../helpers/utils';
import lockSoundUrl from '../assets/audio/lock.oga';
import unlockSoundUrl from '../assets/audio/unlock.oga';
/* MUI */
import { useTheme } from '@mui/material/styles';
import Alert from '@mui/material/Alert';
import CheckCircleIcon from '@mui/icons-material/CheckCircle';
import Divider from '@mui/material/Divider';
import ErrorIcon from '@mui/icons-material/Error';
import Slide from '@mui/material/Slide';
import Snackbar from '@mui/material/Snackbar';
import WarningAmberIcon from '@mui/icons-material/WarningAmber';

interface AlertSnackbarProps {
  openAlert: boolean;
  alertSeverity: 'success' | 'warning' | 'error' | 'info';
  alertContent: string;
  handleCloseAlert: () => void;
  alertOffset?: string;
}

const SlideTransition = (props: any) => {
  return <Slide {...props} direction="left" />;
};

const getIconForSeverity = (severity: string) => {
  switch (severity) {
    case 'success':
      return <CheckCircleIcon style={{ color: 'green', fontSize: '1.5rem' }} />;
    case 'warning':
      return (
        <WarningAmberIcon style={{ color: 'orange', fontSize: '1.5rem' }} />
      );
    case 'error':
      return <ErrorIcon style={{ color: 'red', fontSize: '1.5rem' }} />;
    default:
      return null;
  }
};

const getBorderColor = (severity: string, theme: any) => {
  return theme.palette[severity]?.main || theme.palette.grey[500];
};

const getBackgroundColor = (theme: any) => {
  return theme.palette.mode === 'dark'
    ? theme.palette.grey[800]
    : theme.palette.grey[100];
};

const AlertSnackbar: React.FC<AlertSnackbarProps> = ({
  openAlert,
  alertSeverity,
  alertContent,
  handleCloseAlert,
  alertOffset = undefined
}) => {
  const theme = useTheme();
  const [isPersistent, setIsPersistent] = useState(false);

  useEffect(() => {
    if (openAlert) setIsPersistent(false);
  }, [openAlert]);

  return (
    <Snackbar
      open={openAlert}
      autoHideDuration={
        isPersistent ? null : alertSeverity === 'success' ? 2000 : 6000
      }
      onClose={handleCloseAlert}
      anchorOrigin={{ vertical: 'top', horizontal: 'right' }}
      slots={{
        transition: SlideTransition
      }}
      sx={{
        '&.MuiSnackbar-root': {
          ...(alertOffset ? { top: alertOffset } : {}),
          right: theme.spacing(2)
        }
      }}
    >
      <div
        onClick={() => {
          setIsPersistent((prev) => {
            const newState = !prev;
            playSound(newState ? lockSoundUrl : unlockSoundUrl);
            return newState;
          });
        }}
      >
        <Alert
          onClose={handleCloseAlert}
          severity={alertSeverity}
          icon={false}
          sx={{
            width: '100%',
            borderRadius: 0,
            fontSize: '1.1rem',
            display: 'flex',
            alignItems: 'center',
            cursor: 'pointer',
            position: 'relative',
            border: isPersistent
              ? `2px solid ${getBorderColor(alertSeverity, theme)}`
              : 'none',
            backgroundColor: isPersistent
              ? getBackgroundColor(theme)
              : undefined,
            '@keyframes snackbar-timer': {
              from: { transform: 'scaleX(1)' },
              to: { transform: 'scaleX(0)' }
            }
          }}
        >
          {/* Timer bar */}
          {!isPersistent && (
            <div
              style={{
                position: 'absolute',
                bottom: 0,
                left: 0,
                height: '3px',
                width: '100%',
                backgroundColor: theme.palette.grey[500],
                overflow: 'hidden'
              }}
            >
              <div
                style={{
                  height: '100%',
                  width: '100%',
                  backgroundColor: theme.palette.primary.main,
                  transformOrigin: 'left',
                  animation: `snackbar-timer ${
                    alertSeverity === 'success' ? 2000 : 6000
                  }ms linear forwards`
                }}
              />
            </div>
          )}

          <div
            style={{
              display: 'flex',
              alignItems: 'center',
              width: '100%',
              gap: '8px'
            }}
          >
            <div
              style={{
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'center'
              }}
            >
              {getIconForSeverity(alertSeverity)}
            </div>
            <Divider
              orientation="vertical"
              flexItem
              sx={{
                backgroundColor: theme.palette.divider,
                margin: '0 8px'
              }}
            />
            <span>{alertContent}</span>
          </div>
        </Alert>
      </div>
    </Snackbar>
  );
};

export default AlertSnackbar;
