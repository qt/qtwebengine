chrome.runtime.sendMessage("ping", (response) => {
    let testNode = document.createElement("div")
    testNode.id = "testNode"
    testNode.textContent = response
    document.body.appendChild(testNode)
})
