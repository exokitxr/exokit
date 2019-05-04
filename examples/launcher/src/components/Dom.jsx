import React from 'react';
import '../css/dom.css';

const _decorateKeypaths = (el, keypath = []) => {
  el.keypath = keypath;
  for (let i = 0; i < el.childNodes.length; i++) {
    _decorateKeypaths(el.childNodes[i], keypath.concat([i]));
  }
};
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

    const root = {
      nodeType: 1,
      tagName: 'HTML',
      value: '',
      attrs: [{
        name: 'href',
        value: 'https://',
      }],
      childNodes: [
        {
          nodeType: 1,
          tagName: 'BODY',
          value: '',
          attrs: [{
            name: 'lol',
            value: 'zol',
            name: 'woot',
            value: 'toot',
          }],
          childNodes: [],
        },
      ],
    };
    _decorateKeypaths(root);
    this.state = {
      root,
      selectEl: null,
      epoch: 0,
    };
  }

  componentDidMount() {
    window.addEventListener('message', e => {
      const m = e.data;
      if (m.method === 'dom') {
        _decorateKeypaths(m.root);
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
        epoch: this.state.epoch + 1,
      });
    } else {
      this.setState({
        selectEl: null,
        epoch: this.state.epoch + 1,
      });
    }
  }

  render() {
    return (
      <div className="Dom">
        <DomList root={this.state.root} selectEl={this.state.selectEl} onClick={el => this.onClick(el)} />
        {this.state.selectEl ? <DomDetail el={this.state.selectEl} epoch={this.state.epoch} /> : null}
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
        <DomItem el={this.props.root} level={0} selectEl={this.props.selectEl} hoverEl={this.state.hoverEl} onClick={el => this.props.onClick(el)} onMouseEnter={el => this.onMouseEnter(el)} onMouseLeave={el => this.onMouseLeave(el)}/>
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
    if (this.props.selectEl === this.props.el) {
      classNames.push('select');
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
          <div className="dom-item-label" style={this.getStyle()} onClick={() => this.props.onClick(el)} onMouseEnter={() => this.props.onMouseEnter(el)} onMouseLeave={() => this.props.onMouseLeave(el)}>
            <div className="dom-item-arrow" onClick={e => this.toggleOpen(e)}>⮞</div>
            <div className="dom-item-name">{_el2Text(el)}</div>
          </div>
          <div className="dom-item-children">
            {el.childNodes.map((childNode, i) => <DomItem el={childNode} level={level+1} selectEl={this.props.selectEl} hoverEl={this.props.hoverEl} onClick={el => this.props.onClick(el)} onMouseEnter={this.props.onMouseEnter} onMouseLeave={this.props.onMouseLeave} key={i}/>)}
          </div>
        </li>
      );
    } else if (el.nodeType === Node.TEXT_NODE) {
      if (/\S/.test(el.value)) { // has non-whitespace
        return (
          <li className={this.getClassnames()} onClick={() => this.props.onClick(el)} onMouseEnter={() => this.props.onMouseEnter(el)} onMouseLeave={() => this.props.onMouseLeave(el)}>
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
        {el.attrs.map(attr => <DomAttribute name={attr.name} value={attr.value} keypath={el.keypath} epoch={this.props.epoch} key={attr.name} />)}
      </div>
    );
  }
}

class DomAttribute extends React.Component {
  constructor(props) {
    super(props);

    this.state = {
      name: props.name,
      value: props.value,
    };
  }
  
  UNSAFE_componentWillUpdate(prevProps, prevState) {
    if (this.props.epoch !== prevProps.epoch) {
      const {name, value} = this.props;
      this.setState({
        name,
        value,
      });
    }
  }

  onNameChange(e) {
    const name = e.target.value;
    this.setState({
      name,
    });
  }
  
  onNameBlur() {
    window.postMessage({
      method: 'edit',
      keypath: this.props.keypath,
      edit: {
        type: 'name',
        oldName: this.props.name,
        oldValue: this.props.value,
        newName: this.state.name,
      },
    });
  }

  onValueChange(e) {
    const value = e.target.value;
    this.setState({
      value,
    });
  }
  
  onValueBlur() {
    window.postMessage({
      method: 'edit',
      keypath: this.props.keypath,
      edit: {
        type: 'value',
        name: this.props.name,
        newValue: this.state.value,
      },
    });
  }

  render() {
    return (
      <div className="dom-attribute">
        <input type="text" className="dom-attribute-name" value={this.state.name} onChange={e => this.onNameChange(e)} onBlur={e => this.onNameBlur()} />
        <input type="text" className="dom-attribute-value" value={this.state.value} onChange={e => this.onValueChange(e)} onBlur={e => this.onValueBlur()} />
      </div>
    );
  }
}

export default Dom;