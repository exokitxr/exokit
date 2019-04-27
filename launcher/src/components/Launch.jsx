import React from 'react';
import '../css/launch.css';

class Launch extends React.Component {

    constructor(props) {
      super(props);
      this.postMessage = this.postMessage.bind(this);
      this.setFlag = this.setFlag.bind(this);
      this.state = {
          flags: []
      };

    }

    postMessage(){
        window.postMessage({
            message: 'launch',
            flags: this.state.flags
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
        console.log('Flags', this.state.flags);
    }

    render() {
      return (
        <div id="Launch">
            <div className="row mt-4">
                <div className="col-lg-1 col-md-1 col-sm-0 col-xs-0">
                </div>
                <div className="col-lg-5 col-md-5 col-sm-12 col-xs-12 m-4">
                    <div className="mb-4">
                        <h1>Reality Tab</h1>
                        <div className="form-group">
                            <input type="text" className="form-control" id="urlInput" placeholder="Enter URL"/>
                        </div>
                        <button onClick={() => this.postMessage} type="button" className="btn btn-primary">Launch</button>
                    </div>

                    <h3>Flags</h3>
                    <div className="form-check">
                        <input onChange={(e) => this.setFlag(e)} className="form-check-input" type="checkbox" value="u" id="defaultCheck1"/>
                        <label className="form-check-label">
                            <code>--uncapped</code> - Uncapped FPS
                        </label>
                    </div>
                    <div className="form-check">
                        <input onChange={(e) => this.setFlag(e)} className="form-check-input" type="checkbox" value="p" id="defaultCheck1"/>
                        <label className="form-check-label">
                            <code>--performance</code> - Log Performance
                        </label>
                    </div>
                    <div className="form-check">
                        <input onChange={(e) => this.setFlag(e)} className="form-check-input" type="checkbox" value="w" id="defaultCheck1"/>
                        <label className="form-check-label">
                            <code>--webgl</code> - WebGL
                        </label>
                    </div>
                    <div className="form-check">
                        <input onChange={(e) => this.setFlag(e)} className="form-check-input" type="checkbox" value="x" id="defaultCheck1"/>
                        <label className="form-check-label">
                            <code>--xr</code> - WebXR
                        </label>
                    </div>
                    <div className="form-check">
                        <input onChange={(e) => this.setFlag(e)} className="form-check-input" type="checkbox" value="b" id="defaultCheck1"/>
                        <label className="form-check-label">
                            <code>--blit</code> - Blit
                        </label>
                    </div>
                    <div className="form-check">
                        <input onChange={(e) => this.setFlag(e)} className="form-check-input" type="checkbox" value="t" id="defaultCheck1"/>
                        <label className="form-check-label">
                            <code>--tab</code> - Reality Tab
                        </label>
                    </div>
                    <div className="form-check">
                        <input onChange={(e) => this.setFlag(e)} className="form-check-input" type="checkbox" value="s" id="defaultCheck1"/>
                        <label className="form-check-label">
                            <code>--size</code> - Size
                        </label>
                    </div>
                    <div className="form-check">
                        <input onChange={(e) => this.setFlag(e)} className="form-check-input" type="checkbox" value="f" id="defaultCheck1"/>
                        <label className="form-check-label">
                            <code>--frame</code> - Frame
                        </label>
                    </div>
                    <div className="form-check">
                        <input onChange={(e) => this.setFlag(e)} className="form-check-input" type="checkbox" value="m" id="defaultCheck1"/>
                        <label className="form-check-label">
                            <code>--minimalFrame</code> - Minimal Frame
                        </label>
                    </div>
                    <div className="form-check">
                        <input onChange={(e) => this.setFlag(e)} className="form-check-input" type="checkbox" value="q" id="defaultCheck1"/>
                        <label className="form-check-label">
                            <code>--quit</code> - Quit
                        </label>
                    </div>
                    <div className="form-check">
                        <input onChange={(e) => this.setFlag(e)} className="form-check-input" type="checkbox" value="r" id="defaultCheck1"/>
                        <label className="form-check-label">
                            <code>--require</code> - Require
                        </label>
                    </div>
                    <div className="form-check">
                        <input onChange={(e) => this.setFlag(e)} className="form-check-input" type="checkbox" value="i" id="defaultCheck1"/>
                        <label className="form-check-label">
                            <code>--image</code> - Image
                        </label>
                    </div>
                    
                </div>
                <div className="col-lg-5 col-md-5 col-sm-12 col-xs-12 m-4">
                    <div id="howto">
                        <h1>How To</h1>
                        <p>Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged.</p>
                        <p>It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.</p>
                    </div>
                </div>
                <div className="col-lg-1 col-md-1 col-sm-0 col-xs-0">
                </div>
            </div>
        </div>
        );
    }
  }

  export default Launch;