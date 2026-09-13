/**
 * AboutModal
 *
 * This component displays an "About" modal dialog with application
 * information, including version, license, and link to GitHub.
 */
import React from 'react';
import AppVersion from './AppVersion';
import logo from '../assets/img/doom.png';
import github_logo_white from '../assets/img/github-mark-white.svg';
import { useGlobalContext } from './context/GlobalContext.js';
import { useScreenSizes } from '../helpers/hooks.jsx';
import { CustomBox, CustomButton } from './CustomComponents';
/* MUI */
import { useTheme } from '@mui/material/styles';
import Box from '@mui/material/Box';
import BuildIcon from '@mui/icons-material/Build';
import Chip from '@mui/material/Chip';
import Fade from '@mui/material/Fade';
import MuiLink from '@mui/material/Link';
import Modal from '@mui/material/Modal';
import Typography from '@mui/material/Typography';

import license from '../assets/etc/LICENSE?raw';

interface AboutModalProps {
  open: boolean;
  handleClose: () => void;
}

const AboutModal: React.FC<AboutModalProps> = ({ open, handleClose }) => {
  /* Screen size */
  const { isMobile } = useScreenSizes();

  /* Global context */
  const { appSettings } = useGlobalContext();

  const theme = useTheme();

  return (
    <Modal
      aria-labelledby="about-modal-title"
      aria-describedby="about-modal-description"
      open={open}
      onClose={handleClose}
      closeAfterTransition
      sx={{ zIndex: 2000 }}
    >
      <Fade in={open}>
        <Box
          sx={{
            p: 2,
            position: 'absolute',
            textAlign: 'center',
            top: isMobile ? 0 : '50%',
            left: isMobile ? 0 : '50%',
            transform: isMobile ? 'none' : 'translate(-50%, -50%)',
            width: isMobile
              ? '100%'
              : { xs: '90%', sm: '80%', md: '60%', lg: '50%', xl: '40%' },
            height: isMobile ? '100%' : 'auto',
            maxWidth: '800px',
            minWidth: '300px',
            maxHeight: isMobile ? '100%' : '90vh',
            overflowY: 'auto',
            bgcolor:
              theme.palette.mode === 'dark' ? 'secondary.main' : 'primary.main',
            border: isMobile
              ? 'none'
              : theme.palette.mode === 'dark'
                ? `1px solid ${theme.palette.grey[700]}`
                : `2px solid ${theme.palette.warning.main}`,
            boxShadow:
              theme.palette.mode === 'dark'
                ? '0 24px 64px rgba(0, 0, 0, 0.75)'
                : '0 24px 64px rgba(0, 0, 0, 0.45)',
            borderRadius: isMobile ? 0 : 1
          }}
        >
          <img
            src={logo}
            alt="DOOM logo"
            style={{
              width: isMobile ? '80px' : '120px',
              marginBottom: '10px'
            }}
          />
          <Typography
            color="text.primary"
            id="about-modal-title"
            variant="h5"
            component="h2"
            sx={{ fontFamily: 'Doom' }}
          >
            About {import.meta.env.VITE_WEBSITE_NAME}
          </Typography>

          {/* Version info */}
          <Typography
            id="about-modal-description"
            color="text.primary"
            sx={{ marginTop: 2, marginBottom: 2 }}
          >
            Version: {import.meta.env.VITE_VERSION}
            <br />
            {appSettings.debug ? (
              <>
                <Chip
                  color="warning"
                  size="small"
                  label={<AppVersion />}
                  icon={<BuildIcon />}
                  sx={{ mt: 1, mb: 1 }}
                />
                <br />
              </>
            ) : null}
            Copyright © {new Date().getFullYear()}{' '}
            {import.meta.env.VITE_WEBSITE_NAME}
          </Typography>

          {/* GitHub link */}
          <Box
            sx={{
              display: 'flex',
              flexDirection: 'column',
              alignItems: 'center',
              mt: 2
            }}
          >
            <MuiLink
              href="https://github.com/ErikMN/acap_doom"
              target="_blank"
              rel="noopener noreferrer"
              underline="none"
              aria-label="View acap_doom source code on GitHub"
              color="text.primary"
              sx={{
                display: 'inline-flex',
                flexDirection: 'column',
                alignItems: 'center',
                '& img': {
                  width: '50px',
                  height: '50px',
                  cursor: 'pointer'
                }
              }}
            >
              <img src={github_logo_white} alt="GitHub logo" />
              <Typography variant="caption" sx={{ mt: 1 }}>
                View on GitHub
              </Typography>
            </MuiLink>
          </Box>

          {/* License box */}
          <Box>
            <Box sx={{ textAlign: 'center', mt: 2 }}>
              <Typography
                color="text.primary"
                variant="h6"
                sx={{ fontFamily: 'Doom' }}
              >
                License
              </Typography>
            </Box>
            {/* Scrollable license box */}
            <CustomBox
              sx={(theme) => ({
                maxHeight: '300px',
                overflowY: 'auto',
                border:
                  theme.palette.mode === 'dark'
                    ? `1px solid ${theme.palette.grey[600]}`
                    : `1px solid ${theme.palette.warning.main}`,
                borderRadius: 1,
                padding: 2,
                textAlign: 'left',
                bgcolor: 'background.default',
                color: 'text.primary'
              })}
            >
              {/* Preserve newlines in license text */}
              <pre
                style={{
                  margin: 0,
                  fontFamily: 'inherit',
                  whiteSpace: 'pre-wrap',
                  wordWrap: 'break-word'
                }}
              >
                {license}
              </pre>
            </CustomBox>
          </Box>

          {/* Close button */}
          <Box sx={{ display: 'flex', justifyContent: 'center', marginTop: 2 }}>
            <CustomButton
              onClick={handleClose}
              sx={(theme) => ({
                width: 'auto',
                paddingX: 3,
                fontFamily: 'Doom',
                ...(theme.palette.mode !== 'dark' && {
                  backgroundColor: 'secondary.main',
                  color: 'background.default',
                  '&:hover': {
                    backgroundColor: 'warning.light'
                  }
                })
              })}
              variant="contained"
            >
              Close
            </CustomButton>
          </Box>
        </Box>
      </Fade>
    </Modal>
  );
};

export default AboutModal;
