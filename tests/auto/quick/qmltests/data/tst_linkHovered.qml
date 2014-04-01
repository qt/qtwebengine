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

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400
    focus: true

    property string lastUrl
    property string lastTitle

    SignalSpy {
        id: spy
        target: webEngineView
        signalName: "linkHovered"
    }

    onLinkHovered: {
        webEngineView.lastUrl = hoveredUrl
        webEngineView.lastTitle = hoveredTitle
    }

    TestCase {
        name: "DesktopWebEngineViewLinkHovered"

        // Delayed windowShown to workaround problems with Qt5 in debug mode.
        when: false
        Timer {
            running: parent.windowShown
            repeat: false
            interval: 1
            onTriggered: parent.when = true
        }

        function init() {
            webEngineView.lastUrl = ""
            webEngineView.lastTitle = ""
            spy.clear()
        }

        function test_linkHovered() {
            compare(spy.count, 0)

            // If the next mouseMove is the first mouse action in the test,
            // the tooltip text won't get updated, so this mouseMove is needed
            // to update it.
            mouseMove(webEngineView, 100, 300)

            webEngineView.url = Qt.resolvedUrl("test2.html")
            verify(webEngineView.waitForLoadSucceeded())
            mouseMove(webEngineView, 100, 100)
            spy.wait()
            compare(spy.count, 1)
            compare(webEngineView.lastUrl, Qt.resolvedUrl("test1.html"))
            compare(webEngineView.lastTitle, "A title")
            mouseMove(webEngineView, 100, 300)
            spy.wait()
            compare(spy.count, 2)
            compare(webEngineView.lastUrl, "")
            compare(webEngineView.lastTitle, "")
        }

        function test_linkHoveredDoesntEmitRepeated() {
            compare(spy.count, 0)
            webEngineView.url = Qt.resolvedUrl("test2.html")
            verify(webEngineView.waitForLoadSucceeded())

            for (var i = 0; i < 100; i += 10)
                mouseMove(webEngineView, 100, 100 + i)

            spy.wait()
            compare(spy.count, 1)
            compare(webEngineView.lastUrl, Qt.resolvedUrl("test1.html"))

            for (var i = 0; i < 100; i += 10)
                mouseMove(webEngineView, 100, 300 + i)

            spy.wait()
            compare(spy.count, 2)
            compare(webEngineView.lastUrl, "")
        }
    }
}
