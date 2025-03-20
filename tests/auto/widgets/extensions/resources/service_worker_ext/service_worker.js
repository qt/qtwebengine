chrome.runtime.onMessage.addListener((message, _, sendResponse) => {
    if (message === "ping")
        sendResponse("pong")
        return true;
})
