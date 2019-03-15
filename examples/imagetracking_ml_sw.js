var cacheName = 'imagetracking_ml-page';
var filesToCache = [
  '/',
  '/imagetracking_ml.html',
  '/tracker.png',
  '/sword.png',
  '/three.js',
  '/inflate.min.js',
  '/FBXLoader.js',
  '/sprite3d.js'
];

self.addEventListener('install', function(e) {
  console.log('[ServiceWorker] Install');
  e.waitUntil(
    caches.open(cacheName).then(function(cache) {
      console.log('[ServiceWorker] Caching app shell');
      return cache.addAll(filesToCache);
    })
  );
});

self.addEventListener('activate',  event => {
  event.waitUntil(self.clients.claim());
});

self.addEventListener('fetch', event => {
  event.respondWith(
    caches.match(event.request, {ignoreSearch:true}).then(response => {
      return response || fetch(event.request);
    })
  );
});
