import React from 'react';
import '../css/dom.css';

const _el2Text = el => {
  let result = '<' + el.tagName.toLowerCase();
  for (let i = 0; i < el.attrs.length; i++) {
    const attr = el.attrs[i];
    result += ' ' + attr.name + '=' + JSON.stringify(attr.value);
  }
  result += '>';
  return result;
};

class Dom extends React.Component {
  constructor(props) {
    super(props);
    
    this.state = {
      root: {
        nodeType: 1,
        tagName: 'HTML',
        value: '',
        attrs: [{
          name: 'href',
          value: 'https://',
        }],
        childNodes: [],
      },
      selectEl: null,
    };
  }

  componentDidMount() {
    window.addEventListener('message', e => {
      const m = e.data;
      if (m.method === 'dom') {
        this.setState({
          root: m.root,
        });
      }
    });
  }

  onClick(el) {
    if (this.state.selectEl !== el) {
      this.setState({
        selectEl: el,
      });
    } else {
      this.setState({
        selectEl: null,
      });
    }
  }

  render() {
    return (
      <div className="Dom">
        <DomList root={this.state.root} selectEl={this.state.selectEl} onClick={el => this.onClick(el)} />
        {this.state.selectEl ? <DomDetail el={this.state.selectEl} /> : null}
      </div>
    );
  }
}

class DomList extends React.Component {
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
    if (this.state.hoverEl === el) {
      this.setState({
        hoverEl: null,
      });
    }
  }

  render() {
    return (
      <ul className="dom-list">
        <DomItem el={this.props.root} level={0} selectEl={this.props.selectEl} hoverEl={this.state.hoverEl} onClick={() => this.props.onClick(this.props.root)} onMouseEnter={el => this.onMouseEnter(el)} onMouseLeave={el => this.onMouseLeave(el)}/>
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
    if (this.props.selectEl === this.props.el || this.props.hoverEl === this.props.el) {
      classNames.push('highlight');
    }
    return classNames.join(' ');
  }
  
  getStyle() {
    return {
      paddingLeft: `${this.props.level * 10}px`,
    };
  }
  
  toggleOpen(e) {
    this.setState({
      open: !this.state.open,
    });

    e.stopPropagation();
  }
  
  render() {
    const {el, level} = this.props;

    if (el.nodeType === Node.ELEMENT_NODE) {
      return (
        <li className={this.getClassnames()}>
          <div className="dom-item-label" style={this.getStyle()} onClick={el => this.props.onClick(el)} onMouseEnter={() => this.props.onMouseEnter(el)} onMouseLeave={() => this.props.onMouseLeave(el)}>
            <div className="dom-item-arrow" onClick={e => this.toggleOpen(e)}>⮞</div>
            <div className="dom-item-name">{_el2Text(el)}</div>
          </div>
          <div className="dom-item-children">
            {el.childNodes.map((childNode, i) => <DomItem el={childNode} level={level+1} hoverEl={this.props.hoverEl} onMouseEnter={this.props.onMouseEnter} onMouseLeave={this.props.onMouseLeave} key={i}/>)}
          </div>
        </li>
      );
    } else if (el.nodeType === Node.TEXT_NODE) {
      if (el.value) {
        return (
          <li className={this.getClassnames()} onMouseEnter={() => this.props.onMouseEnter(el)} onMouseLeave={() => this.props.onMouseLeave(el)}>
            <div className="dom-item-text" style={this.getStyle()}>"{el.value}"</div>
          </li>
        );
      } else {
        return null;
      }
    } else {
      return null;
    }
  }
}

class DomDetail extends React.Component {
  /* constructor(props) {
    super(props);
  } */

  render() {
    const {el} = this.props;

    return (
      <div className="dom-detail">
        <div className="dom-detail-name">{el.tagName.toLowerCase()}</div>
        {el.attrs.map(attr => <DomAttribute attr={attr} key={attr.name} />)}
      </div>
    );
  }
}

class DomAttribute extends React.Component {
  constructor(props) {
    super(props);

    this.state = {
      name: props.attr.name,
      value: props.attr.value,
    };
  }

  onNameChange(e) {
    const name = e.target.value;
    this.setState({
      name,
    });
  }

  onValueChange(e) {
    const value = e.target.value;
    this.setState({
      value,
    });
  }

  render() {
    return (
      <div className="dom-attribute">
        <input type="text" className="dom-attribute-name" value={this.state.name} onChange={e => this.onNameChange(e)} />
        <input type="text" className="dom-attribute-value" value={this.state.value} onChange={e => this.onValueChange(e)} />
      </div>
    );
  }
}

export default Dom;