import React from 'react';
import PropTypes from 'prop-types';
import '../assets/css/App.css';

/* MUI */
import Box from '@mui/material/Container';
import Typography from '@mui/material/Typography';
import ListItemIcon from '@mui/material/ListItemIcon';
import List from '@mui/material/List';
import ListItem from '@mui/material/ListItem';
import LightbulbIcon from '@mui/icons-material/Lightbulb';
import CheckIcon from '@mui/icons-material/Check';
import InfoOutlinedIcon from '@mui/icons-material/InfoOutlined';

function MyListItem({ primaryText, icon, fontSize }) {
  return (
    <ListItem
      disablePadding
      style={{ display: 'flex', alignItems: 'flex-start', paddingTop: '10px' }}
    >
      <ListItemIcon style={{ minWidth: '35px', alignSelf: 'flex-start' }}>
        {typeof icon === 'string' ? (
          <img
            src={icon}
            alt="Icon"
            style={{ width: '24px', height: '24px', color: 'white' }}
          />
        ) : (
          icon && <icon.type {...icon.props} sx={{ color: 'white' }} />
        )}
      </ListItemIcon>
      <Typography
        sx={{
          fontFamily: 'Doom',
          fontSize: fontSize || '16px'
        }}
      >
        {primaryText}
      </Typography>
    </ListItem>
  );
}

MyListItem.propTypes = {
  primaryText: PropTypes.string.isRequired,
  icon: PropTypes.oneOfType([PropTypes.string, PropTypes.element]),
  fontSize: PropTypes.number
};

const InfoBox = () => {
  return (
    <Box
      sx={{
        // backgroundColor: '#2a2a2a',
        border: '1px solid',
        borderColor: '#ff0000 !important',
        borderRadius: '6px',
        color: '#ffcc00',
        paddingLeft: '18px !important',
        paddingRight: '18px !important',
        paddingBottom: '10px'
      }}
    >
      <div
        style={{
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          paddingTop: '12px'
        }}
      >
        <LightbulbIcon sx={{ color: 'white' }} />
        <Typography sx={{ paddingLeft: '10px', fontFamily: 'Doom' }}>
          TIPS
        </Typography>
      </div>
      <List>
        <MyListItem
          primaryText="Recommended image resolution: 1280x720"
          icon={CheckIcon}
        />
        <MyListItem
          primaryText="Recommended video compression: 20"
          icon={CheckIcon}
        />
        <MyListItem
          primaryText="Reload the page or video stream if the input is slow or unresponsive."
          icon={CheckIcon}
        />
        <MyListItem
          primaryText="Avoid running other CPU-intensive applications or continuous recordings."
          icon={CheckIcon}
        />
        <MyListItem
          primaryText="Game controls only work on this page."
          icon={InfoOutlinedIcon}
        />
        <MyListItem
          primaryText="To enable audio: Video -> Stream -> Audio AND Audio -> Device Settings -> Enable input"
          icon={InfoOutlinedIcon}
          fontSize={12}
        />
      </List>
    </Box>
  );
};

export default InfoBox;
