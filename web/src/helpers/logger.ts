const isProduction = import.meta.env.MODE === 'production';
let isLoggingEnabled = false;

export const enableLogging = (enable: boolean) => {
  isLoggingEnabled = enable;
};

export const log = (...args: (number | string | object)[]) => {
  if (isLoggingEnabled && !isProduction) {
    console.log(...args);
  }
};
