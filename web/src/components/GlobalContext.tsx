/* Application global context */
import React, { createContext, useContext, useState } from 'react';
import { useLocalStorage } from '../helpers/hooks.jsx';
import { log, enableLogging } from '../helpers/logger.js';
import { AppSettings, defaultAppSettings } from './commonInterfaces';

/* Interface defining the structure of the context */
interface GlobalContextProps {
  appLoading: boolean;
  setAppLoading: React.Dispatch<React.SetStateAction<boolean>>;

  /* Alert handling */
  handleOpenAlert: (
    content: string,
    severity: 'info' | 'success' | 'error' | 'warning'
  ) => void;

  /* UI-related state */
  openDropdownIndex: number | null;
  setOpenDropdownIndex: React.Dispatch<React.SetStateAction<number | null>>;

  /* Alert-related state */
  openAlert: boolean;
  setOpenAlert: React.Dispatch<React.SetStateAction<boolean>>;
  alertContent: string;
  setAlertContent: React.Dispatch<React.SetStateAction<string>>;
  alertSeverity: 'info' | 'success' | 'error' | 'warning';
  setAlertSeverity: React.Dispatch<
    React.SetStateAction<'info' | 'success' | 'error' | 'warning'>
  >;

  /* Theme-related state */
  currentTheme: string;
  setCurrentTheme: React.Dispatch<React.SetStateAction<string>>;

  /* Global settings for the application */
  appSettings: AppSettings;
  setAppSettings: React.Dispatch<React.SetStateAction<AppSettings>>;
  jsonTheme: undefined;
  setJsonTheme: React.Dispatch<React.SetStateAction<string>>;
}

/* Creating the global context */
const GlobalContext = createContext<GlobalContextProps | undefined>(undefined);

export const GlobalProvider: React.FC<{ children: React.ReactNode }> = ({
  children
}) => {
  const [appLoading, setAppLoading] = useState<boolean>(false);
  const [openDropdownIndex, setOpenDropdownIndex] = useState<number | null>(
    null
  );

  /* Alert-related state variables */
  const [openAlert, setOpenAlert] = useState<boolean>(false);
  const [alertContent, setAlertContent] = useState<string>('');
  const [alertSeverity, setAlertSeverity] = useState<
    'info' | 'success' | 'error' | 'warning'
  >('info');

  /* Local storage state */
  const [currentTheme, setCurrentTheme] = useLocalStorage('theme', 'dark');
  const [appSettings, setAppSettings] = useLocalStorage(
    'appSettings',
    defaultAppSettings
  );
  const [jsonTheme, setJsonTheme] = useLocalStorage('jsonTheme', 'monokai');

  /* Disabling logging by default, but can be enabled as needed */
  enableLogging(false);

  /* Function to open an alert with content and severity */
  const handleOpenAlert = (
    content: string,
    severity: 'info' | 'success' | 'error' | 'warning'
  ) => {
    setAlertContent(content);
    setAlertSeverity(severity);
    setOpenAlert(true);
  };

  /****************************************************************************/

  return (
    <GlobalContext.Provider
      value={{
        appLoading,
        setAppLoading,
        openDropdownIndex,
        setOpenDropdownIndex,
        handleOpenAlert,
        openAlert,
        setOpenAlert,
        alertContent,
        setAlertContent,
        alertSeverity,
        setAlertSeverity,
        currentTheme,
        setCurrentTheme,
        appSettings,
        setAppSettings,
        jsonTheme,
        setJsonTheme
      }}
    >
      {children}
    </GlobalContext.Provider>
  );
};

/* Hook to use the GlobalContext, with an error if used outside the provider */
export const useGlobalContext = () => {
  const context = useContext(GlobalContext);
  if (context === undefined) {
    throw new Error('useGlobalContext must be used within a GlobalProvider');
  }
  return context;
};
