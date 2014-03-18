/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Quick Controls module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtTest 1.0
import QtWebEngine 1.0
import QtWebEngine.experimental 1.0

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    ListView {
        id: backItemsList
        anchors.fill: parent
        model: webEngineView.experimental.navigationHistory.backItems
        delegate:
            Text {
                color:"black"
                text: "title : " + title
            }
    }

    ListView {
        id: backForwardItemsList
        anchors.fill: parent
        model: webEngineView.experimental.navigationHistory.items
        delegate:
            Text {
                color:"black"
                text: "title : " + title
            }
    }

    ListView {
        id: forwardItemsList
        anchors.fill: parent
        model: webEngineView.experimental.navigationHistory.forwardItems
        delegate:
            Text {
                color:"black"
                text: "title : " + title
            }
    }

    TestCase {
        name: "WebViewNavigationHistory"

        function test_navigationHistory() {
            compare(webEngineView.loadProgress, 0)

            webEngineView.url = Qt.resolvedUrl("test1.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.canGoBack, false)
            compare(webEngineView.canGoForward, false)
            compare(webEngineView.experimental.currentNavigationEntryIndex, 0)
            compare(backForwardItemsList.count, 1)
            compare(backItemsList.count, 0)
            compare(forwardItemsList.count, 0)

            webEngineView.url = Qt.resolvedUrl("test2.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("test2.html"))
            compare(webEngineView.canGoBack, true)
            compare(webEngineView.canGoForward, false)
            compare(webEngineView.experimental.currentNavigationEntryIndex, 1)
            compare(backForwardItemsList.count, 2)
            compare(backItemsList.count, 1)

            webEngineView.experimental.goBackTo(0)
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("test1.html"))
            compare(webEngineView.canGoBack, false)
            compare(webEngineView.canGoForward, true)
            compare(webEngineView.experimental.currentNavigationEntryIndex, 0)
            compare(backForwardItemsList.count, 2)
            compare(backItemsList.count, 0)
            compare(forwardItemsList.count, 1)

            webEngineView.goForward()
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("test2.html"))
            compare(webEngineView.canGoBack, true)
            compare(webEngineView.canGoForward, false)
            compare(webEngineView.experimental.currentNavigationEntryIndex, 1)
            compare(backForwardItemsList.count, 2)
            compare(backItemsList.count, 1)
            compare(forwardItemsList.count, 0)

            webEngineView.url = Qt.resolvedUrl("javascript.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("javascript.html"))
            compare(webEngineView.canGoBack, true)
            compare(webEngineView.canGoForward, false)
            compare(webEngineView.experimental.currentNavigationEntryIndex, 2)
            compare(backForwardItemsList.count, 3)
            compare(backItemsList.count, 2)
            compare(forwardItemsList.count, 0)

            webEngineView.experimental.goBackTo(0)
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("test1.html"))
            compare(webEngineView.canGoBack, false)
            compare(webEngineView.canGoForward, true)
            compare(webEngineView.experimental.currentNavigationEntryIndex, 0)
            compare(backForwardItemsList.count, 3)
            compare(backItemsList.count, 0)
            compare(forwardItemsList.count, 2)

            webEngineView.experimental.goForwardTo(1)
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("javascript.html"))
            compare(webEngineView.canGoBack, true)
            compare(webEngineView.canGoForward, false)
            compare(webEngineView.experimental.currentNavigationEntryIndex, 2)
            compare(backForwardItemsList.count, 3)
            compare(backItemsList.count, 2)
            compare(forwardItemsList.count, 0)

            webEngineView.goBack()
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("test2.html"))
            compare(webEngineView.canGoBack, true)
            compare(webEngineView.canGoForward, true)
            compare(webEngineView.experimental.currentNavigationEntryIndex, 1)
            compare(backForwardItemsList.count, 3)
            compare(backItemsList.count, 1)
            compare(forwardItemsList.count, 1)
        }
    }
}
