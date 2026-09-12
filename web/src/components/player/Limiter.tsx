import React, { forwardRef, PropsWithChildren } from 'react';
import { useScreenSizes } from '../../helpers/hooks.jsx';

/**
 * The limiter prevents the video element to use up all of the available width.
 * The player container will automatically limit it's own height based on the
 * available width (keeping aspect ratio).
 */
export const Limiter = forwardRef<HTMLDivElement, PropsWithChildren>(
  ({ children }, ref) => {
    /* Screen size */
    const { isMobile } = useScreenSizes();

    return (
      <div
        ref={ref}
        style={{
          position: isMobile ? 'relative' : 'absolute',
          left: isMobile ? undefined : '50%',
          transform: isMobile ? undefined : 'translateX(-50%)',
          width: '100%',
          top: isMobile ? undefined : 0,
          bottom: isMobile ? undefined : 0,
          display: 'flex',
          flexDirection: 'column',
          justifyContent: 'center',
          alignItems: isMobile ? 'stretch' : 'center',
          height: '100%'
        }}
      >
        {children}
      </div>
    );
  }
);
