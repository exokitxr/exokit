import React from 'react';
import '../hterm_all.js';
import '../css/console.css';

class Console extends React.Component {
  /* constructor(props) {
    super(props);
  } */

  componentDidMount() {
    // setTimeout(() => {
      window.hterm.defaultStorage = new window.lib.Storage.Local();
      window.hterm.messageManager = new window.lib.MessageManager([]);
      const term = new window.hterm.Terminal();
      
      // term.keyboard.installKeyboard(window.document);
      // term.keyboard.installKeyboard(el);
      // el.focus();

      term.setAutoCarriageReturn(true);

      // term.setCursorPosition(0, 0);
      // term.setCursorVisible(true);
      term.prefs_.set('ctrl-c-copy', true);
      term.prefs_.set('ctrl-v-paste', true);
      term.prefs_.set('use-default-window-copy', true);
      // term.prefs_.set('send-encoding', 'raw');
      // term.prefs_.set('receive-encoding', 'raw');
      term.prefs_.set('font-size', 13);
      term.prefs_.set('cursor-color', '#FFF');
      term.prefs_.set('scrollbar-visible', false);

      const el = document.getElementById('terminal');
      term.decorate(el);

      term.scrollPort_.screen_.setAttribute('spellcheck', 'false');
      term.scrollPort_.screen_.setAttribute('autocorrect', 'false');
      term.scrollPort_.screen_.setAttribute('autocomplete', 'false');
      term.scrollPort_.screen_.setAttribute('contenteditable', 'false');
      
      class CommandClass {
        run() {}
      }
      term.runCommandClass(CommandClass, document.location.hash.substr(1));
      
      // console.log('got open');
      // term.runCommandClass(Wetty, document.location.hash.substr(1));
      console.log(JSON.stringify({
        method: 'resize',
        args: {
          col: term.screenSize.width,
          row: term.screenSize.height,
        },
      }));

      /* if (buf && buf !== '') {
        term.io.writeUTF8(buf);
        buf = '';
      } */

      const _send = s => {
        console.log('data out', JSON.stringify({
          method: 'c',
          args: s,
        }));
      };
      term.io.onVTKeystroke = _send;
      term.io.sendString = _send;

      const _resize = (col, row) => {
        console.log(JSON.stringify({
          method: 'resize',
          args: {
            col,
            row,
          },
        }));
      };
      term.io.onTerminalResize = _resize;

      window.addEventListener('keypress', e => {
        console.log('data in', e.key);
        term.io.writeUTF8(e.key);
      });
      /* const textDecoder = new TextDecoder();
      socket.addEventListener('message', m => {
        term.io.writeUTF8(textDecoder.decode(m.data));
      }); */
    // }, 2000);
  }
  
  shouldComponentUpdate() {
    return false;
  }

  render() {
    console.log('!!!!!!!!!!!!render');
    Error.stackTraceLimit = 300;
    
    const {root} = this.props;

    return (
      <div className="Console" id="terminal" />
    );
  }
}

export default Console;