import React from 'react';
import PropTypes from 'prop-types';

/* MUI */
import Button from '@mui/material/Button';
import IconButton from '@mui/material/IconButton';
import Typography from '@mui/material/Typography';
import CloseIcon from '@mui/icons-material/Close';
import Dialog from '@mui/material/Dialog';
import DialogTitle from '@mui/material/DialogTitle';
import DialogContent from '@mui/material/DialogContent';
import DialogActions from '@mui/material/DialogActions';

const PopupBox = ({ title, open, handleClose, text }) => {
  return (
    <Dialog
      maxWidth="md"
      onClose={handleClose}
      aria-labelledby="text"
      open={open}
    >
      <DialogTitle
        sx={{
          m: 0,
          p: 2,
          backgroundColor: 'black',
          color: 'white',
          fontSize: '24px',
          fontFamily: 'Doom'
        }}
        id="text"
      >
        {title}
      </DialogTitle>
      <IconButton
        aria-label="close"
        onClick={handleClose}
        sx={{
          position: 'absolute',
          right: 8,
          top: 8
        }}
      >
        <CloseIcon sx={{ color: 'white' }} />
      </IconButton>
      <DialogContent
        dividers
        sx={{
          padding: 2,
          backgroundColor: '#1a0000',
          color: 'white',
          overflowY: 'auto',
          maxHeight: '60vh',
          '&::-webkit-scrollbar': {
            width: '8px'
          },
          '&::-webkit-scrollbar-thumb': {
            backgroundColor: 'rgba(255, 255, 255, 0.5)',
            borderRadius: '4px'
          },
          '&::-webkit-scrollbar-track': {
            backgroundColor: 'transparent'
          },
          '&:hover': {
            '&::-webkit-scrollbar-thumb': {
              backgroundColor: 'rgba(255, 255, 255, 0.7)'
            }
          },
          '&.Mui-focused': {
            '&::-webkit-scrollbar-thumb': {
              backgroundColor: 'rgba(220, 20, 60, 0.7)'
            }
          }
        }}
      >
        <Typography gutterBottom>
          {/* Preserve newlines in body text */}
          {text.split('\n').map((line, index) => (
            <React.Fragment key={index}>
              {line}
              <br />
            </React.Fragment>
          ))}
        </Typography>
      </DialogContent>
      <DialogActions
        sx={{
          backgroundColor: 'black',
          color: 'white'
        }}
      >
        <Button
          autoFocus
          sx={{
            backgroundColor: '#660000',
            color: 'white',
            '&:hover': {
              backgroundColor: '#6d0000'
            }
          }}
          onClick={handleClose}
        >
          Close
        </Button>
      </DialogActions>
    </Dialog>
  );
};

PopupBox.propTypes = {
  title: PropTypes.string,
  open: PropTypes.bool,
  handleClose: PropTypes.func,
  text: PropTypes.string
};

export default PopupBox;
