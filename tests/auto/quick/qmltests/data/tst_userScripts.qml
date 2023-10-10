// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

Item {

    function changeDocumentTitleScript() {
        return { name: "changeDocumentTitleScript",
                  sourceUrl: Qt.resolvedUrl("change-document-title.js"),
                  injectionPoint: WebEngineScript.DocumentReady }
    }

    function appendDocumentTitleScript() {
        return { sourceUrl: Qt.resolvedUrl("append-document-title.js"),
                  injectionPoint: WebEngineScript.DocumentReady }
    }

    function bigUserScript() {
        return { sourceUrl: Qt.resolvedUrl("big-user-script.js"),
                  injectionPoint: WebEngineScript.DocumentReady }
    }

    function scriptWithMetadata() {
        var script = WebEngine.script()
        script.sourceUrl = Qt.resolvedUrl("script-with-metadata.js")
        return script
    }

    function scriptWithBadMatchMetadata() {
        var script = WebEngine.script()
        script.sourceUrl = Qt.resolvedUrl("script-with-bad-match-metadata.js")
        return script
    }

    WebEngineProfile { id: testProfile }

    TestWebEngineView {
        id: webEngineView
        width: 400
        height: 300
    }

    TestWebEngineView {
        id: webEngineView2
        width: 400
        height: 300
    }

    TestWebEngineView {
        id: webEngineViewWithConditionalUserScripts
        width: 400
        height: 300

        onNavigationRequested: function(request) {
            var urlString = request.url.toString();
            if (urlString.indexOf("test1.html") !== -1)
                userScripts.collection = [ changeDocumentTitleScript() ];
            else if (urlString.indexOf("test2.html") !== -1)
                userScripts.collection = [ appendDocumentTitleScript() ];
            else
                userScripts.collection = [];
        }
    }

    TestCase {
        name: "UserScripts"

        function cleanup() {
            webEngineView.url = "";
            webEngineView.userScripts.collection = [];
            compare(webEngineView.userScripts.collection.length, 0)
            webEngineView.profile.userScripts.collection = [];
            compare(webEngineView.profile.userScripts.collection.length, 0)
        }

        function test_profileScripts() {
            // assusme it is the same type as in View
            let t1 = String(testProfile.userScripts), t2 = String(webEngineView.userScripts)
            compare(t1.substr(0, t1.indexOf('(')), t2.substr(0, t2.indexOf('(')))

            // ... and just test basic things like access
            compare(testProfile.userScripts.collection, [])
            let script = changeDocumentTitleScript()
            testProfile.userScripts.collection = [ script ]

            compare(testProfile.userScripts.collection.length, 1)
            compare(testProfile.userScripts.collection[0].name, script.name)
        }

        function test_oneScript() {
            webEngineView.url = Qt.resolvedUrl("test1.html");
            webEngineView.waitForLoadSucceeded();
            tryCompare(webEngineView, "title", "Test page 1");

            let script = changeDocumentTitleScript()
            webEngineView.userScripts.collection = [ script ]
            compare(webEngineView.userScripts.collection.length, 1)
            compare(webEngineView.userScripts.collection[0].name, script.name)
            compare(webEngineView.title, "Test page 1");

            webEngineView.reload();
            webEngineView.waitForLoadSucceeded();
            tryCompare(webEngineView, "title", "New title");

            webEngineView.url = Qt.resolvedUrl("test2.html");
            webEngineView.waitForLoadSucceeded();
            tryCompare(webEngineView, "title", "New title");

            webEngineView.userScripts.collection = [];
            compare(webEngineView.userScripts.collection.length, 0)
            compare(webEngineView.title, "New title");

            webEngineView.reload();
            webEngineView.waitForLoadSucceeded();
            tryCompare(webEngineView, "title", "Test page with huge link area");
        }

        function test_twoScripts() {
            webEngineView.url = Qt.resolvedUrl("test1.html");
            webEngineView.waitForLoadSucceeded();
            tryCompare(webEngineView, "title", "Test page 1");
            var script1 = changeDocumentTitleScript();
            var script2 = appendDocumentTitleScript();
            script2.injectionPoint = WebEngineScript.Deferred;
            webEngineView.userScripts.collection = [ script1, script2 ];
            compare(webEngineView.userScripts.collection.length, 2)

            // Make sure the scripts are loaded in order.
            webEngineView.reload();
            webEngineView.waitForLoadSucceeded();
            tryCompare(webEngineView, "title", "New title with appendix");

            script2.injectionPoint = WebEngineScript.DocumentReady
            script1.injectionPoint = WebEngineScript.Deferred
            webEngineView.userScripts.collection = [ script1, script2 ];
            compare(webEngineView.userScripts.collection.length, 2)
            webEngineView.reload();
            webEngineView.waitForLoadSucceeded();
            tryCompare(webEngineView, "title", "New title");

            // Make sure we can remove scripts from the preload list.
            webEngineView.userScripts.collection = [ script2 ];
            compare(webEngineView.userScripts.collection.length, 1)
            webEngineView.reload();
            webEngineView.waitForLoadSucceeded();
            tryCompare(webEngineView, "title", "Test page 1 with appendix");

            changeDocumentTitleScript.injectionPoint = WebEngineScript.DocumentReady
        }

        function test_setUserScriptsConditionally() {
            webEngineViewWithConditionalUserScripts.url = Qt.resolvedUrl("test1.html");
            webEngineViewWithConditionalUserScripts.waitForLoadSucceeded();
            tryCompare(webEngineViewWithConditionalUserScripts, "title", "New title");

            webEngineViewWithConditionalUserScripts.url = Qt.resolvedUrl("test2.html");
            webEngineViewWithConditionalUserScripts.waitForLoadSucceeded();
            tryCompare(webEngineViewWithConditionalUserScripts, "title", "Test page with huge link area with appendix");

            webEngineViewWithConditionalUserScripts.url = Qt.resolvedUrl("test3.html");
            webEngineViewWithConditionalUserScripts.waitForLoadSucceeded();
            tryCompare(webEngineViewWithConditionalUserScripts, "title", "Test page 3");
        }

        function test_bigScript() {
            webEngineView.userScripts.collection = [ bigUserScript() ];
            compare(webEngineView.userScripts.collection.length, 1)
            webEngineView.url = Qt.resolvedUrl("test1.html");
            webEngineView.waitForLoadSucceeded();
            tryCompare(webEngineView , "title", "Big user script changed title");
        }

        function test_parseMetadataHeader() {
            var script = scriptWithMetadata()
            compare(script.name, "Test script");
            compare(script.injectionPoint, WebEngineScript.DocumentReady);

            webEngineView.userScripts.collection = [ script ];
            compare(webEngineView.userScripts.collection.length, 1)
            compare(webEngineView.userScripts.collection[0].name, script.name)

            // @include *data/test*.html
            webEngineView.url = Qt.resolvedUrl("test1.html");
            webEngineView.waitForLoadSucceeded();
            tryCompare(webEngineView, "title", "New title");

            // @exclude *test2.html
            webEngineView.url = Qt.resolvedUrl("test2.html");
            webEngineView.waitForLoadSucceeded();
            tryCompare(webEngineView, "title", "Test page with huge link area");

            // @include /favicon.html?$/
            webEngineView.url = Qt.resolvedUrl("favicon.html");
            webEngineView.waitForLoadSucceeded();
            tryCompare(webEngineView, "title", "New title");

            // @exclude /test[-]iframe/
            webEngineView.url = Qt.resolvedUrl("test-iframe.html");
            webEngineView.waitForLoadSucceeded();
            tryCompare(webEngineView, "title", "Test page with huge link area and iframe");
        }

        function test_dontInjectBadUrlPatternsEverywhere() {
            var script = scriptWithBadMatchMetadata();
            compare(script.name, "Test bad match script");
            compare(script.injectionPoint, WebEngineScript.DocumentReady);

            webEngineView.userScripts.collection = [ script ];
            compare(webEngineView.userScripts.collection.length, 1)

            // @match some:junk
            webEngineView.url = Qt.resolvedUrl("test2.html");
            webEngineView.waitForLoadSucceeded();
            tryCompare(webEngineView, "title", "Test page with huge link area");
        }

        function test_profileWideScript() {
            let script = changeDocumentTitleScript()
            webEngineView.profile.userScripts.collection = [ script ];
            compare(webEngineView.profile.userScripts.collection.length, 1)
            compare(webEngineView.profile.userScripts.collection[0].name, script.name)

            webEngineView.url = Qt.resolvedUrl("test1.html");
            webEngineView.waitForLoadSucceeded();
            compare(webEngineView.title, "New title");

            webEngineView2.url = Qt.resolvedUrl("test1.html");
            webEngineView2.waitForLoadSucceeded();
            compare(webEngineView2.title, "New title");
        }
    }
}
