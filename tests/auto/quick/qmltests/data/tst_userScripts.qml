/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtTest 1.0
import QtWebEngine 1.1

Item {
    WebEngineScript {
        id: changeDocumentTitleScript
        sourceUrl: Qt.resolvedUrl("change-document-title.js")
        injectionPoint: WebEngineScript.DocumentReady
    }

    WebEngineScript {
        id: appendDocumentTitleScript
        sourceUrl: Qt.resolvedUrl("append-document-title.js")
        injectionPoint: WebEngineScript.DocumentReady
    }

    WebEngineScript {
        id: bigUserScript
        sourceUrl: Qt.resolvedUrl("big-user-script.js")
        injectionPoint: WebEngineScript.DocumentReady
    }

    TestWebEngineView {
        id: webEngineView
        width: 400
        height: 300
    }

    TestWebEngineView {
        id: webEngineViewWithConditionalUserScripts
        width: 400
        height: 300

        onNavigationRequested: {
            var urlString = request.url.toString();
            if (urlString.indexOf("test1.html") !== -1)
                userScripts = [ changeDocumentTitleScript ];
            else if (urlString.indexOf("test2.html") !== -1)
                userScripts = [ appendDocumentTitleScript ];
            else
                userScripts = [];
        }
    }

    TestCase {
        name: "WebEngineViewUserScripts"

        function init() {
            webEngineView.url = "";
            webEngineView.userScripts = [];
        }

        function test_oneScript() {
            webEngineView.url = Qt.resolvedUrl("test1.html");
            webEngineView.waitForLoadSucceeded();
            compare(webEngineView.title, "Test page 1");

            webEngineView.userScripts = [ changeDocumentTitleScript ];
            compare(webEngineView.title, "Test page 1");

            webEngineView.reload();
            webEngineView.waitForLoadSucceeded();
            compare(webEngineView.title, "New title");

            webEngineView.url = Qt.resolvedUrl("test2.html");
            webEngineView.waitForLoadSucceeded();
            compare(webEngineView.title, "New title");

            webEngineView.userScripts = [];
            compare(webEngineView.title, "New title");

            webEngineView.reload();
            webEngineView.waitForLoadSucceeded();
            compare(webEngineView.title, "Test page with huge link area");
        }

        function test_twoScripts() {
            webEngineView.url = Qt.resolvedUrl("test1.html");
            webEngineView.waitForLoadSucceeded();
            compare(webEngineView.title, "Test page 1");

            webEngineView.userScripts = [ changeDocumentTitleScript, appendDocumentTitleScript ];

            // Make sure the scripts are loaded in order.
            appendDocumentTitleScript.injectionPoint = WebEngineScript.Deferred
            webEngineView.reload();
            webEngineView.waitForLoadSucceeded();
            compare(webEngineView.title, "New title with appendix");

            appendDocumentTitleScript.injectionPoint = WebEngineScript.DocumentReady
            changeDocumentTitleScript.injectionPoint = WebEngineScript.Deferred
            webEngineView.reload();
            webEngineView.waitForLoadSucceeded();
            compare(webEngineView.title, "New title");

            // Make sure we can remove scripts from the preload list.
            webEngineView.userScripts = [ appendDocumentTitleScript ];
            webEngineView.reload();
            webEngineView.waitForLoadSucceeded();
            compare(webEngineView.title, "Test page 1 with appendix");

            changeDocumentTitleScript.injectionPoint = WebEngineScript.DocumentReady
        }

        function test_setUserScriptsConditionally() {
            webEngineViewWithConditionalUserScripts.url = Qt.resolvedUrl("test1.html");
            webEngineViewWithConditionalUserScripts.waitForLoadSucceeded();
            compare(webEngineViewWithConditionalUserScripts.title, "New title");

            webEngineViewWithConditionalUserScripts.url = Qt.resolvedUrl("test2.html");
            webEngineViewWithConditionalUserScripts.waitForLoadSucceeded();
            compare(webEngineViewWithConditionalUserScripts.title, "Test page with huge link area with appendix");

            webEngineViewWithConditionalUserScripts.url = Qt.resolvedUrl("test3.html");
            webEngineViewWithConditionalUserScripts.waitForLoadSucceeded();
            compare(webEngineViewWithConditionalUserScripts.title, "Test page 3");
        }

        function test_bigScript() {
            webEngineView.userScripts = [ bigUserScript ];
            webEngineView.url = Qt.resolvedUrl("test1.html");
            webEngineView.waitForLoadSucceeded();
            compare(webEngineView.title, "Big user script changed title");
        }
    }
}
