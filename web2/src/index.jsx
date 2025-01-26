import React from 'react';
import ReactDOM from 'react-dom/client';
import './assets/css/index.css';
import RootComponent from './components/RootComponent.jsx';

const root = ReactDOM.createRoot(document.getElementById('root'));
root.render(
  <React.StrictMode>
    <RootComponent />
  </React.StrictMode>
);
