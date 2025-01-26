import { HashRouter, Routes, Route } from 'react-router-dom';
import LoadingScreen from './LoadingScreen';

/* Components */
import App from './App';

const AppRoutes = () => {
  return (
    <HashRouter>
      <Routes>
        {/* NOTE: Add routes here: */}
        <Route path="/" element={<LoadingScreen Component={App} />} />
        <Route path="*" element={<LoadingScreen Component={App} />} />
      </Routes>
    </HashRouter>
  );
};

export default AppRoutes;
