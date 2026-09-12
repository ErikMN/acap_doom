import React, { PropsWithChildren } from 'react';
import { useScreenSizes } from '../../helpers/hooks.jsx';

/**
 * Aspect ratio and layout behavior
 *
 * This container enforces the visible video area to always remain fully visible,
 * while preserving the stream’s aspect ratio.
 *
 * Desktop layout:
 *  - Aspect ratio is enforced using the classic padding-top technique.
 *  - The container width is 100%, and height is derived from the aspect ratio.
 *  - Letterboxing/pillarboxing happens *inside* the video element if needed.
 *
 * Mobile layout:
 *  - The container fills the available height (height: 100%).
 *  - No padding-top is used.
 *  - Centering is handled by the Layer component using flexbox.
 *
 * In both cases:
 *  - The video content is never clipped.
 *  - The visible video area is fully contained within the container.
 *
 * Desktop aspect ratio math:
 *
 *          +---- 100% -----+
 *          |               |
 *   video  |               |
 *   height |               | (100 / (AR))%
 *          |               |
 *          +---------------+
 *             video width
 *
 * AR = width / height
 * padding-top = (1 / AR) * 100%
 */

// Default aspect ratio is fixed to 16:9, but can be modified by changing the
// aspectRatio property on the Container component.
const DEFAULT_ASPECT_RATIO = 16 / 9;

const getHeightPct = (aspectRatio: number) => {
  if (aspectRatio === 0) {
    throw new Error('Cannot handle aspect ratio 0');
  }
  return 100 / aspectRatio;
};

/**
 * Layer
 *
 * A positioning layer that spans the full container area and hosts playback,
 * and UI elements.
 *
 * Desktop:
 *  - Uses absolute positioning to exactly match the aspect-ratio–controlled
 *    container.
 *
 * Mobile:
 *  - Uses flexbox to center playback content within the available height.
 *  - This ensures the video remains fully visible even when height is constrained.
 *
 * Layer itself does not perform any aspect-ratio calculations; it relies on
 * Container to define the visible area.
 */
export const Layer = ({ children }: PropsWithChildren) => {
  const { isMobile } = useScreenSizes();

  return (
    <div
      style={{
        position: 'absolute',
        top: '0',
        left: '0',
        bottom: '0',
        right: '0',
        /* Mobile-only centering of playback content */
        display: isMobile ? 'flex' : undefined,
        alignItems: isMobile ? 'center' : undefined,
        justifyContent: isMobile ? 'center' : undefined
      }}
    >
      {children}
    </div>
  );
};

interface ContainerProps {
  readonly aspectRatio?: number;
}

/**
 * Container
 *
 * Defines the layout box that the player and its controls render into.
 * This component is responsible for keeping the visible video area fully
 * visible while preserving the intended aspect ratio, but it does so
 * differently depending on device mode.
 *
 * Desktop:
 *  - Enforces aspect ratio using the padding-top technique.
 *  - width: 100% and paddingTop create a height that is a percentage of width.
 *  - The container height is therefore derived from the given aspectRatio.
 *
 * Mobile:
 *  - Fills the available vertical space (height: 100%).
 *  - Does not apply padding-top (aspect ratio is not enforced here).
 *  - Playback centering and size constraints are handled by Layer and parent
 *    layout components.
 */
export const Container = ({
  aspectRatio = DEFAULT_ASPECT_RATIO,
  children
}: PropsWithChildren<ContainerProps>) => {
  const { isMobile } = useScreenSizes();

  return (
    <div
      style={{
        background: 'black',
        paddingTop: isMobile ? undefined : `${getHeightPct(aspectRatio)}%`,
        position: 'relative',
        width: '100%',
        height: isMobile ? '100%' : undefined
      }}
    >
      {children}
    </div>
  );
};
