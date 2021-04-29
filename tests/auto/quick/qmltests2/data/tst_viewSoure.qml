/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

import QtQuick 2.0
import QtTest 1.0
import QtWebEngine 1.4
import QtWebEngine.testsupport 1.0
import "../../qmltests/data" 1.0

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400

    property var viewRequest: null
    property var loadRequestArray: []

    testSupport: WebEngineTestSupport {
        errorPage.onLoadingChanged: {
            loadRequestArray.push({
               "status": loadRequest.status,
               "url": loadRequest.url
            })
        }
    }

    onLoadingChanged: {
        loadRequestArray.push({
            "status": loadRequest.status,
            "url": loadRequest.url
        });
    }

    SignalSpy {
        id: newViewRequestedSpy
        target: webEngineView
        signalName: "newViewRequested"
    }

    SignalSpy {
        id: titleChangedSpy
        target: webEngineView
        signalName: "titleChanged"
    }

    onNewViewRequested: {
        viewRequest = {
            "destination": request.destination,
            "userInitiated": request.userInitiated
        };

        request.openIn(webEngineView);
    }

    TestCase {
        id: testCase
        name: "WebEngineViewSource"

        function init() {
            webEngineView.loadStatus = null;
            webEngineView.url = Qt.resolvedUrl("test1.html");
            tryCompare(webEngineView, "loadStatus", WebEngineView.LoadSucceededStatus);
            webEngineView.loadStatus = null;

            newViewRequestedSpy.clear();
            titleChangedSpy.clear();
            viewRequest = null;
        }

        function test_viewSourceURL_data() {
            var testLocalUrl = "view-source:" + Qt.resolvedUrl("test1.html");
            var testLocalUrlWithoutScheme = "view-source:" + Qt.resolvedUrl("test1.html").substring(7);

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
            loadRequestArray = [];
            WebEngine.settings.errorPageEnabled = true
            webEngineView.url = row.userInputUrl;


            if (row.loadSucceed) {
                tryVerify(function() { return loadRequestArray.length == 2 });
                compare(loadRequestArray[1].status, WebEngineView.LoadSucceededStatus);
            } else {
                tryVerify(function() { return loadRequestArray.length == 4 }, 90000);
                // error page load is done inside main load through test support
                compare(loadRequestArray[2].status, WebEngineView.LoadSucceededStatus);
                compare(loadRequestArray[2].url, "chrome-error://chromewebdata/")
                compare(loadRequestArray[3].status, WebEngineView.LoadFailedStatus);
            }
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

