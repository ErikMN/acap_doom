import React from 'react';
import CircularProgress from '@mui/material/CircularProgress';

interface FeedbackProps {
  readonly waiting?: boolean;
}

export const Feedback: React.FC<FeedbackProps> = ({ waiting = false }) => (
  <div
    style={{
      width: '100%',
      height: '100%',
      display: 'flex',
      flexDirection: 'column',
      justifyContent: 'center',
      alignItems: 'center'
    }}
  >
    <CircularProgress
      size={30}
      sx={{ visibility: waiting ? 'visible' : 'hidden' }}
    />
  </div>
);
