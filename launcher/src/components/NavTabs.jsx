import React from 'react';
import logo from '../media/exokit.svg'

class NavTabs extends React.Component {

    constructor(props) {
      super(props);
      this.toggleTab = this.toggleTab.bind(this);
    }

    toggleTab(tab){
      const launchTab = document.getElementById('launch');
      const updateTab = document.getElementById('update');
      const socialTab = document.getElementById('social');

      switch(tab){
        case 'launch':
          if(!launchTab.classList.contains('active')){
            launchTab.classList.add('active');
          }
          updateTab.classList.remove('active');
          socialTab.classList.remove('active');
          break;
        case 'update':
          if(!updateTab.classList.contains('active')){
            updateTab.classList.add('active');
          }
          launchTab.classList.remove('active');
          socialTab.classList.remove('active');
          break;
        case 'social':
          if(!socialTab.classList.contains('active')){
            socialTab.classList.add('active');
          }
          updateTab.classList.remove('active');
          launchTab.classList.remove('active');
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
              <a className="nav-link active" id="launch" onClick={() => this.toggleTab('launch')} href="#">Launch</a>
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