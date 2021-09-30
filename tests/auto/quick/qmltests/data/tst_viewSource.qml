/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400

    property var viewRequest: null

    SignalSpy {
        id: loadSpy
        target: webEngineView
        signalName: 'loadingChanged'
    }

    SignalSpy {
        id: newViewRequestedSpy
        target: webEngineView
        signalName: "newWindowRequested"
    }

    SignalSpy {
        id: titleChangedSpy
        target: webEngineView
        signalName: "titleChanged"
    }

    onNewWindowRequested: function(request) {
        viewRequest = {
            "destination": request.destination,
            "userInitiated": request.userInitiated
        };

        webEngineView.acceptAsNewWindow(request);
    }

    TestCase {
        id: test
        name: "WebEngineViewSource"

        function init() {
            webEngineView.loadStatus = null;
            webEngineView.url = Qt.resolvedUrl("test1.html");
            tryCompare(webEngineView, "loadStatus", WebEngineView.LoadSucceededStatus);
            webEngineView.loadStatus = null;

            loadSpy.clear();
            newViewRequestedSpy.clear();
            titleChangedSpy.clear();
            viewRequest = null;
        }

        function test_viewSource() {
            webEngineView.url = Qt.resolvedUrl("test1.html");
            verify(webEngineView.waitForLoadSucceeded());
            tryCompare(webEngineView, "title", "Test page 1");
            verify(webEngineView.action(WebEngineView.ViewSource).enabled);

            titleChangedSpy.clear();
            webEngineView.triggerWebAction(WebEngineView.ViewSource);
            tryCompare(newViewRequestedSpy, "count", 1);
            verify(webEngineView.waitForLoadSucceeded());
            // The first titleChanged signal is emitted by adoptWebContents()
            tryVerify(function() { return titleChangedSpy.count >= 2; });

            compare(viewRequest.destination, WebEngineNewWindowRequest.InNewTab);
            verify(viewRequest.userInitiated);
            verify(!webEngineView.action(WebEngineView.ViewSource).enabled);

            tryCompare(webEngineView, "title", "test1.html");
            compare(webEngineView.url, "view-source:" + Qt.resolvedUrl("test1.html"));
        }

        function test_viewSourceCredentials() {
            var url = "http://user:passwd@httpbin.org/basic-auth/user/passwd";

            // Test explicit view-source URL with credentials
            webEngineView.url = Qt.resolvedUrl("view-source:" + url);
            if (!webEngineView.waitForLoadSucceeded(12000))
                skip("Couldn't load page from network, skipping test.");

            compare(webEngineView.url, "view-source:" + url.replace("user:passwd@", ""));
            compare(webEngineView.title, "view-source:" + url.replace("http://user:passwd@", ""));
            titleChangedSpy.clear();

            // Test ViewSource web action on URL with credentials
            webEngineView.url = Qt.resolvedUrl(url);
            if (!webEngineView.waitForLoadSucceeded(12000))
                skip("Couldn't load page from network, skipping test.");
            webEngineView.triggerWebAction(WebEngineView.ViewSource);
            tryCompare(newViewRequestedSpy, "count", 1);

            // The first titleChanged signal is emitted by adoptWebContents()
            tryVerify(function() { return titleChangedSpy.count >= 2; });
            compare(viewRequest.destination, WebEngineNewWindowRequest.InNewTab);
            verify(viewRequest.userInitiated);

            tryCompare(webEngineView, "url", "view-source:" + url.replace("user:passwd@", ""));
            tryCompare(webEngineView, "title", "view-source:" + url.replace("http://user:passwd@", ""));
        }

        function test_viewSourceURL_data() {
            var testLocalUrl = "view-source:" + Qt.resolvedUrl("test1.html");
            var testLocalUrlWithoutScheme = "view-source:" + Qt.resolvedUrl("test1.html").toString().substring(7);

            return [
                   { tag: "view-source:", userInputUrl: "view-source:", loadSucceed: true, url: "view-source:", title: "view-source:" },
                   { tag: "view-source:about:blank", userInputUrl: "view-source:about:blank", loadSucceed: true, url: "view-source:about:blank", title: "view-source:about:blank" },
                   { tag: testLocalUrl, userInputUrl: testLocalUrl, loadSucceed: true, url: testLocalUrl, title: "test1.html" },
                   { tag: testLocalUrlWithoutScheme, userInputUrl: testLocalUrlWithoutScheme, loadSucceed: true, url: testLocalUrl, title: "test1.html" },
                   { tag: "view-source:http://non.existent", userInputUrl: "view-source:http://non.existent", loadSucceed: false, url: "http://non.existent/", title: "non.existent" },
                   { tag: "view-source:non.existent", userInputUrl: "view-source:non.existent", loadSucceed: false, url: "http://non.existent/", title: "non.existent" },
            ];
        }

        function test_viewSourceURL(row) {
            WebEngine.settings.errorPageEnabled = true
            webEngineView.url = row.userInputUrl;

            tryCompare(loadSpy, 'count', 2);
            let load = loadSpy.signalArguments[1][0]
            let expectedStatus = row.loadSucceed ? WebEngineView.LoadSucceededStatus : WebEngineView.LoadFailedStatus
            compare(load.status, expectedStatus);
            compare(load.isErrorPage, !row.loadSucceed);
            tryVerify(function() { return titleChangedSpy.count == 1; });

            compare(webEngineView.url, row.url);
            tryCompare(webEngineView, "title", row.title);
            if (row.loadSucceed) {
                verify(!webEngineView.action(WebEngineView.ViewSource).enabled);
            } else {
                verify(webEngineView.action(WebEngineView.ViewSource).enabled);
            }
        }
    }
}

