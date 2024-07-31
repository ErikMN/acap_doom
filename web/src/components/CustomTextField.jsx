import React from 'react';
import PropTypes from 'prop-types';

/* MUI */
import TextField from '@mui/material/TextField';

const CustomTextField = ({ label, value, textFieldRef }) => {
  return (
    <TextField
      id="outlined-multiline-static"
      label={label}
      defaultValue=""
      multiline
      rows={24}
      value={value}
      variant="outlined"
      inputRef={textFieldRef}
      InputProps={{
        style: { fontSize: '12px', color: '#ffffff' }
      }}
      InputLabelProps={{
        style: { color: 'white' }
      }}
      sx={{
        width: '100%',
        backgroundColor: 'black',
        fontSize: '12px',
        color: 'white',
        '& .MuiOutlinedInput-root': {
          '& fieldset': {
            borderColor: '#ff0000'
          },
          '&:hover fieldset': {
            borderColor: 'red'
          },
          '&.Mui-focused fieldset': {
            borderColor: 'crimson'
          }
        },
        '& .MuiInputBase-input': {
          '&::-webkit-scrollbar': {
            width: '0px'
          },
          '&:hover': {
            '&::-webkit-scrollbar': {
              width: '10px'
            },
            '&::-webkit-scrollbar-thumb': {
              backgroundColor: 'rgba(255,255,255,0.5)',
              borderRadius: '10px'
            },
            '&::-webkit-scrollbar-track': {
              backgroundColor: 'transparent'
            }
          }
        }
      }}
    />
  );
};

CustomTextField.propTypes = {
  label: PropTypes.string.isRequired,
  value: PropTypes.string.isRequired,
  textFieldRef: PropTypes.object.isRequired
};

export default CustomTextField;
