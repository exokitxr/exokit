import React from 'react';
import '../css/console.css';

class Console extends React.Component {
  /* constructor(props) {
    super(props);
  } */

  componentDidMount() {
    const consoleWrap = document.getElementById('console');
    const consoleOuput = document.getElementById('console-output');

    window.addEventListener('message', e => {
      const m = e.data;
      if (m.method === 'terminal') {
        if (consoleOuput.textContent) {
          consoleOuput.textContent += '\n';
        }
        consoleOuput.textContent += m.output;
        consoleWrap.scrollTop = consoleWrap.scrollHeight;
      }
    });
  }
  
  shouldComponentUpdate() {
    return false;
  }

  onKeyDown(e) {
    if (e.keyCode === 13) { // enter
      const consoleInput = document.getElementById('console-input');
      const value = consoleInput.value;
      consoleInput.value = '';
      
      window.postMessage({
        method: 'eval',
        jsString: value,
      });
    }
  }

  render() {
    return (
      <div className="Console" id="console">
        <div className="console-output" id="console-output" />
        <input type="text" className="console-input" id="console-input" onKeyDown={e => this.onKeyDown(e)} />
      </div>
    );
  }
}

export default Console;