// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine
import Test.util
import "../../qmltests/data"
import "../mock-delegates/TestParams"


TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300
    property var logs: []
    property bool accessRequested: false
    property url file: tempDir.pathUrl('file.txt')

    onJavaScriptConsoleMessage: function(level, message, lineNumber, source) {
        var pair = message.split(':');
        if (pair.length == 2 && pair[0] == "TEST")
            logs.push(pair[1]);
    }

    TempDir { id: tempDir }

    TestCase {
        id: testCase
        name: "FileSystemAPI"
        when: windowShown

        function init() {
            clearLog()
            FilePickerParams.filePickerOpened = false
            FilePickerParams.selectFiles = false
            FilePickerParams.selectedFilesUrl = []
            FilePickerParams.nameFilters = []
            accessRequested = false;
        }

        function cleanup() {
            clearLog()
        }

        function clearLog() {
            logs = []
        }

        function logContainsDoneMarker() {
            if (logs.indexOf("DONE") > -1)
                return true
            else
                return false
        }

        function result() {
            return logs[0]
        }

        function fileAccessRequest(request) {
            testCase.verify(!accessRequested)
            accessRequested = true
            testCase.verify(request.filePath == file)
            testCase.verify(request.accessFlags == WebEngineFileSystemAccessRequest.Write | WebEngineFileSystemAccessRequest.Read)
            request.accept()
        }

        function directoryAccessRequest(request) {
            testCase.verify(!accessRequested)
            accessRequested = true
            testCase.verify(request.filePath == tempDir.pathUrl())
            testCase.verify(request.accessFlags == WebEngineFileSystemAccessRequest.Read)
            request.accept()
        }

        function test_saveFile() {
            webEngineView.fileSystemAccessRequested.connect(fileAccessRequest);
            webEngineView.url = Qt.resolvedUrl("filesystemapi.html?dialog=savePicker");
            verify(webEngineView.waitForLoadSucceeded());
            FilePickerParams.selectFiles = true;
            FilePickerParams.selectedFilesUrl.push(file);
            keyClick(Qt.Key_Enter); // Open SaveDialog.
            tryCompare(FilePickerParams, "filePickerOpened", true);
            tryVerify(logContainsDoneMarker,2000)
            // write access for save dialogs is automatically granted
            verify(!accessRequested)
            webEngineView.fileSystemAccessRequested.disconnect(fileAccessRequest);
        }

        function test_openFile() {
            // first save the file before open
            test_saveFile()
            init()
            webEngineView.fileSystemAccessRequested.connect(fileAccessRequest);
            webEngineView.url = Qt.resolvedUrl("filesystemapi.html?dialog=filePicker");
            verify(webEngineView.waitForLoadSucceeded());
            FilePickerParams.selectFiles = true;
            FilePickerParams.selectedFilesUrl.push(file);
            keyClick(Qt.Key_Enter); // Open FileDialog.
            tryCompare(FilePickerParams, "filePickerOpened", true);
            tryVerify(logContainsDoneMarker,2000)
            verify(logs.indexOf("TEST_CONTENT") > -1)
            verify(accessRequested)
            webEngineView.fileSystemAccessRequested.disconnect(fileAccessRequest);
        }

        function test_selectDirectory() {
            tempDir.createDirectory("TEST_DIR")
            webEngineView.fileSystemAccessRequested.connect(directoryAccessRequest);
            webEngineView.url = Qt.resolvedUrl("filesystemapi.html?dialog=directoryPicker");
            verify(webEngineView.waitForLoadSucceeded())
            FilePickerParams.selectFiles = true;
            FilePickerParams.selectedFilesUrl.push(tempDir.pathUrl());
            keyClick(Qt.Key_Enter); // Open showDirectoryDialog.
            tryCompare(FilePickerParams, "directoryPickerOpened", true);
            tryVerify(logContainsDoneMarker,2000)
            verify(logs.indexOf("TEST_DIR") > -1)
            verify(accessRequested)
            webEngineView.fileSystemAccessRequested.disconnect(directoryAccessRequest);
        }

    }
}
