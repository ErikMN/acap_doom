import React, { useState, useEffect } from 'react';
import PropTypes from 'prop-types';
import { getCgiResponse } from '../helpers/cgihelper.jsx';

const PARAMS_BASE_PATH = '/axis-cgi/param.cgi?action=list&group=';

const centerStyle = {
  display: 'flex',
  justifyContent: 'center',
  paddingTop: '5px',
  color: 'white'
};

const GetParam = ({ param }) => {
  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(false);

  useEffect(() => {
    const fetchData = async () => {
      setLoading(true);
      try {
        const resp = await getCgiResponse(`${PARAMS_BASE_PATH}${param}`);
        const parsedData = resp.substring(resp.indexOf('=') + 1);
        setData(parsedData);
      } catch (error) {
        console.error(error);
        setData('Error fetching data');
      } finally {
        setLoading(false);
      }
    };
    fetchData();
  }, [param]);

  return <>{!loading && <h4 style={centerStyle}>{data}</h4>}</>;
};

GetParam.propTypes = {
  param: PropTypes.string.isRequired
};

export default GetParam;
