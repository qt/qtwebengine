// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

const express = require('express');
const webpush = require('web-push');

// setup server
const port = 5000;
const server = express();
server.use(express.json());
server.use(express.static('content'));

// we support only one subscription at the time
var subscription = null;

// setup vapid keys
const vapidKeys = {
    publicKey :
            "BNO4fIv439RpvbReeABNlDNiiBD2Maykn7EVnwsPseH7-P5hjnzZLEfnejXVP7Zt6MFoKqKeHm4nV9BHvbgoRPg",
    privateKey : "HqhrzsRfG5-SB3j45lyUmV7cYZuy-71r2Bb0tgaOefk"
};

// set vapid keys for webpush libs
webpush.setVapidDetails('mailto:push@qt.io', vapidKeys.publicKey, vapidKeys.privateKey);

// add subscribe route
server.post('/subscribe', (req, res) => {

    // subscription request
    subscription = req.body;
    const delay = req.headers['ping-time'];
    console.log('Got subscription with endpoint: ' + subscription.endpoint);
    console.log('Ping delay is at: ' + delay);

    // confirm resource created
    res.status(201).json({});

    // schedule notification
    setTimeout(() => { sendNotification(delay) }, delay * 1000);
});

// add unsubscribe route
server.post('/unsubscribe', (req, res) => {
    console.log('Got unsubscribe with endpoint: ' + req.body.endpoint);
    subscription = null;
    res.status(201).json({});
});

function sendNotification(delay)
{
    if (!subscription)
        return;

    // create payload text
    const payload = JSON.stringify({ title : 'Ping !', text : 'Visit qt.io', url : 'www.qt.io' });

    // send notification
    console.log('Sending notification !');
    webpush.sendNotification(subscription, payload).catch(err => console.error(err));

    // schedule next notification
    setTimeout(() => { sendNotification(delay) }, delay * 1000);
}

server.listen(port, () => console.log(`Push server started at port ${port}`));
