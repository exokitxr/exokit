import React from 'react';
import '../css/console.css';

class Console extends React.Component {
  constructor(props) {
    super(props);

    this.state = {
      open: this.props.open,
    };
  }

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
    return true;
  }

  componentDidUpdate(prevProps, prevState) {
    if (this.state.open !== prevState.open) {
      this.setState({
        open: prevState.open,
      });
    }
  }

  getClassnames() {
    const classNames = ['Console'];
    if (this.state.open) {
      classNames.push('open');
    }
    return classNames.join(' ');
  }

  static getDerivedStateFromProps(nextProps, prevState) {
    return {
      open: nextProps.open,
    };
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
      <div className={this.getClassnames()} id="console">
        <div className="console-output" id="console-output" />
        <input type="text" className="console-input" id="console-input" onKeyDown={e => this.onKeyDown(e)} />
      </div>
    );
  }
}

export default Console;
