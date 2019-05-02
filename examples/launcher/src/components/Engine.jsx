import React from 'react';
import Dom from './Dom';
import Console from './Console';
import '../css/engine.css';

const _objectHtml = el => {
  return {
    nodeType: el.nodeType || 0,
    tagName: el.tagName || '',
    nodeValue: el.nodeType === Node.TEXT_NODE ? el.nodeValue : '',
    childNodes: Array.from(el.childNodes).map(_objectHtml),
  };
};

class Engine extends React.Component {

    constructor(props) {
      super(props);
      this.postMessage = this.postMessage.bind(this);
      this.setFlag = this.setFlag.bind(this);
      this.handleURLChange = this.handleURLChange.bind(this);
      this.state = {
          flags: [],
          url: '',
          root: (() => {
            const html = document.createElement('html');
            html.innerHTML = `
<ul>
  <li>lol</li>
  <li>zol</li>
</ul>
`.replace(/\n/g, '');
            return _objectHtml(html);
          })(),
      };
    }
    
    componentDidMount() {
      const app = document.getElementById('app');
      const engineRender = document.getElementById('engine-render');

      const _postViewportMessage = () => {
        const bcr = engineRender.getBoundingClientRect();
        const bcr2 = app.getBoundingClientRect();
        const viewport = [bcr.x/bcr2.width, bcr.y/bcr2.height, bcr.width/bcr2.width, bcr.height/bcr2.height];
        console.log('engine render viewport', viewport);
        window.postMessage({
          method: 'viewport',
          viewport,
        });
      };
      _postViewportMessage();
      window.addEventListener('resize', _postViewportMessage);
      
      /* setTimeout(() => {
        _postViewportMessage();
      }, 1000); */
    }

    postMessage(action){
      window.postMessage({
        action: action,
        flags: this.state.flags,
        url: this.state.url
      });
    }

    handleURLChange(e){
        this.setState({
            url: e.target.value
        })
    }

    setFlag(e){
      let flag = e.target.value;
      if (!this.state.flags.includes(flag)) {
        this.state.flags.push(flag)
      } else {
        this.state.flags.splice(this.state.flags.indexOf(flag), 1);
      }
    }
    
    menuItemClassNames(item) {
      const classNames = ['menu-item'];
      if (item === this.state.item) {
        classNames.push('open');
      }
      return classNames.join(' ');
    }
    
    menuItemPopupClassNames(item) {
      const classNames = ['menu-item-popup'];
      if (item === this.state.item) {
        classNames.push('open');
      }
      return classNames.join(' ');
    }
    
    urlPopupClassNames() {
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
            <div className={this.menuItemClassNames('file')} onClick={() => this.open('file')}>
              <div className={this.menuItemPopupClassNames('file')}>
                <div className="menu-item-popup-item">
                  New A-Frame...
                </div>
              </div>
              <div>File</div>
            </div>
            <div className={this.menuItemClassNames('import')}onClick={() => this.open('import')}>
              <div className={this.menuItemPopupClassNames('import')}>
                <div className="menu-item-popup-item">
                  New A-Frame...
                </div>
              </div>
              <div>Import</div>
            </div>
            <div className={this.menuItemClassNames('export')} onClick={() => this.open('export')}>
              <div className={this.menuItemPopupClassNames('export')}>
                <div className="menu-item-popup-item">
                  New A-Frame...
                </div>
              </div>
              <div>Export</div>
            </div>
            <div className={this.menuItemClassNames('about')} onClick={() => this.open('about')}>
              <div className={this.menuItemPopupClassNames('about')}>
                <div className="menu-item-popup-item">
                  New A-Frame...
                </div>
              </div>
              <div>About</div>
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
              <div className="engine-render" id="engine-render">
              </div>
              <Console/>
            </div>
            <div className="engine-right">
              <Dom root={this.state.root}/>
            </div>
          </div>
        </div>
      );
    }
  }

  export default Engine;