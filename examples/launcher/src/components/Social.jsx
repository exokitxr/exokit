import React from 'react';
import '../css/social.css';

class Social extends React.Component {

    /* constructor(props) {
      super(props);
    } */

    render() {
      return (
        <div id="Social">
            <div className="row pb-5 mt-5">
              <div className="col-4 text-center">
                <a href="https://twitter.com/webmixedreality?lang=en">
                  <i className="fab fa-twitter fa-5x"></i>
                  <h4 className="mt-3">Twitter</h4>
                </a>
              </div>
              <div className="col-4 text-center">
                <a href="https://www.twitch.tv/avaer">
                  <i className="fab fa-twitch fa-5x"></i>
                  <h4 className="mt-3">Twitch</h4>
                </a>
              </div>
              <div className="col-4 text-center">
                <a href="https://github.com/exokitxr/exokit">
                  <i className="fab fa-github fa-5x"></i>
                  <h4 className="mt-3">Github</h4>
                </a>
              </div>
            </div>
            <div className="row pb-5 mt-5">
              <div className="col-4 text-center">
                <a href="https://www.youtube.com/channel/UC87Q7_5ooY8FSLwOec52ZPQ">
                <i className="fab fa-youtube fa-5x"></i>                  
                <h4 className="mt-3">Youtube</h4>
                </a>
              </div>
              <div className="col-4 text-center">
                <a href="https://discordapp.com/invite/Apk6cZN">
                <i className="fab fa-discord fa-5x"></i>
                <h4 className="mt-3">Discord</h4>
                </a>
              </div>
              <div className="col-4 text-center">
                <a href="https://communityinviter.com/apps/exokit/exokit">
                <i className="fab fa-slack fa-5x"></i>
                  <h4 className="mt-3">Slack</h4>
                </a>
              </div>
            </div>
        </div>
        );
    }
  }

  export default Social;