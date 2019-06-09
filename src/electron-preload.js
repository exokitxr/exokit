window.postMessage = m => {
  console.log('<postMessage>' + JSON.stringify(m));
};