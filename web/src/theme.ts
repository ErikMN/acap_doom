/**
 * This file contains the theme configuration for the app.
 */
import { createTheme } from '@mui/material/styles';
import { grey } from '@mui/material/colors';

const doomTheme = createTheme({
  palette: {
    mode: 'light',
    primary: {
      main: '#b30000', // Primary color (buttons, app bar, etc.)
      contrastText: '#fff' // Text color on primary elements
    },
    secondary: {
      main: '#ffcc00' // Secondary color
    },
    background: {
      default: '#6d0000', // Background color for the whole app
      paper: '#6d0000' // Background color for cards, dialogs, etc.
    },
    text: {
      primary: '#ffffff', // Text color for the default dark background
      secondary: '#ffcc00' // Lighter text color
    },
    error: {
      main: '#d32f2f' // Error color
    },
    warning: {
      main: '#ffcc00' // Warning color
    },
    info: {
      main: '#29b6f6' // Info color
    },
    success: {
      main: '#66bb6a' // Success color
    }
  },
  typography: {
    fontFamily: 'Segoe UI, Roboto, Arial, sans-serif',
    h1: {
      fontSize: '2.5rem',
      fontWeight: 700
    },
    h5: {
      color: '#ffcc00',
      fontSize: '22px'
    },
    h6: {
      color: '#ffcc00',
      fontSize: '16px',
      fontWeight: 500
    },
    body1: {
      fontSize: '0.875rem',
      lineHeight: 1.5
    },
    button: {
      textTransform: 'none' // Disable uppercase transformation for buttons
    }
  },
  shape: {
    borderRadius: 8 // Default border-radius for components (e.g., buttons, cards)
  },
  components: {
    MuiCssBaseline: {
      styleOverrides: (theme) => ({
        body: {
          scrollbarColor: `${theme.palette.grey[500]} ${theme.palette.background.default}`,
          scrollbarWidth: 'thin'
        },
        '::-webkit-scrollbar': {
          width: '8px',
          height: '8px'
        },
        '::-webkit-scrollbar-thumb': {
          backgroundColor: theme.palette.grey[400],
          borderRadius: '4px'
        },
        '::-webkit-scrollbar-track': {
          backgroundColor: theme.palette.background.default
        }
      })
    },
    MuiButton: {
      styleOverrides: {
        root: {
          borderRadius: 8 // Rounded button edges
        },
        contained: {
          boxShadow: '0px 3px 6px rgba(0, 0, 0, 0.2)' // Custom shadow for contained buttons
        }
      }
    },
    MuiAppBar: {
      styleOverrides: {
        colorPrimary: {
          backgroundColor: '#ffcc33' // AppBar color
        }
      }
    },
    MuiCard: {
      styleOverrides: {
        root: {
          boxShadow: '0px 3px 10px rgba(0, 0, 0, 0.1)', // Custom card shadow
          borderRadius: 8 // Rounded corners for cards
        }
      }
    }
  }
});

const darkTheme = createTheme({
  palette: {
    mode: 'dark',
    primary: {
      main: '#ffa726', // Primary color (buttons, app bar, etc.)
      contrastText: '#fff' // Text color on primary elements
    },
    secondary: {
      main: '#292929' // Secondary color
    },
    background: {
      default: '#151515', // Background color for the whole app
      paper: '#000000' // Background color for cards, dialogs, etc.
    },
    text: {
      primary: '#fff', // Text color for the default dark background
      secondary: '#d6d6d6' // Lighter text color
    },
    error: {
      main: '#f44336' // Error color
    },
    warning: {
      main: '#ffa726' // Warning color
    },
    info: {
      main: '#29b6f6' // Info color
    },
    success: {
      main: '#66bb6a' // Success color
    }
  },
  typography: {
    fontFamily: 'Segoe UI, Roboto, Arial, sans-serif',
    h1: {
      fontSize: '2.5rem',
      fontWeight: 700
    },
    h5: {
      color: '#b8b8b8',
      fontSize: '22px'
    },
    h6: {
      color: '#b8b8b8',
      fontSize: '16px',
      fontWeight: 500
    },
    body1: {
      fontSize: '0.875rem',
      lineHeight: 1.5
    },
    button: {
      textTransform: 'none' // Disable uppercase transformation for buttons
    }
  },
  shape: {
    borderRadius: 8 // Default border-radius for components (e.g., buttons, cards)
  },
  components: {
    MuiCssBaseline: {
      styleOverrides: (theme) => ({
        body: {
          scrollbarColor: `${theme.palette.grey[500]} ${theme.palette.background.default}`,
          scrollbarWidth: 'thin'
        },
        '::-webkit-scrollbar': {
          width: '8px',
          height: '8px'
        },
        '::-webkit-scrollbar-thumb': {
          backgroundColor: theme.palette.grey[700],
          borderRadius: '4px'
        },
        '::-webkit-scrollbar-track': {
          backgroundColor: theme.palette.background.default
        }
      })
    },
    MuiButton: {
      styleOverrides: {
        root: {
          borderRadius: 8 // Rounded button edges
        },
        contained: {
          boxShadow: '0px 3px 6px rgba(0, 0, 0, 0.2)' // Custom shadow for contained buttons
        }
      }
    },
    MuiAppBar: {
      styleOverrides: {
        colorPrimary: {
          backgroundColor: grey[800] // Dark AppBar color
        }
      }
    },
    MuiCard: {
      styleOverrides: {
        root: {
          boxShadow: '0px 3px 10px rgba(0, 0, 0, 0.1)', // Custom card shadow
          borderRadius: 8 // Rounded corners for cards
        }
      }
    }
  }
});

export { doomTheme, darkTheme };
