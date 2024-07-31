import React from 'react';
import version from '../assets/etc/version_info?raw';

function AppVersion() {
  const commitHash = import.meta.env.VITE_COMMIT_HASH;

  /* If no static version info: try dynamic version info */
  if (version === null) {
    if (
      commitHash === undefined ||
      commitHash === null ||
      commitHash === '' ||
      commitHash === '(VITE_COMMIT_HASH)'
    ) {
      return null;
    }
    return <footer style={{ color: 'red' }}>{commitHash}</footer>;
  }

  return <footer style={{ color: 'white' }}>{version}</footer>;
}

export default AppVersion;
