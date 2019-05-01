import React from 'react';
import logo from '../media/exokit.svg'

class NavTabs extends React.Component {

    constructor(props) {
      super(props);
      this.toggleTab = this.toggleTab.bind(this);
      this.togglePage = this.togglePage.bind(this);
    }

    componentDidMount(){
      this.togglePage('engine');
    }

    togglePage(page){
      const enginePage = document.getElementById('Engine');
      const launchPage = document.getElementById('Launch');
      const updatePage = document.getElementById('Update');
      const socialPage = document.getElementById('Social')

      enginePage.hidden = true;
      updatePage.hidden = true;
      launchPage.hidden = true;
      socialPage.hidden = true;

      switch(page) {
        case 'engine':
          enginePage.hidden = false;
          break;
        case 'launch':
          launchPage.hidden = false;
          break;
        case 'update':
          updatePage.hidden = false;
          break;
        case 'social':
          socialPage.hidden = false;
          break;
        default:
          break;
      }
    }

    toggleTab(tab){
      const engineTab = document.getElementById('engine');
      const launchTab = document.getElementById('launch');
      const updateTab = document.getElementById('update');
      const socialTab = document.getElementById('social');

      switch(tab){
        case 'engine':
          if(!engineTab.classList.contains('active')){
            engineTab.classList.add('active');
          }
          launchTab.classList.remove('active');
          updateTab.classList.remove('active');
          socialTab.classList.remove('active');
          this.togglePage('engine');
          break;
        case 'launch':
          if(!launchTab.classList.contains('active')){
            launchTab.classList.add('active');
          }
          engineTab.classList.remove('active');
          updateTab.classList.remove('active');
          socialTab.classList.remove('active');
          this.togglePage('launch');
          break;
        case 'update':
          if(!updateTab.classList.contains('active')){
            updateTab.classList.add('active');
          }
          engineTab.classList.remove('active');
          launchTab.classList.remove('active');
          socialTab.classList.remove('active');
          this.togglePage('update');
          break;
        case 'social':
          if(!socialTab.classList.contains('active')){
            socialTab.classList.add('active');
          }
          engineTab.classList.remove('active');
          launchTab.classList.remove('active');
          updateTab.classList.remove('active');
          this.togglePage('social');
          break;
        default:
          break;
      }
    }

    render() {
      return (
        <div id="NavTabs">
          <ul className="nav nav-pills p-2">
            <li className="nav-item">
              <a className="nav-link active" id="engine" onClick={() => this.toggleTab('engine')} href="#">Engine</a>
            </li>
            <li className="nav-item">
              <a className="nav-link" id="launch" onClick={() => this.toggleTab('launch')} href="#">Launch</a>
            </li>
            <li className="nav-item">
              <a className="nav-link" id="update" onClick={() => this.toggleTab('update')} href="#">Update</a>
            </li>
            <li className="nav-item">
              <a className="nav-link" id="social" onClick={() => this.toggleTab('social')} href="#">Social</a>
            </li>
          </ul>
          <img id="logo" src={logo}></img>
        </div>
        );
    }
  }

  export default NavTabs;