import React, { useState, useEffect, Suspense } from 'react';
import App from './App';
import { getParam } from '../helpers/cgihelper.jsx';

function Loading() {
  return <h3 style={{ color: 'white', fontFamily: 'Doom' }}>Loading...</h3>;
}

const RootComponent = () => {
  /* Local states */
  const [paramData, setParamData] = useState(null);

  useEffect(() => {
    const fetchData = async () => {
      const data = await getParam('Brand.ProdFullName');
      setParamData(data);
    };

    fetchData();
  }, []);

  return (
    <Suspense fallback={<Loading />}>
      <App paramData={paramData} />;
    </Suspense>
  );
};

export default RootComponent;
