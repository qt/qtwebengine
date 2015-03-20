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
    id: parentItem
    width: 400
    height: 300

    property var pressEvents: []
    property var releaseEvents: []
    Keys.onPressed: pressEvents.push(event.key)
    Keys.onReleased: releaseEvents.push(event.key)

    TestWebEngineView {
        id: webEngineView
        anchors.fill: parent
        focus: true
    }
    TestCase {
        name: "WebEngineViewUnhandledKeyEventPropagation"

        when: false
        Timer {
            running: parent.windowShown
            repeat: false
            interval: 1
            onTriggered: parent.when = true
        }

        function test_keyboardModifierMapping() {
            webEngineView.loadHtml("<input type='text'/>")
            webEngineView.waitForLoadSucceeded()
            webEngineView.runJavaScript("document.body.firstChild.focus()")

            keyPress(Qt.Key_A)
            keyRelease(Qt.Key_A)
            keyPress(Qt.Key_Left)
            keyRelease(Qt.Key_Left)
            keyPress(Qt.Key_Left)
            keyRelease(Qt.Key_Left)

            for (var i = 0; i < 20 && parentItem.releaseEvents.length < 3; i++)
                wait(100)

            compare(parentItem.pressEvents.length, 1)
            compare(parentItem.pressEvents[0], Qt.Key_Left)
            compare(parentItem.releaseEvents.length, 3)
            compare(parentItem.releaseEvents[0], Qt.Key_A)
            compare(parentItem.releaseEvents[1], Qt.Key_Left)
            compare(parentItem.releaseEvents[2], Qt.Key_Left)
        }
    }
}
