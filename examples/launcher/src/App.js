import React from 'react';
import './css/App.css';
import Engine from './components/Engine.jsx'

function App() {
  return (
    <div className="App" id="app" onContextMenu={(e) => {e.preventDefault(); return false;}} >
      <Engine/>
    </div>
  );
}

export default App;
