import React from 'react';
import './css/App.css';
import Engine from './components/Engine.jsx'

document.oncontextmenu = function() {
    return false;
}
function App() {
  return (
    <div className="App" id="app">
      <Engine/>
    </div>
  );
}

export default App;
