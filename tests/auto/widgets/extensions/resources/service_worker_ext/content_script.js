chrome.runtime.sendMessage("ping", (response) => {
    if (response === "pong") {
        let testNode = document.createElement("div")
        testNode.id = "testNode"
        document.body.appendChild(testNode)
    }
})
