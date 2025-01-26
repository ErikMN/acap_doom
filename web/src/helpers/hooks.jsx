/**
 * A collection of custom hooks.
 */

import React, { useState, useEffect } from 'react';
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
      if (value) {
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
      localStorage.setItem(key, JSON.stringify(defaultValue));
      return defaultValue;
    }
  });
  const setLocalStorageStateValue = (valueOrFn) => {
    let newValue;
    if (typeof valueOrFn === 'function') {
      const fn = valueOrFn;
      newValue = fn(localStorageValue);
    } else {
      newValue = valueOrFn;
    }
    localStorage.setItem(key, JSON.stringify(newValue));
    setLocalStorageValue(newValue);
  };
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

export { useLocalStorage, useDebouncedValue, useScreenSizes, useTabVisibility };
