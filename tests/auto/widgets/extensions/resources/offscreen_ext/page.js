chrome.runtime.onMessage.addListener((message) => {
    if (message === "offscreen ready")
        document.title = "offscreen ready";
});
chrome.offscreen.createDocument({
    url: "offscreen.html",
    reasons: ["CLIPBOARD"],
    justification: "test"
}).catch((error) => document.title = "failed: " + error.message);
