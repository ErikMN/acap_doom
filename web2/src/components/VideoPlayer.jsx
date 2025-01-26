import React, { useEffect, useState } from 'react';
import PropTypes from 'prop-types';
import { Player } from 'media-stream-player';

const OFFSET = 250;

/* Force a login by fetching usergroup */
const authorize = async () => {
  try {
    await window.fetch('/axis-cgi/usergroup.cgi', {
      credentials: 'include',
      mode: 'no-cors'
    });
  } catch (err) {
    console.error(err);
  }
};

/* Set default Vapix params if not already set */
const setDefaultParams = () => {
  const existingVapixJSON = localStorage.getItem('vapix');
  if (!existingVapixJSON) {
    const vapixConfig = {
      compression: '20',
      resolution: '1280x720'
    };
    const vapixJSON = JSON.stringify(vapixConfig);
    localStorage.setItem('vapix', vapixJSON);
    console.log('Setting Vapix params:', vapixJSON);
  }
};

function Authenticating() {
  return (
    <h3 style={{ color: 'white', fontFamily: 'Doom' }}>Authenticating...</h3>
  );
}

const VideoPlayer = ({ height }) => {
  const [authorized, setAuthorized] = useState(false);

  let vapixParams = {};
  try {
    vapixParams = JSON.parse(window.localStorage.getItem('vapix')) ?? {};
  } catch (err) {
    console.warn('no stored VAPIX parameters: ', err);
  }

  useEffect(() => {
    authorize()
      .then(() => {
        setAuthorized(true);
      })
      .catch((err) => {
        console.error(err);
      });
    setDefaultParams();
  }, []);

  if (!authorized) {
    return <Authenticating />;
  }

  return (
    <div
      style={{
        height: `${height - OFFSET}px`,
        flex: 1,
        backgroundColor: 'black',
        padding: '3px'
      }}
    >
      <Player
        hostname={window.location.host}
        initialFormat="RTP_H264"
        autoPlay
        autoRetry
        vapixParams={vapixParams}
      />
    </div>
  );
};

VideoPlayer.propTypes = {
  height: PropTypes.number.isRequired
};

export default VideoPlayer;
