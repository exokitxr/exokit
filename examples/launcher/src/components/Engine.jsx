import React from 'react';
import '../css/engine.css';

class Engine extends React.Component {

    constructor(props) {
      super(props);
      this.postMessage = this.postMessage.bind(this);
      this.setFlag = this.setFlag.bind(this);
      this.handleURLChange = this.handleURLChange.bind(this);
      this.state = {
          flags: [],
          url: ''
      };
    }

    postMessage(action){
        window.postMessage({
            action: action,
            flags: this.state.flags,
            url: this.state.url
        })
    }

    handleURLChange(e){
        this.setState({
            url: e.target.value
        })
    }

    setFlag(e){
        let flag = e.target.value;
        if(!this.state.flags.includes(flag)){
            this.state.flags.push(flag)
        }
        else{
            this.state.flags.splice(this.state.flags.indexOf(flag), 1);
        }
    }
    
    classNames(item) {
      const classNames = ['menu-item'];
      if (item === this.state.item) {
        classNames.push('open');
      }
      return classNames.join(' ');
    }
    
    open(item) {
      if (this.state.item !== item) {
        this.setState({item});
      } else {
        this.setState({item: null});
      }
    }

    render() {
      return (
        <div id="Engine">
            <div className="row menu">
                <div className={this.classNames('file')} onClick={() => this.open('file')}>
                  File
                </div>
                <div className={this.classNames('import')}onClick={() => this.open('import')}>
                  Import
                </div>
                <div className={this.classNames('export')} onClick={() => this.open('export')}>
                  Export
                </div>
                <div className={this.classNames('about')} onClick={() => this.open('about')}>
                  About
                </div>
            </div>
            <div className="engine-split">
              <div className="engine-left">
                <div className="engine-render">
                </div>
                <div className="engine-shell">[x] exokit</div>
              </div>
              <div className="engine-right">
                <ul>
                  <li>&lt;html&gt;</li>
                  <li>&lt;body&gt;</li>
                </ul>
              </div>
            </div>
        </div>
        );
    }
  }

  export default Engine;