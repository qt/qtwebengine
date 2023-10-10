// Copyright (C) 2014 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

import QtWebChannel

Item {
    id: test
    signal barCalled(var arg)
    signal clientInitializedCalled(var arg)

    QtObject {
        id: testObject
        WebChannel.id: "testObject"

        property var foo: 42

        function clientInitialized(arg)
        {
            clientInitializedCalled(arg);
        }

        function bar(arg) {
            barCalled(arg);
        }

        signal runTest(var foo)
    }

    TestWebEngineView {
        id: webView
        webChannel.registeredObjects: [testObject]
    }

    SignalSpy {
        id: initializedSpy
        target: test
        signalName: "clientInitializedCalled"
    }

    SignalSpy {
        id: barSpy
        target: test
        signalName: "barCalled"
    }

    TestCase {
        name: "WebViewWebChannel"
        property url testUrl: Qt.resolvedUrl("./webchannel-test.html")

        function init() {
            initializedSpy.clear();
            barSpy.clear();
        }

        function test_basic() {
            webView.userScripts.collection = [ {
                name: "qtwebchanneljs",
                sourceUrl: Qt.resolvedUrl("qrc:/qtwebchannel/qwebchannel.js"),
                injectionPoint: WebEngineScript.DocumentCreation,
                worldId: WebEngineScript.MainWorld
            }]
            webView.url = testUrl;
            verify(webView.waitForLoadSucceeded());

            initializedSpy.wait();
            compare(initializedSpy.signalArguments.length, 1);
            compare(initializedSpy.signalArguments[0][0], 42);

            var newValue = "roundtrip";
            testObject.runTest(newValue);
            barSpy.wait();
            compare(barSpy.signalArguments.length, 1);
            compare(barSpy.signalArguments[0][0], newValue);

            compare(testObject.foo, newValue);
        }
    }
}
