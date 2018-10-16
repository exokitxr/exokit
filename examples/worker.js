console.log('worker 1 loaded');

importScripts(self.location.pathname.split('/').slice(0,-1).join('/') + '/worker2.js');

onmessage = m => {
  console.log('worker got message', m.data);

  postMessage({world: true});
};
