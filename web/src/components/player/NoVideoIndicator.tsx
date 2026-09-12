import React from 'react';
import VideocamOffIcon from '@mui/icons-material/VideocamOff';

export const NoVideoIndicator: React.FC = () => (
  <div
    style={{
      position: 'absolute',
      top: 0,
      left: 0,
      width: '100%',
      height: '100%',
      display: 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      pointerEvents: 'none',
      color: 'rgba(255,255,255,0.7)',
      backgroundColor: 'black' /* Make sure the video area is black */
    }}
  >
    <VideocamOffIcon sx={{ fontSize: 72 }} />
  </div>
);
