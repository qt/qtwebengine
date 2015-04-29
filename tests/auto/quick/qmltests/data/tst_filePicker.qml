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
import QtWebEngine 1.2
import "../mock-delegates/TestParams" 1.0

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
        name: "WebEngineViewSingleFileUpload"
        when: windowShown

        function init() {
            FilePickerParams.filePickerOpened = false
            FilePickerParams.selectFiles = false
            FilePickerParams.selectedFilesUrl = []
            titleSpy.clear()
        }

        // FIXME: Almost every second url loading progress does get stuck at about 90 percent, so the loadFinished signal won't arrive.
        // This cleanup function is a workaround for this problem.
        function cleanup() {
            webEngineView.url = Qt.resolvedUrl("about:blank")
            webEngineView.waitForLoadSucceeded()
        }

        function test_acceptSingleFileSelection() {
            webEngineView.url = Qt.resolvedUrl("singlefileupload.html")
            verify(webEngineView.waitForLoadSucceeded())

            FilePickerParams.selectFiles = true
            FilePickerParams.selectedFilesUrl.push(Qt.resolvedUrl("test1.html"))

            keyPress(Qt.Key_Enter) // Focus is on the button. Open FileDialog.
            wait(100) // The ui delegate is invoked asynchronously
            verify(FilePickerParams.filePickerOpened)
            titleSpy.wait()
            compare(webEngineView.title, "test1.html")
        }

        function test_acceptMultipleFilesSelection() {
            webEngineView.url = Qt.resolvedUrl("multifileupload.html")
            verify(webEngineView.waitForLoadSucceeded())

            FilePickerParams.selectFiles = true
            FilePickerParams.selectedFilesUrl.push(Qt.resolvedUrl("test1.html"))
            FilePickerParams.selectedFilesUrl.push(Qt.resolvedUrl("test2.html"))

            keyPress(Qt.Key_Enter) // Focus is on the button. Open FileDialog.
            wait(100)
            verify(FilePickerParams.filePickerOpened)
            titleSpy.wait()
            compare(webEngineView.title, "test1.html,test2.html")
        }

        function test_acceptDirectory() {
            webEngineView.url = Qt.resolvedUrl("directoryupload.html")
            verify(webEngineView.waitForLoadSucceeded())

            FilePickerParams.selectFiles = true
            FilePickerParams.selectedFilesUrl.push(Qt.resolvedUrl("../data"))

            keyPress(Qt.Key_Enter) // Focus is on the button. Open FileDialog.
            wait(100) // The ui delegate is invoked asynchronously
            verify(FilePickerParams.filePickerOpened)
            titleSpy.wait()
            compare(webEngineView.title, "data")
        }

        function test_reject() {
            webEngineView.url = Qt.resolvedUrl("singlefileupload.html")
            verify(webEngineView.waitForLoadSucceeded())

            titleSpy.clear()
            keyPress(Qt.Key_Enter) // Focus is on the button. Open FileDialog.
            wait(100)
            compare(titleSpy.count, 0)
        }
    }
}
