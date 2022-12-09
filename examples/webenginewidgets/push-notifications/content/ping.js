// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

const publicVapidKey =
        "BNO4fIv439RpvbReeABNlDNiiBD2Maykn7EVnwsPseH7-P5hjnzZLEfnejXVP7Zt6MFoKqKeHm4nV9BHvbgoRPg";

async function setup(delay)
{
    console.log('>> register service worker...');
    const register = await navigator.serviceWorker.register('/worker.js', { scope : '/' });
    console.log('>> service worker registered...');

    console.log('>> subscribe push subscription to FCM');
    var subscription = await register.pushManager.subscribe(
            { userVisibleOnly : true, applicationServerKey : publicVapidKey });
    console.log('>> subscription created...');

    console.log('>> subscribe to push service...');
    await fetch('/subscribe', {
        method : 'POST',
        body : JSON.stringify(subscription),
        headers : { 'content-type' : 'application/json', 'ping-time' : delay }
    });
    console.log('>> push subscription created...')
}

async function clear()
{
    const register = await navigator.serviceWorker.getRegistration();
    var subscription = await register.pushManager.getSubscription();
    console.log('>> unsubscribe to push service...');
    await fetch('/unsubscribe', {
        method : 'POST',
        body : JSON.stringify(subscription),
        headers : { 'content-type' : 'application/json' }
    });
    console.log('>> push unsubscription removed...')

    console.log('>> unsubscribe push subscription to FCM');
    await subscription.unsubscribe();
    console.log('>> subscription removed...');
}

function displaySetup(delay = null)
{
    const pingSetup = document.getElementById('ping-setup');
    const pingClear = document.getElementById('ping-clear');
    const pingText = document.getElementById('ping-text');
    if (delay) {
        pingClear.style.display = 'block';
        pingSetup.style.display = 'none';
        pingText.innerHTML = 'Ping Me Every ' + delay + ' seconds';
    } else {
        pingClear.style.display = 'none';
        pingSetup.style.display = 'block';
        pingText.innerHTML = "";
    }
}

function handleSetupPing(event)
{
    event.preventDefault();

    const seconds = document.forms[0].seconds.value;
    document.forms[0].reset();

    // check for service worker support
    if ('serviceWorker' in navigator) {
        setup(seconds).catch(err => console.error(err));
    }

    displaySetup(seconds);
};

function handleClearPing(event)
{
    event.preventDefault();
    clear();
    displaySetup();
};

document.forms[0].addEventListener('submit', handleSetupPing);
const clearButton = document.getElementById('ping-clear-button');
clearButton.addEventListener('click', handleClearPing);
