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
    
    menuClassNames(item) {
      const classNames = ['menu-item'];
      if (item === this.state.item) {
        classNames.push('open');
      }
      return classNames.join(' ');
    }
    
    urlPopupClassNames(item) {
      const classNames = ['url-popup'];
      if (this.state.urlFocus) {
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
    
    focus() {
      this.setState({urlFocus: true});
    }
    
    blur() {
      this.setState({urlFocus: false});
    }

    render() {
      return (
        <div id="Engine">
          <div className="row menu">
            <div className={this.menuClassNames('file')} onClick={() => this.open('file')}>
              File
            </div>
            <div className={this.menuClassNames('import')}onClick={() => this.open('import')}>
              Import
            </div>
            <div className={this.menuClassNames('export')} onClick={() => this.open('export')}>
              Export
            </div>
            <div className={this.menuClassNames('about')} onClick={() => this.open('about')}>
              About
            </div>
            <div class="url">
              <div className={this.urlPopupClassNames()}>
                <div className="url-item">3D Reality Tab</div>
                <div className="url-item">2D Reality Tab</div>
              </div>
              <input type="text" className="url-input" value="https://aframe.io/a-painter" onFocus={() => this.focus()} onBlur={() => this.blur()}/>
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