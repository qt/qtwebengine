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

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400

    SignalSpy {
        id: spy
        target: webEngineView
        signalName: "iconChanged"
    }

    // FIXME: This test is flaky if the loading of the icon image is asynchronous,
    // because the iconChanged signal is emitted before the image has been downloaded.
    // We can set this property to true after we have some kind of favicon downloading
    // logic in the WebEngine.

    Image {
        id: favicon
        asynchronous: false
        source: webEngineView.icon
    }

    TestCase {
        id: test
        name: "WebEngineViewLoadFavIcon"
        when: windowShown

        function init() {
            if (webEngineView.icon != '') {
                // If this is not the first test, then load a blank page without favicon, restoring the initial state.
                webEngineView.url = 'about:blank'
                verify(webEngineView.waitForLoadSucceeded())
                spy.wait()
            }
            spy.clear()
        }

        function test_favIconLoad() {
            compare(spy.count, 0)
            var url = Qt.resolvedUrl("favicon.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())
            spy.wait()
            compare(spy.count, 1)
            compare(favicon.width, 48)
            compare(favicon.height, 48)
        }

        function test_favIconLoadEncodedUrl() {
            compare(spy.count, 0)
            var url = Qt.resolvedUrl("favicon2.html?favicon=load should work with#whitespace!")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())
            spy.wait()
            compare(spy.count, 1)
            compare(favicon.width, 16)
            compare(favicon.height, 16)
        }
    }
}
