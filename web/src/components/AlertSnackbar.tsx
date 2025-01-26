import React from 'react';
/* MUI */
import { useTheme } from '@mui/material/styles';
import Alert from '@mui/material/Alert';
import CheckCircleIcon from '@mui/icons-material/CheckCircle';
import Divider from '@mui/material/Divider';
import ErrorIcon from '@mui/icons-material/Error';
import Snackbar from '@mui/material/Snackbar';
import WarningAmberIcon from '@mui/icons-material/WarningAmber';

interface AlertSnackbarProps {
  openAlert: boolean;
  alertSeverity: 'success' | 'warning' | 'error' | 'info';
  alertContent: string;
  handleCloseAlert: () => void;
}

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

const AlertSnackbar: React.FC<AlertSnackbarProps> = ({
  openAlert,
  alertSeverity,
  alertContent,
  handleCloseAlert
}) => {
  const theme = useTheme();

  return (
    <Snackbar
      open={openAlert}
      autoHideDuration={2000}
      onClose={handleCloseAlert}
      anchorOrigin={{ vertical: 'bottom', horizontal: 'right' }}
    >
      <Alert
        onClose={handleCloseAlert}
        severity={alertSeverity}
        icon={false}
        sx={{
          fontFamily: 'Doom',
          width: '100%',
          borderRadius: 0,
          fontSize: '1.1rem',
          display: 'flex',
          alignItems: 'center'
        }}
      >
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
    </Snackbar>
  );
};

export default AlertSnackbar;
