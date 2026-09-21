// Evaluated at the top level of the worker, before any event handler runs.
const greeting = chrome.i18n.getMessage("greeting")
chrome.runtime.onMessage.addListener((message, _, sendResponse) => {
    if (message === "ping")
        sendResponse(greeting)
    return true;
})
