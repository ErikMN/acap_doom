import React, { useEffect, useState, useRef } from 'react';
import CustomTextField from './CustomTextField.jsx';
import { serverGet } from '../helpers/cgihelper.jsx';

/* MUI */
import Button from '@mui/material/Button';
import ClearIcon from '@mui/icons-material/Clear';
import ArticleIcon from '@mui/icons-material/Article';

const L_CGI = '/axis-cgi/admin/systemlog.cgi?appname=acap_doom';

const LogBox = () => {
  /* Local states */
  const [serverLog, setServerLog] = useState('');
  const [loadingLogs, setLoadingLogs] = useState(false);

  const textFieldRef = useRef(null);

  /* Scroll to the bottom when serverLog changes */
  const scrollToBottom = () => {
    if (textFieldRef.current) {
      textFieldRef.current.scrollTop = textFieldRef.current.scrollHeight;
    }
  };

  useEffect(() => {
    scrollToBottom();
  }, [serverLog]);

  /* Get app system logs */
  const getLogs = () => {
    const setData = async () => {
      setLoadingLogs(true);
      try {
        const resp = await serverGet(L_CGI);
        if ('error' in resp) {
          console.error(resp.error);
          setLoadingLogs(false);
          return;
        }
        const responseText = await resp.text();
        setServerLog(responseText);
        setLoadingLogs(false);
      } catch (error) {
        console.error('Request failed:', error);
      }
      setLoadingLogs(false);
    };
    setData();
  };

  const clearLogs = () => {
    setServerLog('');
  };

  return (
    <div>
      {/* App log box */}
      <CustomTextField
        label={'Logs'}
        value={serverLog}
        textFieldRef={textFieldRef}
      />
      <div style={{ marginTop: '1em' }} />
      {/* App log controls */}
      <div
        style={{
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-evenly'
        }}
      >
        <Button
          onClick={getLogs}
          variant="contained"
          type="button"
          disabled={loadingLogs}
          sx={{
            fontFamily: 'Doom',
            backgroundColor: '#b30000',
            color: '#ffcc00',
            '&:hover': {
              backgroundColor: '#6d0000'
            }
          }}
        >
          <ArticleIcon sx={{ paddingRight: '5px' }} />
          Get logs
        </Button>
        <Button
          onClick={clearLogs}
          variant="contained"
          color="warning"
          type="button"
          disabled={loadingLogs}
          sx={{
            fontFamily: 'Doom',
            backgroundColor: '#b30000',
            color: '#ffcc00',
            '&:hover': {
              backgroundColor: '#6d0000'
            }
          }}
        >
          <ClearIcon sx={{ paddingRight: '5px' }} />
          Clear logs
        </Button>
      </div>
    </div>
  );
};

export default LogBox;
