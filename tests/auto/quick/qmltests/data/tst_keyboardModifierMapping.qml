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

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    SignalSpy {
        id: titleSpy
        target: webEngineView
        signalName: "titleChanged"
    }

    TestCase {
        name: "WebEngineViewKeyboardModifierMapping"

        when: false
        Timer {
            running: parent.windowShown
            repeat: false
            interval: 1
            onTriggered: parent.when = true
        }

        function test_keyboardModifierMapping() {
            webEngineView.url = Qt.resolvedUrl("keyboardModifierMapping.html")
            waitForLoadSucceeded();
            titleSpy.wait()
            var callbackCalled = false;

            // Alt
            keyPress(Qt.Key_Alt);
            titleSpy.wait()
            runJavaScript("getPressedModifiers()", function(result) {
                    compare(result, "alt:pressed ctrl:no meta:no");
                    callbackCalled = true;
                });
            wait(100);
            verify(callbackCalled);
            keyRelease(Qt.Key_Alt)
            titleSpy.wait()
            callbackCalled = false;

            // Ctrl
            // On mac Qt automatically translates Meta to Ctrl and vice versa.
            // However, if sending the events manually no mapping is being done,
            // so we have to do this here manually.
            // For testing we assume that the flag Qt::AA_MacDontSwapCtrlAndMeta is NOT set.
            keyPress(Qt.platform.os == "osx" ? Qt.Key_Meta : Qt.Key_Control);
            titleSpy.wait()
            runJavaScript("getPressedModifiers()", function(result) {
                    compare(result, "alt:released ctrl:pressed meta:no");
                    callbackCalled = true;
                });
            wait(100);
            verify(callbackCalled);
            keyRelease(Qt.platform.os == "osx" ? Qt.Key_Meta : Qt.Key_Control);
            titleSpy.wait()
            callbackCalled = false;

            // Meta (Command on Mac)
            keyPress(Qt.platform.os == "osx" ? Qt.Key_Control : Qt.Key_Meta);
            titleSpy.wait()
            runJavaScript("getPressedModifiers()", function(result) {
                    compare(result, "alt:released ctrl:released meta:pressed");
                    callbackCalled = true;
                });
            wait(100);
            verify(callbackCalled);
            keyRelease(Qt.platform.os == "osx" ? Qt.Key_Control : Qt.Key_Meta);
            titleSpy.wait()
            callbackCalled = false;

            runJavaScript("getPressedModifiers()", function(result) {
                    compare(result, "alt:released ctrl:released meta:released");
                    callbackCalled = true;
                });
            wait(100);
            verify(callbackCalled);
            callbackCalled = false;
        }
    }
}
