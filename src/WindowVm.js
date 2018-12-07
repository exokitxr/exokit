const path = require('path');
const {performance} = require('perf_hooks');

const vmOne = require('vm-one');

const GlobalContext = require('./GlobalContext');
const symbols = require('./symbols');

const windowVms = [];
const _makeWindowVm = (htmlString = '', options = {}) => {
  const v = vmOne.make({
    initModule: path.join(__dirname, 'Window.js'),
    args: {
      htmlString,
      options,
      args: GlobalContext.args,
    },
  });
  v.hidden = !!options.hidden;
  v.tickAnimationFrame = timestamp => {
    v.postMessage({
      method: 'tickAnimationFrame',
      timestamp,
    });
  };
  v.updateXrFrame = update => {
    v.postMessage({
      method: 'updateXrFrame',
      update,
    });
  };

  windowVms.push(v);
  v.on('exit', () => {
    windowVms.splice(windowVms.indexOf(v), 1);
  });

  return v;
};
module.exports._makeWindowVm = _makeWindowVm;

function tickAnimationFrame() {
  if (windowVms.length > 0) {
    const performanceNow = performance.now();

    // hidden
    for (let i = 0; i < windowVms.length; i++) {
      const windowVm = windowVms[i];

      if (windowVm.hidden) {
        tickAnimationFrame.windowVm = windowVm;

        windowVm.tickAnimationFrame(performanceNow);
      }
    }

    // visible
    for (let i = 0; i < windowVms.length; i++) {
      const windowVm = windowVms[i];

      if (!windowVm.hidden) {
        tickAnimationFrame.windowVm = windowVm;

        windowVm.tickAnimationFrame(performanceNow);
      }
    }

    tickAnimationFrame.windowVm = null;
  }
}
tickAnimationFrame.windowVm = null;
module.exports.tickAnimationFrame = tickAnimationFrame
