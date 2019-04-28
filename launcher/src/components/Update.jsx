import React from 'react';
import '../css/update.css';
const ReactMarkdown = require('react-markdown')

class Update extends React.Component {

    constructor(props) {
      super(props);
      this.state = {
          readme: '',
          progress: ''
        };
        this.postMessage = this.postMessage.bind(this);
    }

    componentDidMount(){
        fetch('https://raw.githubusercontent.com/exokitxr/exokit/add-changelog/CHANGELOG.md')
            .then(function(response) {
                return response.text();
            }).then((data) => {
                this.setState({
                    readme: data
                });
            });
    }

    postMessage(action){
        window.postMessage({
            action: action
        })
        window.onmessage = m => {
            console.log('progress from backend: ', m.data.progress);
            this.setState({
                progress: m.data.progress
            })
            document.getElementById('progressBar').style.width = this.state.progress + '%';
        };
    }

    render() {
      return (
        <div id="Update">
            <div className="row">
                <div className="col-lg-1 col-md-1 col-sm-1 col-xs-1">
                </div>
                <div className="col-lg-10 col-md-10 col-sm-10 col-xs-10 p-5">
                    <h1>Changelog</h1>
                    <div id="changelog" className="p-4">
                        <ReactMarkdown source={this.state.readme}/>
                    </div>
                </div>
                <div className="col-lg-1 col-md-1 col-sm-1 col-xs-1">
                </div>
                <div className="col-lg-1 col-md-1 col-sm-1 col-xs-1">
                </div>
                <div className="col-lg-10 col-md-10 col-sm-10 col-xs-10 p-5 text-center">
                    <p>{this.state.progress}%</p>
                    <div className="progress">
                        <div className="progress-bar progress-bar-striped progress-bar-animated" role="progressbar" aria-valuenow={this.state.progress} aria-valuemin="0" aria-valuemax="100" id="progressBar"></div>
                    </div>
                    <br/>
                    <button onClick={() => this.postMessage('update')} type="button" className="btn btn-primary btn-lg m-2">Update</button>
                </div>
                <div className="col-lg-1 col-md-1 col-sm-1 col-xs-1">
                </div>
            </div>
        </div>
        );
    }
  }

  export default Update;