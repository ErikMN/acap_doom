import React, { createContext, useContext, useEffect, useState } from 'react';
import { P_CGI } from './constants';

interface ParametersContextType {
  parameters: { [key: string]: string } | null;
  paramsLoading: boolean;
  error: string | null;
  fetchParameters: () => Promise<void>;
}

const ParametersContext = createContext<ParametersContextType | undefined>(
  undefined
);

export const ParametersProvider: React.FC<{ children: React.ReactNode }> = ({
  children
}) => {
  /* Local state */
  const [parameters, setParameters] = useState<{
    [key: string]: string;
  } | null>(null);
  const [paramsLoading, setParamsLoading] = useState<boolean>(true);
  const [error, setError] = useState<string | null>(null);

  const parseParameters = (text: string): { [key: string]: string } => {
    const paramMap: { [key: string]: string } = {};
    const lines = text.split('\n');
    for (const line of lines) {
      const [key, value] = line.split('=');
      if (key) {
        paramMap[key.trim()] = value?.trim() || '';
      }
    }
    return paramMap;
  };

  const fetchParameters = async () => {
    setParamsLoading(true);
    try {
      const response = await fetch(P_CGI);
      if (!response.ok) {
        throw new Error(`Error fetching parameters: ${response.statusText}`);
      }
      const text = await response.text();
      const parsedParameters = parseParameters(text);
      setParameters(parsedParameters);
      setError(null);
    } catch (err) {
      if (err instanceof Error) {
        setError(err.message);
      } else {
        setError('An unknown error occurred.');
      }
    } finally {
      setParamsLoading(false);
    }
  };

  /* Fetch all parameters on mount */
  useEffect(() => {
    fetchParameters();
  }, []);

  return (
    <ParametersContext.Provider
      value={{ parameters, paramsLoading, error, fetchParameters }}
    >
      {children}
    </ParametersContext.Provider>
  );
};

export const useParameters = (): ParametersContextType => {
  const context = useContext(ParametersContext);
  if (!context) {
    throw new Error('useParameters must be used within a ParametersProvider');
  }
  return context;
};
