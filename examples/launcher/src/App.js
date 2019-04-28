import React from 'react';
import './css/App.css';
import NavTabs from './components/NavTabs.jsx';
import Update from './components/Update.jsx'
import Launch from './components/Launch';
import Social from './components/Social';

function App() {
  return (
    <div className="App">
      <NavTabs/>
      <Update/>
      <Launch/>
      <Social/>
    </div>
  );
}

export default App;
