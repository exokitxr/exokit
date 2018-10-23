const express = require('express');
const bodyParser = require('body-parser');
const { spawn } = require('child_process');
const port = 9000;
const app = express();

// app.post('/recover', bodyParser.json(), async (req, res) => {
//
// });

app.post('/launchTerminal', bodyParser.json(), async (req, res) => {
  console.log('launching terminal');
  spawn('C:\\Users\\ceddy\\Documents\\GitHub\\exokit\\scripts\\exokit.cmd', [], {detached: true, stdio: ['ignore', 'ignore', 'ignore']});
});

app.post('/launchExoHome', bodyParser.json(), async (req, res) => {
  console.log('launching ExoHome');
  spawn('C:\\Users\\ceddy\\Documents\\GitHub\\exokit\\scripts\\exokit.cmd', ['-h'], {detached: true, stdio: ['ignore', 'ignore', 'ignore']});
});

app.listen(port, 'localhost', () => console.log(`Example app listening on port ${port}!`));
