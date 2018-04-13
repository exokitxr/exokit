const http = require('http');
const express = require('express');

const port = 8000;

const app = express();
app.use(express.static(__dirname));
http.createServer(app)
  .listen(port, err => {
    if (!err) {
      console.log(`listening on http://127.0.0.1:${port}`);
    } else {
      throw err;
    }
  });
