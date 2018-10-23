import React, { Component } from 'react';
import logo from './logo.svg';
import './App.css';

class App extends Component {
  render() {
    return (
      <div className="App">
        <div className="row p-3">
          <div className="col-12 text-center mx-auto">
            <h1 className="mx-auto">Welcome To Exokit!</h1>
          </div>
          <div className="col-12 text-center mx-auto">
            <img src="https://docs.webmr.io/img/ExokitLogo.png" height="100" width="100" className="mx-auto"/>
          </div>
        </div>

        <div className="row p-3">
          <div className="col-6">
            <h3>Settings/Flags</h3>
            <form>
              <div className="form-group">
                <div className="custom-control custom-checkbox">
                  <input type="checkbox" className="custom-control-input" id="customCheck1"/>
                    <label className="custom-control-label" htmlFor="customCheck1"><code>-u</code> | Uncap FPS</label>
                </div>
                <div className="custom-control custom-checkbox">
                  <input type="checkbox" className="custom-control-input" id="customCheck2"/>
                    <label className="custom-control-label" htmlFor="customCheck2"><code>-p</code> | Performance Log to
                      Console</label>
                </div>
                <div className="custom-control custom-checkbox">
                  <input type="checkbox" className="custom-control-input" id="customCheck3"/>
                    <label className="custom-control-label" htmlFor="customCheck3"><code>-x</code> | WebXR</label>
                </div>
              </div>
              <button type="submit" className="btn btn-primary">Save</button>
              <button type="submit" className="btn btn-primary">Defaults</button>
            </form>
          </div>

          <div className="col-6">
            <h3>News</h3>
            <div id="carouselExampleIndicators" className="carousel slide" data-ride="carousel">
              <ol className="carousel-indicators">
                <li data-target="#carouselExampleIndicators" data-slide-to="0" className="active"/>
                <li data-target="#carouselExampleIndicators" data-slide-to="1"/>
                <li data-target="#carouselExampleIndicators" data-slide-to="2"/>
              </ol>
              <div className="carousel-inner">
                <div className="carousel-item active">
                  <img className="d-block text-center mx-auto"
                       src="https://i.ytimg.com/vi/1NpBvrDMYIo/maxresdefault.jpg" alt="First slide"/>
                </div>
                <div className="carousel-item">
                  <img className="d-block text-center mx-auto"
                       src="https://i.ytimg.com/vi/prOh9OOY-M4/maxresdefault.jpg" alt="Second slide"/>
                </div>
                <div className="carousel-item">
                  <img className="d-block text-center mx-auto"
                       src="https://cdn-images-1.medium.com/max/2000/1*XdsMp25hIwODAA2O2Ef1TQ.png" alt="Third slide"/>
                </div>
              </div>
              <a className="carousel-control-prev" href="#carouselExampleIndicators" role="button" data-slide="prev">
                <span className="carousel-control-prev-icon" aria-hidden="true"/>
                <span className="sr-only">Previous</span>
              </a>
              <a className="carousel-control-next" href="#carouselExampleIndicators" role="button" data-slide="next">
                <span className="carousel-control-next-icon" aria-hidden="true"/>
                <span className="sr-only">Next</span>
              </a>
            </div>
          </div>
        </div>

        <div className="row pt-5 p-3">
          <div className="col-8">
            <h5>Updates/Progress</h5>
            <div className="progress">
              <div className="progress-bar progress-bar-striped bg-info" role="progressbar" style={{width: 50 + '%'}}
                   aria-valuenow="50" aria-valuemin="0" aria-valuemax="100"/>
            </div>
            <small>75%</small>
          </div>
          <div className="col-4">
            <button type="submit" className="btn btn-primary">Launch Terminal</button>
            <button type="submit" className="btn btn-primary">Launch Exo-Home</button>
          </div>
        </div>
      </div>
    );
  }
}

export default App;
