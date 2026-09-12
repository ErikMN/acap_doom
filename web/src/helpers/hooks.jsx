/**
 * A collection of custom hooks.
 */
import React, { useCallback, useState, useEffect } from 'react';
/* MUI */
import { useMediaQuery } from '@mui/material';
import { useTheme } from '@mui/material/styles';

/**
 * Custom hook to get and set a value in localStorage.
 */
const useLocalStorage = (key, defaultValue, setItemIfNone = false) => {
  const [localStorageValue, setLocalStorageValue] = useState(() => {
    try {
      const value = localStorage.getItem(key);
      if (value !== null) {
        return JSON.parse(value);
      } else {
        if (setItemIfNone) {
          localStorage.setItem(key, JSON.stringify(defaultValue));
        }
        return defaultValue;
      }
    } catch (error) {
      console.error(
        `Failed to get or set localStorage item with key "${key}":`,
        error
      );
      try {
        localStorage.setItem(key, JSON.stringify(defaultValue));
      } catch (setItemError) {
        console.error(
          `Failed to recover localStorage item with key "${key}":`,
          setItemError
        );
      }
      return defaultValue;
    }
  });
  /* Keep the setter stable and always derive from the latest state value. */
  const setLocalStorageStateValue = useCallback(
    (valueOrFn) => {
      setLocalStorageValue((prevValue) => {
        const newValue =
          typeof valueOrFn === 'function' ? valueOrFn(prevValue) : valueOrFn;
        try {
          localStorage.setItem(key, JSON.stringify(newValue));
        } catch (error) {
          console.error(
            `Failed to set localStorage item with key "${key}":`,
            error
          );
        }
        return newValue;
      });
    },
    [key]
  );
  return [localStorageValue, setLocalStorageStateValue];
};

/**
 * Custom hook to debounce values at a delay.
 */
const useDebouncedValue = (value, delay) => {
  const [debouncedValue, setDebouncedValue] = useState(value);

  useEffect(() => {
    const handler = setTimeout(() => {
      setDebouncedValue(value);
    }, delay);

    return () => {
      clearTimeout(handler);
    };
  }, [value, delay]);

  return debouncedValue;
};

/**
 * Custom hook to check screen sizes.
 */
const useScreenSizes = () => {
  const theme = useTheme();
  /* Screen widths */
  const isMobile = useMediaQuery(theme.breakpoints.down('sm'));

  return { isMobile };
};

/**
 * Custom hook to handle tab visibility changes.
 */
const useTabVisibility = (callback) => {
  useEffect(() => {
    const handleVisibilityChange = async () => {
      if (document.visibilityState === 'visible') {
        await callback();
      }
    };
    document.addEventListener('visibilitychange', handleVisibilityChange);

    return () => {
      document.removeEventListener('visibilitychange', handleVisibilityChange);
    };
  }, [callback]);
};

const useSwitch = (initialValue = false) => {
  const [value, setValue] = useState(initialValue);

  const toggleValue = useCallback(
    (state) => {
      if (state !== undefined) {
        setValue(state);
      } else {
        setValue((oldValue) => !oldValue);
      }
    },
    [setValue]
  );

  return [value, toggleValue];
};

export {
  useLocalStorage,
  useDebouncedValue,
  useScreenSizes,
  useTabVisibility,
  useSwitch
};
