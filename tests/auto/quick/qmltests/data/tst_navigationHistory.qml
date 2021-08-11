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
    width: 400
    height: 300

    ListView {
        id: backItemsList
        anchors.fill: parent
        model: webEngineView.history.backItems
        currentIndex: count - 1
        delegate:
            Text {
                color:"black"
                text: url
            }
    }

    ListView {
        id: forwardItemsList
        anchors.fill: parent
        model: webEngineView.history.forwardItems
        currentIndex: 0
        delegate:
            Text {
                color:"black"
                text: url
            }
    }

    Item { // simple button-like interface to not depend on controls
        id: backButton
        enabled: webEngineView.canGoBack
        function clicked() { if (enabled) webEngineView.goBack() }
    }

    Item { // simple button-like interface to not depend on controls
        id: forwardButton
        enabled: webEngineView.canGoForward
        function clicked() { if (enabled) webEngineView.goForward() }
    }

    TestCase {
        name: "NavigationHistory"

        function test_navigationHistory() {
            webEngineView.history.clear()

            webEngineView.url = Qt.resolvedUrl("test1.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.canGoBack, false)
            compare(webEngineView.canGoForward, false)
            compare(backItemsList.count, 0)
            compare(forwardItemsList.count, 0)

            webEngineView.url = Qt.resolvedUrl("test2.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("test2.html"))
            compare(webEngineView.canGoBack, true)
            compare(webEngineView.canGoForward, false)
            compare(backItemsList.count, 1)
            compare(backItemsList.currentItem.text, Qt.resolvedUrl("test1.html"))

            webEngineView.goBackOrForward(-1)
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("test1.html"))
            compare(webEngineView.canGoBack, false)
            compare(webEngineView.canGoForward, true)
            compare(backItemsList.count, 0)
            compare(forwardItemsList.count, 1)
            compare(forwardItemsList.currentItem.text, Qt.resolvedUrl("test2.html"))

            webEngineView.goForward()
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("test2.html"))
            compare(webEngineView.canGoBack, true)
            compare(webEngineView.canGoForward, false)
            compare(backItemsList.count, 1)
            compare(forwardItemsList.count, 0)
            compare(backItemsList.currentItem.text, Qt.resolvedUrl("test1.html"))

            webEngineView.url = Qt.resolvedUrl("javascript.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("javascript.html"))
            compare(webEngineView.canGoBack, true)
            compare(webEngineView.canGoForward, false)
            compare(backItemsList.count, 2)
            compare(forwardItemsList.count, 0)
            compare(backItemsList.currentItem.text, Qt.resolvedUrl("test1.html"))

            webEngineView.goBackOrForward(-2)
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("test1.html"))
            compare(webEngineView.canGoBack, false)
            compare(webEngineView.canGoForward, true)
            compare(backItemsList.count, 0)
            compare(forwardItemsList.count, 2)
            compare(forwardItemsList.currentItem.text, Qt.resolvedUrl("test2.html"))

            webEngineView.goBackOrForward(2)
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("javascript.html"))
            compare(webEngineView.canGoBack, true)
            compare(webEngineView.canGoForward, false)
            compare(backItemsList.count, 2)
            compare(forwardItemsList.count, 0)
            compare(backItemsList.currentItem.text, Qt.resolvedUrl("test1.html"))

            webEngineView.goBack()
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("test2.html"))
            compare(webEngineView.canGoBack, true)
            compare(webEngineView.canGoForward, true)
            compare(backItemsList.count, 1)
            compare(forwardItemsList.count, 1)
            compare(backItemsList.currentItem.text, Qt.resolvedUrl("test1.html"))
            compare(forwardItemsList.currentItem.text, Qt.resolvedUrl("javascript.html"))

            webEngineView.history.clear()
            compare(webEngineView.url, Qt.resolvedUrl("test2.html"))
            compare(webEngineView.canGoBack, false)
            compare(webEngineView.canGoForward, false)
            compare(backItemsList.count, 0)
            compare(forwardItemsList.count, 0)
        }

        function test_navigationButtons() {
            webEngineView.history.clear()

            const url1 = Qt.resolvedUrl("test1.html")
            webEngineView.url = url1
            verify(webEngineView.waitForLoadSucceeded())
            compare(backButton.enabled, false)
            compare(forwardButton.enabled, false)

            const url2 = Qt.resolvedUrl("test2.html")
            webEngineView.url = url2
            verify(webEngineView.waitForLoadSucceeded())
            compare(backButton.enabled, true)
            compare(forwardButton.enabled, false)

            const url3 = Qt.resolvedUrl("test3.html")
            webEngineView.url = url3
            verify(webEngineView.waitForLoadSucceeded())
            compare(backButton.enabled, true)
            compare(forwardButton.enabled, false)

            backButton.clicked()
            verify(webEngineView.waitForLoadSucceeded())
            compare(backButton.enabled, true)
            compare(forwardButton.enabled, true)
            compare(webEngineView.url, url2)

            backButton.clicked()
            verify(webEngineView.waitForLoadSucceeded())
            compare(backButton.enabled, false)
            compare(forwardButton.enabled, true)
            compare(webEngineView.url, url1)

            forwardButton.clicked()
            verify(webEngineView.waitForLoadSucceeded())
            compare(backButton.enabled, true)
            compare(forwardButton.enabled, true)
            compare(webEngineView.url, url2)

            webEngineView.url = url1
            verify(webEngineView.waitForLoadSucceeded())
            compare(backButton.enabled, true)
            compare(forwardButton.enabled, false)
            compare(webEngineView.url, url1)
        }
    }
}
