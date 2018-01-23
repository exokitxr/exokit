const browserPoly = require('.');

process.on('uncaughtException', err => {
  console.warn(err.stack);
});
process.on('unhandledRejection', err => {
  console.warn(err.stack);
});

const window = browserPoly(`\
  <html>
    <body>
      <script>
        console.log('run inline script');
      </script>
      <script src="/archae/index.js"></script>
    </body>
  </html>
`, {
  url: `http://127.0.0.1:8000`,
});
window.addEventListener('error', err => {
  console.warn('got error', err.error.stack);
});
const {document} = window;

const blob = new window.Blob(['function lol() { console.log("run blob script"); }; lol();'], {type: 'application/javascript'});
const worker = new window.Worker(blob);
/* setTimeout(() => {
  worker.terminate();
}, 1000); */
