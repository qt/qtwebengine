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

import QtQuick 2.0
import QtTest 1.0
import QtWebEngine 1.2
import QtWebEngine.testsupport 1.0

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    testSupport: WebEngineTestSupport {
        id: testSupportAPI
    }

    SignalSpy {
        id: showSpy
        target: testSupportAPI
        signalName: "validationMessageShown"
    }

    TestCase {
        name: "WebEngineViewFormValidation"
        when: windowShown

        function init() {
            webEngineView.url = Qt.resolvedUrl("about:blank")
            verify(webEngineView.waitForLoadSucceeded())
            showSpy.clear()
        }

        function test_urlForm() {
            webEngineView.url = Qt.resolvedUrl("forms.html#url_empty")
            verify(webEngineView.waitForLoadSucceeded())
            keyPress(Qt.Key_Enter)
            showSpy.wait()
            compare(showSpy.signalArguments[0][0], "Please fill out this field.")

            webEngineView.url = Qt.resolvedUrl("about:blank")
            verify(webEngineView.waitForLoadSucceeded())

            webEngineView.url = Qt.resolvedUrl("forms.html#url_invalid")
            verify(webEngineView.waitForLoadSucceeded())
            keyPress(Qt.Key_Enter)
            showSpy.wait()
            compare(showSpy.signalArguments[1][0], "Please enter a URL.")

            webEngineView.url = Qt.resolvedUrl("about:blank")
            verify(webEngineView.waitForLoadSucceeded())

            webEngineView.url = Qt.resolvedUrl("forms.html#url_title")
            verify(webEngineView.waitForLoadSucceeded())
            keyPress(Qt.Key_Enter)
            showSpy.wait()
            compare(showSpy.signalArguments[2][1], "url_title")
        }

        function test_emailForm() {
            webEngineView.url = Qt.resolvedUrl("forms.html#email_empty")
            verify(webEngineView.waitForLoadSucceeded())
            keyPress(Qt.Key_Enter)
            showSpy.wait()
            compare(showSpy.signalArguments[0][0], "Please fill out this field.")

            webEngineView.url = Qt.resolvedUrl("about:blank")
            verify(webEngineView.waitForLoadSucceeded())

            webEngineView.url = Qt.resolvedUrl("forms.html#email_invalid")
            verify(webEngineView.waitForLoadSucceeded())
            keyPress(Qt.Key_Enter)
            showSpy.wait()
            compare(showSpy.signalArguments[1][0], "Please include an '@' in the email address. 'invalid' is missing an '@'.")

            webEngineView.url = Qt.resolvedUrl("about:blank")
            verify(webEngineView.waitForLoadSucceeded())

            webEngineView.url = Qt.resolvedUrl("forms.html#email_title")
            verify(webEngineView.waitForLoadSucceeded())
            keyPress(Qt.Key_Enter)
            showSpy.wait()
            compare(showSpy.signalArguments[2][1], "email_title")
        }
    }
}
