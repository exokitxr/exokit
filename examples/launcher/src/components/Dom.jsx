import React from 'react';
import '../css/dom.css';

class Dom extends React.Component {
  constructor(props) {
    super(props);
    
    this.state = {
      hoverEl: null,
    };
  }
  
  onMouseEnter(el) {
    this.setState({
      hoverEl: el,
    });
  }
  
  onMouseLeave(el) {
    if (this.state.hoverEl == el) {
      this.setState({
        hoverEl: null,
      });
    }
  }

  render() {
    const {root} = this.props;

    return (
      <ul className="Dom">
        <DomItem el={this.props.root} level={0} hoverEl={this.state.hoverEl} onMouseEnter={el => this.onMouseEnter(el)} onMouseLeave={el => this.onMouseLeave(el)}/>
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
    if (this.props.hoverEl === this.props.el) {
      classNames.push('hover');
    }
    return classNames.join(' ');
  }
  
  getStyle() {
    return {
      paddingLeft: `${this.props.level * 10}px`,
    };
  }
  
  toggleOpen() {
    this.setState({
      open: !this.state.open,
    });
  }
  
  render() {
    const {el, level} = this.props;

    if (el.nodeType === Node.ELEMENT_NODE) {
      return (
        <li className={this.getClassnames()}>
          <div className="dom-item-label" style={this.getStyle()} onMouseEnter={() => this.props.onMouseEnter(el)} onMouseLeave={() => this.props.onMouseLeave(el)}>
            <div className="dom-item-arrow" onClick={() => this.toggleOpen()}>⮞</div>
            <div className="dom-item-name">&lt;{el.tagName.toLowerCase()}&gt;</div>
          </div>
          <div className="dom-item-children">
            {el.childNodes.map((childNode, i) => <DomItem el={childNode} level={level+1} hoverEl={this.props.hoverEl} onMouseEnter={this.props.onMouseEnter} onMouseLeave={this.props.onMouseLeave} key={i}/>)}
          </div>
        </li>
      );
    } else if (el.nodeType === Node.TEXT_NODE) {
      return (
        <li className={this.getClassnames()} onMouseEnter={() => this.props.onMouseEnter(el)} onMouseLeave={() => this.props.onMouseLeave(el)}>
          <div className="dom-item-text" style={this.getStyle()}>"{el.nodeValue}"</div>
        </li>
      );
    } else {
      return null;
    }
  }
}

export default Dom;