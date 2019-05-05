import React from 'react';
import Dom from './Dom';
import Console from './Console';
import '../css/engine.css';

class Engine extends React.Component {

    constructor(props) {
      super(props);
      this.postMessage = this.postMessage.bind(this);
      this.setFlag = this.setFlag.bind(this);
      this.handleURLChange = this.handleURLChange.bind(this);
      this.state = {
        flags: [],
        item: null,
        addTab: 'template',
        url: 'https://aframe.io/a-painter/',
      };
    }
    
    componentDidMount() {
      const engineRender = document.getElementById('engine-render');

      const _postViewportMessage = () => {
        const bcr = engineRender.getBoundingClientRect();
        const viewport = [bcr.x/window.innerWidth, bcr.y/window.innerHeight, bcr.width/window.innerWidth, bcr.height/window.innerHeight];
        window.postMessage({
          method: 'viewport',
          viewport,
        });
      };
      _postViewportMessage();
      window.addEventListener('resize', _postViewportMessage);
      
      window.addEventListener('keydown', e => {
        console.log('iframe keydown ' + e.keyCode);
      });
      window.addEventListener('keyup', e => {
        console.log('iframe keyup ' + e.keyCode);
      });
      window.addEventListener('keypress', e => {
        console.log('iframe keypress ' + e.keyCode);
      });
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
      });
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
    
    addTabClassNames(addTab) {
      const classNames = ['menu-item-popup-tab'];
      if (addTab === this.state.addTab) {
        classNames.push('selected');
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
    
    openMenu(item) {
      const open = this.state.item !== item;
      this.setState({item: open ? item : null}, () => {
        this.postMenuStatus();
      });
    }
    
    openAddTab(e, addTab) {
      this.setState({
        addTab,
      });
      
      e.stopPropagation();
    }
    
    focusUrlInput() {
      this.setState({
        item: null,
        urlFocus: true,
      }, () => {
        this.postMenuStatus();
      });
    }
    
    blurUrlInput() {
      this.setState({urlFocus: false}, () => {
        this.postMenuStatus();
      });
    }
    
    onUrlChange(e) {
      console.log('url change', e.target.value);
      this.setState({
        url: e.target.value,
      });
    }

    open3dTab() {
      const urlInput = document.getElementById('url-input');
      const url = urlInput.value;

      window.postMessage({
        method: 'open',
        url,
        d: 3,
      });

      this.blur();
    }

    open2dTab() {
      const urlInput = document.getElementById('url-input');
      const url = urlInput.value;

      window.postMessage({
        method: 'open',
        url,
        d: 2,
      });

      this.blur();
    }
    
    loadWorld() {
      const urlInput = document.getElementById('url-input');
      const url = urlInput.value;

      window.postMessage({
        method: 'open',
        url,
        d: null,
      });

      this.blur();
    }

    onFakeXrClick() {
      window.postMessage({
        method: 'click',
        target: 'fakeXr',
      });
      
      this.blur();
    }
    
    onXrClick() {
      window.postMessage({
        method: 'click',
        target: 'xr',
      });
    }

    /* onSettingsClick() {
      window.postMessage({
        method: 'click',
        target: 'settings',
      });
    } */

    blur() {
      this.setState({
        item: null,
        urlFocus: false,
      }, () => {
        this.postMenuStatus();
      });
    }
    
    postMenuStatus() {
      window.postMessage({
        method: 'menu',
        open: this.state.urlFocus || this.state.item !== null,
      });
    }

    render() {
      return (
        <div id="Engine">
          <div className="row menu">
            <div className={this.menuItemClassNames('world')}onClick={() => this.openMenu('world')}>
              <div className={this.menuItemPopupClassNames('world')}>
                <div className="menu-item-popup-item">New</div>
                <div className="menu-item-popup-item">Exit</div>
              </div>
              <i class="fal fa-cube"/>
              {/* <div>World</div> */}
            </div>
            <div className={this.menuItemClassNames('settings')}onClick={() => this.openMenu('settings')}>
              <div className={this.menuItemPopupClassNames('settings')}>
                <div className="menu-item-popup-item">Preferences...</div>
                <div className="menu-item-popup-item">SDK Paths...</div>
              </div>
              <i class="fal fa-cogs"/>
              {/* <div>Settings</div> */}
            </div>
            <div className="url">
              <div className={this.urlPopupClassNames()}>
                <div className="url-item" onMouseDown={e => e.preventDefault()} onClick={() => this.open3dTab()}>3D Reality Tab</div>
                <div className="url-item" onMouseDown={e => e.preventDefault()} onClick={() => this.open2dTab()}>2D Reality Tab</div>
                <div className="url-item" onMouseDown={e => e.preventDefault()} onClick={() => this.loadWorld()}>Reload</div>
              </div>
              <input type="text" className="url-input" id="url-input" value={this.state.url} onChange={e => this.onUrlChange(e)} onFocus={() => this.focusUrlInput()} onBlur={() => this.blurUrlInput()}/>
            </div>
            <div className={this.menuItemClassNames('add')}onClick={() => this.openMenu('add')}>
              <div className={this.menuItemPopupClassNames('add')}>
                <div className="menu-item-popup-tabs">
                  <div className={this.addTabClassNames('template')} onClick={e => this.openAddTab(e, 'template')}>Template</div>
                  <div className={this.addTabClassNames('examples')} onClick={e => this.openAddTab(e, 'examples')}>Examples</div>
                </div>
                <div className="menu-item-popup-item">
                  <i className="fal fa-file"></i>
                  <div className="label">Blank layer</div>
                </div>
                <div className="menu-item-popup-item">
                  <i className="fab fa-autoprefixer"/>
                  <div className="label">A-Frame layer</div>
                </div>
                <div className="menu-item-popup-item">
                  <i className="fab fa-btc"/>
                  <div className="label">Babylon.js layer</div>
                </div>
              </div>
              <i class="fal fa-plus-hexagon"/>
            </div>
            <div className="buttons">
              <div className="button" onClick={() => this.onXrClick()}>
                <i class="fas fa-head-vr"/>
                <div className="label">Enter XR</div>
              </div>
              <div className="button" onClick={() => this.onFakeXrClick()}>
                <i class="fal fa-vr-cardboard"/>
                <div className="label">Fake XR</div>
              </div>
            </div>
          </div>
          <div className="engine-split">
            <div className="engine-left">
              <div className="engine-render" id="engine-render"/>
              <Console/>
            </div>
            <div className="engine-right">
              <Dom/>
            </div>
          </div>
        </div>
      );
    }
  }

  export default Engine;