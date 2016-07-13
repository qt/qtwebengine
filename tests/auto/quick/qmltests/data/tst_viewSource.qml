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
import QtWebEngine 1.4

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400

    property var viewRequest: null

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
        id: test
        name: "WebEngineViewSource"

        function init() {
            newViewRequestedSpy.clear();
            titleChangedSpy.clear();
            viewRequest = null;
        }

        function test_viewSource() {
            webEngineView.url = Qt.resolvedUrl("test1.html");
            verify(webEngineView.waitForLoadSucceeded());
            compare(webEngineView.title, "Test page 1");
            verify(webEngineView.canViewSource, true);

            titleChangedSpy.clear();
            webEngineView.viewSource();
            tryCompare(newViewRequestedSpy, "count", 1);
            verify(webEngineView.waitForLoadSucceeded());
            // The first titleChanged signal is emitted by adoptWebContents()
            tryCompare(titleChangedSpy, "count", 2);

            compare(viewRequest.destination, WebEngineView.NewViewInTab);
            verify(viewRequest.userInitiated);
            verify(!webEngineView.canViewSource);

            compare(webEngineView.title, "test1.html");
            compare(webEngineView.url, "view-source:" + Qt.resolvedUrl("test1.html"));
        }
    }
}

