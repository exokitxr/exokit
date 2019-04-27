import React from 'react';
import '../css/update.css';
const ReactMarkdown = require('react-markdown')

class Update extends React.Component {

    constructor(props) {
      super(props);
      this.state = {
          readme: ''
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

    postMessage(){
        window.postMessage({
            click: 'hello world'
        })
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
                    <p>75%</p>
                    <div className="progress">
                        <div className="progress-bar progress-bar-striped progress-bar-animated w-75" role="progressbar" aria-valuenow="50" aria-valuemin="0" aria-valuemax="100"></div>
                    </div>
                    <br/>
                    <button onClick={() => this.postMessage()} type="button" className="btn btn-primary btn-lg m-2">Update</button>
                </div>
                <div className="col-lg-1 col-md-1 col-sm-1 col-xs-1">
                </div>
            </div>
        </div>
        );
    }
  }

  export default Update;