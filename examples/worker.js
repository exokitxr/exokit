console.log('worker 1');

importScripts(self.location.pathname.split('/').slice(0,-1).join('/') + '/worker2.js');

console.log('worker 2');

onmessage = m => {
  console.log('worker message 1', m.data);

  postMessage({world: true});

  console.log('worker message 2', m.data);
};
