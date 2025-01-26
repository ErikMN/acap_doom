import React, { useState, useEffect } from 'react';
import { getCgiResponse } from '../helpers/cgihelper';

const PARAMS_BASE_PATH = '/axis-cgi/param.cgi?action=list&group=';

interface GetParamProps {
  param: string;
}

const GetParam: React.FC<GetParamProps> = ({ param }) => {
  const [data, setData] = useState<string | null>(null);
  const [loading, setLoading] = useState<boolean>(false);

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

  return <>{!loading && <>{data}</>}</>;
};

export default GetParam;
