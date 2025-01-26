import React from 'react';
import AppVersion from './AppVersion';
import logo from '../assets/img/doom.png';
import { useGlobalContext } from './GlobalContext';
import { useScreenSizes } from '../helpers/hooks.jsx';
import { CustomBox, CustomButton } from './CustomComponents';
/* MUI */
import Box from '@mui/material/Box';
import Fade from '@mui/material/Fade';
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

  return (
    <Modal
      aria-labelledby="about-modal-title"
      aria-describedby="about-modal-description"
      open={open}
      onClose={handleClose}
      closeAfterTransition
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
            bgcolor: 'background.paper',
            boxShadow: 24,
            borderRadius: isMobile ? 0 : 1,
            overflowY: isMobile ? 'auto' : 'unset'
          }}
        >
          <img
            src={logo}
            alt="logo"
            style={{
              width: isMobile ? '80px' : '120px',
              marginBottom: '10px'
            }}
          />
          <Typography
            id="about-modal-title"
            variant="h6"
            component="h2"
            sx={{ fontFamily: 'Doom' }}
          >
            About {import.meta.env.VITE_WEBSITE_NAME}
          </Typography>
          <Typography
            id="about-modal-description"
            sx={{ marginTop: 2, marginBottom: 2 }}
          >
            Version: {import.meta.env.VITE_VERSION}
            {appSettings.debug ? <AppVersion /> : <br />}
            Copyright © {new Date().getFullYear()}
          </Typography>

          {/* License box */}
          <Box>
            <Box sx={{ textAlign: 'center' }}>
              <Typography variant="h6" sx={{ fontFamily: 'Doom' }}>
                License
              </Typography>
            </Box>
            {/* Scrollable license box */}
            <CustomBox
              sx={(theme) => ({
                maxHeight: '300px',
                overflowY: 'auto',
                border: `1px solid ${theme.palette.grey[600]}`,
                padding: 2,
                textAlign: 'left'
              })}
            >
              {/* Preserve newlines in license text */}
              <pre
                style={{
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
              sx={{
                width: 'auto',
                paddingX: 3,
                fontFamily: 'Doom',
                backgroundColor: '#b30000',
                color: '#ffcc00',
                '&:hover': {
                  backgroundColor: '#6d0000'
                }
              }}
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
