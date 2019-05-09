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
    SignalSpy {
        id: terminationSpy
        target: webEngineView
        signalName: "renderProcessTerminated"
    }

    TestCase {
        name: "WebEngineViewSingleFileUpload"
        when: windowShown

        function init() {
            FilePickerParams.filePickerOpened = false
            FilePickerParams.selectFiles = false
            FilePickerParams.selectedFilesUrl = []
            titleSpy.clear()
            terminationSpy.clear()
        }

        function cleanup() {
            // Test that the render process doesn't crash, and make sure if it does it does so now.
            wait(1000)
            verify(terminationSpy.count == 0, "Render process didn't self terminate")

            // FIXME: Almost every second url loading progress does get stuck at about 90 percent, so the loadFinished signal won't arrive.
            // This cleanup function is a workaround for this problem.
            webEngineView.url = Qt.resolvedUrl("about:blank")
            webEngineView.waitForLoadSucceeded()
        }

        function test_acceptSingleFileSelection_data() {
            return [
                   { tag: "/test.txt)", input: "/test.txt", passDefaultDialog: false, passCustomDialog: true },
                   { tag: "/tést.txt", input: "/tést.txt", passDefaultDialog: false, passCustomDialog: true },
                   { tag: "file:///test.txt", input: "file:///test.txt", passDefaultDialog: true, passCustomDialog: false },
                   { tag: "file:///tést.txt", input: "file:///tést.txt", passDefaultDialog: true, passCustomDialog: false  },
                   { tag: "file:///t%C3%A9st.txt", input: "file:///t%C3%A9st.txt", passDefaultDialog: true, passCustomDialog: false },
                   { tag: "file://test.txt", input: "file://test.txt", passDefaultDialog: false, passCustomDialog: false },
                   { tag: "file:/test.txt", input: "file:/test.txt", passDefaultDialog: true, passCustomDialog: false },
                   { tag: "file:test//test.txt", input: "file:test//test.txt", passDefaultDialog: false, passCustomDialog: false },
                   { tag: "http://test.txt", input: "http://test.txt", passDefaultDialog: false, passCustomDialog: false },
                   { tag: "qrc:/test.txt", input: "qrc:/test.txt", passDefaultDialog: false, passCustomDialog: false },
            ];
        }

        function test_acceptSingleFileSelection(row) {
            var expectedFileName;

            // Default dialog (expects URL).
            expectedFileName = "Failed to Upload";
            if (row.passDefaultDialog)
                expectedFileName = row.input.slice(row.input.lastIndexOf('/') + 1);

            webEngineView.url = Qt.resolvedUrl("singlefileupload.html");
            verify(webEngineView.waitForLoadSucceeded());

            FilePickerParams.selectFiles = true;
            FilePickerParams.selectedFilesUrl.push(row.input);

            keyClick(Qt.Key_Enter); // Focus is on the button. Open FileDialog.
            tryCompare(FilePickerParams, "filePickerOpened", true);
            tryCompare(webEngineView, "title", decodeURIComponent(expectedFileName));


            // Custom dialog (expects absolute path).
            var finished = false;

            expectedFileName = "Failed to Upload";
            if (row.passCustomDialog)
                expectedFileName = row.input.slice(row.input.lastIndexOf('/') + 1);

            function acceptedFileHandler(request) {
                request.accepted = true;
                request.dialogAccept(row.input);
                finished = true;
            }

            webEngineView.fileDialogRequested.connect(acceptedFileHandler);
            webEngineView.url = Qt.resolvedUrl("singlefileupload.html");
            verify(webEngineView.waitForLoadSucceeded());

            keyClick(Qt.Key_Enter); // Focus is on the button. Open FileDialog.
            tryVerify(function() { return finished; });
            tryCompare(webEngineView, "title", expectedFileName);
            webEngineView.fileDialogRequested.disconnect(acceptedFileHandler);
        }

        function test_acceptMultipleFilesSelection() {
            webEngineView.url = Qt.resolvedUrl("multifileupload.html")
            verify(webEngineView.waitForLoadSucceeded())

            FilePickerParams.selectFiles = true
            FilePickerParams.selectedFilesUrl.push(Qt.resolvedUrl("test1.html"))
            FilePickerParams.selectedFilesUrl.push(Qt.resolvedUrl("test2.html"))

            keyPress(Qt.Key_Enter) // Focus is on the button. Open FileDialog.
            tryCompare(FilePickerParams, "filePickerOpened", true)
            tryCompare(webEngineView, "title", "test1.html,test2.html")
        }

        function test_acceptDirectory() {
            webEngineView.url = Qt.resolvedUrl("directoryupload.html")
            verify(webEngineView.waitForLoadSucceeded())

            FilePickerParams.selectFiles = true
            FilePickerParams.selectedFilesUrl.push(Qt.resolvedUrl("../data"))

            keyClick(Qt.Key_Enter) // Focus is on the button. Open FileDialog.
            tryCompare(FilePickerParams, "filePickerOpened", true)
            // Check that the title is a file list (eg. "test1.html,test2.html")
            tryVerify(function() { return webEngineView.title.match("^([^,]+,)+[^,]+$"); })
        }

        function test_reject() {
            webEngineView.url = Qt.resolvedUrl("singlefileupload.html")
            verify(webEngineView.waitForLoadSucceeded())

            titleSpy.clear()
            keyPress(Qt.Key_Enter) // Focus is on the button. Open FileDialog.
            wait(100)
            compare(titleSpy.count, 0)
        }

        function test_acceptMultipleFilesWithCustomDialog_data() {
            return [
                   { tag: "path", input: ["/test1.txt", "/test2.txt"], expectedValueForDefaultDialog: "Failed to Upload", expectedValueForCustomDialog: "test1.txt,test2.txt" },
                   { tag: "file", input: ["file:///test1.txt", "file:///test2.txt"], expectedValueForDefaultDialog: "test1.txt,test2.txt", expectedValueForCustomDialog: "Failed to Upload" },
                   { tag: "mixed", input: ["file:///test1.txt", "/test2.txt"], expectedValueForDefaultDialog: "test1.txt", expectedValueForCustomDialog: "test2.txt" },
            ];
        }

        function test_acceptMultipleFilesWithCustomDialog(row) {
            // Default dialog (expects URL).
            webEngineView.url = Qt.resolvedUrl("multifileupload.html");
            verify(webEngineView.waitForLoadSucceeded());

            FilePickerParams.selectFiles = true;
            FilePickerParams.selectedFilesUrl = row.input;

            keyClick(Qt.Key_Enter); // Focus is on the button. Open FileDialog.
            tryCompare(FilePickerParams, "filePickerOpened", true);
            tryCompare(webEngineView, "title", row.expectedValueForDefaultDialog);


            // Custom dialog (expects absolute path).
            var finished = false;

            function acceptedFileHandler(request) {
                request.accepted = true;
                request.dialogAccept(row.input);
                finished = true;
            }

            webEngineView.fileDialogRequested.connect(acceptedFileHandler);
            webEngineView.url = Qt.resolvedUrl("multifileupload.html");
            verify(webEngineView.waitForLoadSucceeded());

            keyClick(Qt.Key_Enter); // Focus is on the button. Open FileDialog.
            tryVerify(function() { return finished; });
            tryCompare(webEngineView, "title", row.expectedValueForCustomDialog);
            webEngineView.fileDialogRequested.disconnect(acceptedFileHandler);
        }
    }
}
