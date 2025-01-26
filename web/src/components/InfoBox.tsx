import React from 'react';
/* MUI */
import Box from '@mui/material/Container';
import CheckIcon from '@mui/icons-material/Check';
import InfoOutlinedIcon from '@mui/icons-material/InfoOutlined';
import LightbulbIcon from '@mui/icons-material/Lightbulb';
import List from '@mui/material/List';
import ListItem from '@mui/material/ListItem';
import ListItemIcon from '@mui/material/ListItemIcon';
import Typography from '@mui/material/Typography';
import { useTheme } from '@mui/material/styles';

/** Props for MyListItem */
interface MyListItemProps {
  primaryText: string;
  icon?: string | React.ReactElement;
  fontSize?: number;
}

/** Custom List Item Component */
const MyListItem: React.FC<MyListItemProps> = ({
  primaryText,
  icon,
  fontSize
}) => {
  const theme = useTheme();
  return (
    <ListItem
      disablePadding
      style={{
        display: 'flex',
        alignItems: 'flex-start',
        paddingTop: '10px'
      }}
    >
      <ListItemIcon
        style={{
          minWidth: '35px',
          alignSelf: 'flex-start'
        }}
      >
        {typeof icon === 'string' ? (
          <img
            src={icon}
            alt="Icon"
            style={{
              width: '24px',
              height: '24px',
              color: theme.palette.text.primary
            }}
          />
        ) : (
          icon && (
            <icon.type
              {...icon.props}
              sx={{ color: theme.palette.error.main }}
            />
          )
        )}
      </ListItemIcon>
      <Typography
        sx={{
          fontFamily: 'Doom',
          fontSize: fontSize || '16px',
          color: theme.palette.warning.main
        }}
      >
        {primaryText}
      </Typography>
    </ListItem>
  );
};

/** InfoBox Component */
const InfoBox: React.FC = () => {
  const theme = useTheme();

  return (
    <Box
      sx={{
        border: '1px solid',
        borderColor: theme.palette.error.main,
        borderRadius: '6px',
        color: theme.palette.warning.light,
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
        <LightbulbIcon sx={{ color: theme.palette.text.primary }} />
        <Typography
          sx={{
            paddingLeft: '10px',
            fontFamily: 'Doom',
            color: theme.palette.warning.main
          }}
        >
          TIPS
        </Typography>
      </div>
      <List>
        <MyListItem
          primaryText="Recommended image resolution: 1280x720"
          icon={<CheckIcon />}
        />
        <MyListItem
          primaryText="Recommended video compression: 20"
          icon={<CheckIcon />}
        />
        <MyListItem
          primaryText="Reload the page or video stream if the input is slow or unresponsive."
          icon={<CheckIcon />}
        />
        <MyListItem
          primaryText="Avoid running other CPU-intensive applications or continuous recordings."
          icon={<CheckIcon />}
        />
        <MyListItem
          primaryText="Game controls only work on this page."
          icon={<InfoOutlinedIcon />}
        />
        <MyListItem
          primaryText="Plug speaker or headphones to the audio output on your device."
          icon={<InfoOutlinedIcon />}
          fontSize={12}
        />
      </List>
    </Box>
  );
};

export default InfoBox;
