import React from 'react';
import '../css/dom.css';

class Dom extends React.Component {
  /* constructor(props) {
    super(props);
  } */

  render() {
    const {root} = this.props;

    return (
      <ul className="Dom">
        <DomItem el={this.props.root}/>
      </ul>
    );
  }
}

class DomItem extends React.Component {
  constructor(props) {
    super(props);
    
    this.state = {
      open: true,
    };
  }

  getClassnames() {
    const classNames = ['dom-item'];
    if (this.state.open) {
      classNames.push('open');
    }
    return classNames.join(' ');
  }
  
  toggleOpen() {
    this.setState({
      open: !this.state.open,
    });
  }
  
  render() {
    const {el} = this.props;
    console.log('got el', this.props, this.getClassnames());

    if (el.nodeType === Node.ELEMENT_NODE) {
      return (
        <li className={this.getClassnames()}>
          <div className="dom-item-label">
            <div className="dom-item-arrow" onClick={() => this.toggleOpen()}>⮞</div>
            <div className="dom-item-name">&lt;{el.tagName.toLowerCase()}&gt;</div>
          </div>
          <div className="dom-item-children">
            {el.childNodes.map((childNode, i) => <DomItem el={childNode} key={i}/>)}
          </div>
        </li>
      );
    } else if (el.nodeType === Node.TEXT_NODE) {
      return (
        <li className={this.getClassnames()}>
          <div className="dom-item-text">"{el.nodeValue}"</div>
        </li>
      );
    } else {
      return null;
    }
  }
}

export default Dom;