// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

Item {

    WebEngineProfile { id: testProfile }

    TestWebEngineView {
        id: webEngineView
        width: 400
        height: 300
    }

    TestCase {
        name: "UserScriptCollection"

        function cleanup() {
            webEngineView.url = ""
            webEngineView.userScripts.collection = []
            compare(webEngineView.userScripts.collection.length, 0)
        }

        function test_collection() {
            let scriptFoo = { name: "Foo",
                sourceUrl: Qt.resolvedUrl("foo.js"),
                injectionPoint: WebEngineScript.DocumentReady
            }
            let scriptBar = WebEngine.script()

            scriptBar.name = "Bar"
            scriptBar.sourceUrl = Qt.resolvedUrl("bar.js")
            scriptBar.injectionPoint = WebEngineScript.DocumentReady

            compare(webEngineView.userScripts.collection.length, 0)
            webEngineView.userScripts.collection = [ scriptFoo, scriptBar ]
            compare(webEngineView.userScripts.collection.length, 2)
            compare(webEngineView.userScripts.collection[0].name, scriptFoo.name)
            compare(webEngineView.userScripts.collection[0].sourceUrl, scriptFoo.sourceUrl)
            compare(webEngineView.userScripts.collection[1].name, scriptBar.name)
            compare(webEngineView.userScripts.collection[1].sourceUrl, scriptBar.sourceUrl)
            webEngineView.userScripts.collection = []
            compare(webEngineView.userScripts.collection.length, 0)
        }

        function test_insert() {
            let scriptFoo = WebEngine.script()
            scriptFoo.name = "Foo"
            scriptFoo.sourceUrl = Qt.resolvedUrl("foo.js")
            scriptFoo.injectionPoint = WebEngineScript.DocumentReady
            let scriptBar = WebEngine.script()
            scriptBar.name = "Bar"
            scriptBar.sourceUrl = Qt.resolvedUrl("bar.js")
            scriptBar.injectionPoint = WebEngineScript.DocumentReady

            compare(webEngineView.userScripts.collection.length, 0)
            webEngineView.userScripts.insert(scriptFoo)
            webEngineView.userScripts.insert(scriptBar)
            compare(webEngineView.userScripts.collection.length, 2)
            compare(webEngineView.userScripts.collection[0].name, scriptFoo.name)
            compare(webEngineView.userScripts.collection[1].name, scriptBar.name)
            webEngineView.userScripts.collection = []
            compare(webEngineView.userScripts.collection.length, 0)

            var list = [ scriptFoo , scriptBar]
            webEngineView.userScripts.insert(list)
            compare(webEngineView.userScripts.collection.length, 2)
            compare(webEngineView.userScripts.collection[0].name, scriptFoo.name)
            compare(webEngineView.userScripts.collection[1].name, scriptBar.name)
        }

        function test_find() {
            let scriptA = WebEngine.script()
            scriptA.name = "A"
            scriptA.sourceUrl = Qt.resolvedUrl("A.js")
            let scriptB = WebEngine.script()
            scriptB.name = "A"
            scriptB.sourceUrl = Qt.resolvedUrl("B.js")
            let scriptC = WebEngine.script()
            scriptC.name = "C"
            scriptC.sourceUrl = Qt.resolvedUrl("C.js")

            compare(webEngineView.userScripts.collection.length, 0)
            webEngineView.userScripts.collection = [ scriptA, scriptB, scriptC ];
            compare(webEngineView.userScripts.collection.length, 3)
            let scriptsA = webEngineView.userScripts.find("A")
            let scriptsB = webEngineView.userScripts.find("B")
            let scriptsC = webEngineView.userScripts.find("C")
            compare(scriptsA.length, 2)
            compare(scriptsB.length, 0)
            compare(scriptsC.length, 1)
            compare(scriptsA[0].name, scriptA.name)
            compare(scriptsA[0].sourceUrl, scriptA.sourceUrl)
            compare(scriptsA[1].name, scriptB.name)
            compare(scriptsA[1].sourceUrl, scriptB.sourceUrl)
            compare(scriptsC[0].name, scriptC.name)
            compare(scriptsC[0].sourceUrl, scriptC.sourceUrl)
       }

       function test_contains() {
            let scriptFoo = WebEngine.script()
            scriptFoo.name = "Foo"
            let scriptBar = WebEngine.script()
            scriptBar.name = "Bar"
            compare(webEngineView.userScripts.collection.length, 0)
            webEngineView.userScripts.collection = [ scriptFoo ]
            compare(webEngineView.userScripts.collection.length, 1)
            verify(webEngineView.userScripts.contains(scriptFoo))
            verify(!webEngineView.userScripts.contains(scriptBar))
       }

       function test_clear() {
            let scriptFoo = WebEngine.script()
            scriptFoo.name = "Foo"
            let scriptBar = WebEngine.script()
            scriptBar.name = "Bar"
            compare(webEngineView.userScripts.collection.length, 0)
            webEngineView.userScripts.collection = [ scriptFoo ];
            compare(webEngineView.userScripts.collection.length, 1)
            webEngineView.userScripts.clear()
            compare(webEngineView.userScripts.collection.length, 0)
       }
    }
}
