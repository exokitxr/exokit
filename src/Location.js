const {EventEmitter} = require('events');
const url = require('url');

const {
  workerData: {
    args: {
      options: {
        baseUrl,
      },
    },
  },
} = require('worker_threads');

class Location extends EventEmitter {
  constructor(u) {
    super();

    this._url = new url.URL(u);
  }
  // triggers navigation
  get href() { return this._url.href || ''; }
  set href(href) {
    const oldUrl = this._url;
    const newUrl = new url.URL(href, baseUrl);
    this._url = newUrl;
    if (
      newUrl.origin !== oldUrl.origin ||
      newUrl.protocol !== oldUrl.protocol ||
      newUrl.username !== oldUrl.username ||
      newUrl.password !== oldUrl.password ||
      newUrl.host !== oldUrl.host ||
      newUrl.hostname !== oldUrl.hostname ||
      newUrl.port !== oldUrl.port ||
      newUrl.pathname !== oldUrl.pathname ||
      newUrl.search !== oldUrl.search
    ) {
      this.update();
    }
  }
  get protocol() { return this._url.protocol || ''; }
  set protocol(protocol) { this._url.protocol = protocol; this.update(); }
  get host() { return this._url.host || ''; }
  set host(host) { this._url.host = host; this.update(); }
  get hostname() { return this._url.hostname || ''; }
  set hostname(hostname) { this._url.hostname = hostname; this.update(); }
  get port() { return this._url.port || ''; }
  set port(port) { this._url.port = port; this.update(); }
  get pathname() { return this._url.pathname || ''; }
  set pathname(pathname) { this._url.pathname = pathname; this.update(); }
  get search() { return this._url.search || ''; }
  set search(search) { this._url.search = search; this.update(); }
  // does not trigger navigation
  get hash() { return this._url.hash || ''; }
  set hash(hash) { this._url.hash = hash; }
  get username() { return this._url.username || ''; }
  set username(username) { this._url.username = username; }
  get password() { return this._url.password || ''; }
  set password(password) { this._url.password = password; }
  get origin() { return this._url.origin || ''; }
  set origin(origin) {} // read only
  // methods
  reload() {
    this.href = this.href;
  }
  replace(href) {
    this.href = href;
  }
  // conversions
  toString() {
    return this.href;
  }
  // helpers
  set(u) {
    this._url.href = u;
  }
  update() {
    this.emit('update', this.href);
  }
}
module.exports.Location = Location;
