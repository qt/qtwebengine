// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtWebEngine
import QtWebEngine.TestMockDelegates
import Test.util
import "../../qmltests/data"


TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300
    property bool accessRequested: false
    property url file: tempDir.pathUrl('file.txt')

    JsConsoleMessageSpy { id: jsConsoleMessageSpy; target: webEngineView }

    TempDir { id: tempDir }

    TestCase {
        id: testCase
        name: "FileSystemAPI"
        when: windowShown

        function init() {
            FilePickerParams.filePickerOpened = false
            FilePickerParams.selectFiles = false
            FilePickerParams.selectedFilesUrl = []
            FilePickerParams.nameFilters = []
            accessRequested = false;
        }

        function cleanup() {
            jsConsoleMessageSpy.clear()
        }

        function messageReceived(expectedMessage) {
            jsConsoleMessageSpy.wait()
            return (jsConsoleMessageSpy.popMessage() == expectedMessage)
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
            verify(jsConsoleMessageSpy.waitForMessage("TEST:DONE"));
            // write access for save dialogs is automatically granted
            verify(!accessRequested)
            webEngineView.fileSystemAccessRequested.disconnect(fileAccessRequest);
        }

        function test_openFile() {
            // first save the file before open
            test_saveFile()
            jsConsoleMessageSpy.clear();
            init()
            webEngineView.fileSystemAccessRequested.connect(fileAccessRequest);
            webEngineView.url = Qt.resolvedUrl("filesystemapi.html?dialog=filePicker");
            verify(webEngineView.waitForLoadSucceeded());
            FilePickerParams.selectFiles = true;
            FilePickerParams.selectedFilesUrl.push(file);
            keyClick(Qt.Key_Enter); // Open FileDialog.
            tryCompare(FilePickerParams, "filePickerOpened", true);
            verify(jsConsoleMessageSpy.waitForMessage("TEST:DONE"));
            verify(jsConsoleMessageSpy.waitForMessage("TEST:TEST_CONTENT"));
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
            verify(jsConsoleMessageSpy.waitForMessage("start"));
            verify(jsConsoleMessageSpy.waitForMessage("TEST:DONE"));
            verify(jsConsoleMessageSpy.waitForMessage("TEST:TEST_DIR"));
            verify(accessRequested)
            webEngineView.fileSystemAccessRequested.disconnect(directoryAccessRequest);
        }

    }
}
