self.addEventListener('install', function(event) {
    event.waitUntil(self.skipWaiting());
});

self.addEventListener('activate', function(event) {
    event.waitUntil(self.clients.claim());
});
self.addEventListener('message', (event) => {
    if (event.data && event.data.type === 'PING') {
        self.clients.matchAll({includeUncontrolled: true, type: 'window'}).then((clients) => {
            if (clients && clients.length) {
                clients[0].postMessage({type: 'PONG'});
            }
        });
    }
});
