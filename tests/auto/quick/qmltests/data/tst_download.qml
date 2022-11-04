// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine
import Qt.labs.platform
import Test.util

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 200
    profile: testDownloadProfile

    property int totalBytes: 0
    property int receivedBytes: 0
    property bool cancelDownload: false
    property var downloadState: []
    property var downloadInterruptReason: null
    property url downloadUrl: ""
    property string suggestedFileName: ""
    property string downloadDirectory: ""
    property string downloadFileName: ""
    property string downloadedPath: ""
    property string downloadedSetPath: ""
    property int downloadDirectoryChanged: 0
    property int downloadFileNameChanged: 0
    property bool setDirectoryFirst: false

    TempDir { id: tempDir }

    function urlToPath(url) {
        var path = url.toString()
        if (Qt.platform.os !== "windows")
            path = path.replace(/^(file:\/{2})/, "")
        else
            path = path.replace(/^(file:\/{3})/, "")
        return path
    }

    SignalSpy {
        id: downLoadRequestedSpy
        target: testDownloadProfile
        signalName: "downloadRequested"
    }

    SignalSpy {
        id: downloadFinishedSpy
        target: testDownloadProfile
        signalName: "downloadFinished"
    }

    Connections {
        id: downloadItemConnections
        ignoreUnknownSignals: true
        function onStateChanged() {
            downloadState.push(target.state);
        }
        function onInterruptReasonChanged() {
            downloadInterruptReason = target.interruptReason;
        }
        function onDownloadDirectoryChanged() {
            downloadDirectoryChanged++;
        }
        function onDownloadFileNameChanged() {
            downloadFileNameChanged++;
        }
    }

    WebEngineProfile {
        id: testDownloadProfile

        onDownloadRequested: function(download) {
            testDownloadProfile.downloadPath = tempDir.path()
            downloadState.push(download.state)
            downloadItemConnections.target = download
            if (cancelDownload) {
                download.cancel()
            } else {
                totalBytes = download.totalBytes

                if (downloadedSetPath.length != 0) {
                    download.path = testDownloadProfile.downloadPath + downloadedSetPath
                    downloadedPath = download.path
                } else {
                    if (setDirectoryFirst && downloadDirectory.length != 0)
                        download.downloadDirectory = testDownloadProfile.downloadPath + downloadDirectory

                    if (downloadFileName.length != 0)
                        download.downloadFileName = downloadFileName

                    if (!setDirectoryFirst && downloadDirectory.length != 0)
                        download.downloadDirectory = testDownloadProfile.downloadPath + downloadDirectory

                    downloadedPath = download.downloadDirectory + download.downloadFileName
                }

                download.accept()
            }
            downloadUrl = download.url
            suggestedFileName = download.suggestedFileName
        }
        onDownloadFinished: function(download) {
            receivedBytes = download.receivedBytes;
        }
    }

    TestCase {
        name: "WebEngineViewDownload"

        function init() {
            downLoadRequestedSpy.clear()
            downloadFinishedSpy.clear()
            totalBytes = 0
            receivedBytes = 0
            cancelDownload = false
            downloadItemConnections.target = null
            downloadState = []
            downloadInterruptReason = null
            downloadDirectoryChanged = 0
            downloadFileNameChanged = 0
            downloadDirectory = ""
            downloadFileName = ""
            downloadedPath = ""
            downloadedSetPath = ""
            setDirectoryFirst = false
        }

        function test_downloadRequest() {
            compare(downLoadRequestedSpy.count, 0)
            downloadDirectory = "/test_downloadRequest/";
            webEngineView.url = Qt.resolvedUrl("download.zip")
            downLoadRequestedSpy.wait()
            compare(downLoadRequestedSpy.count, 1)
            compare(downloadUrl, webEngineView.url)
            compare(suggestedFileName, "download.zip")
            compare(downloadState[0], WebEngineDownloadRequest.DownloadRequested)
            verify(!downloadInterruptReason)
        }

        function test_totalFileLength() {
            compare(downLoadRequestedSpy.count, 0)
            downloadDirectory = "/test_totalFileLength/";
            webEngineView.url = Qt.resolvedUrl("download.zip")
            downLoadRequestedSpy.wait()
            compare(downLoadRequestedSpy.count, 1)
            compare(downloadUrl, webEngineView.url)
            compare(suggestedFileName, "download.zip")
            compare(totalBytes, 325)
            verify(!downloadInterruptReason)
        }

        function test_downloadSucceeded() {
            compare(downLoadRequestedSpy.count, 0)
            downloadDirectory = "/test_downloadSucceeded/";
            webEngineView.url = Qt.resolvedUrl("download.zip")
            downLoadRequestedSpy.wait()
            compare(downLoadRequestedSpy.count, 1)
            compare(downloadUrl, webEngineView.url)
            compare(suggestedFileName, "download.zip")
            compare(downloadState[0], WebEngineDownloadRequest.DownloadRequested)
            tryCompare(downloadState, "1", WebEngineDownloadRequest.DownloadInProgress)
            downloadFinishedSpy.wait()
            compare(totalBytes, receivedBytes)
            tryCompare(downloadState, "2", WebEngineDownloadRequest.DownloadCompleted)
            verify(!downloadInterruptReason)
        }

        function test_downloadCancelled() {
            compare(downLoadRequestedSpy.count, 0)
            cancelDownload = true
            webEngineView.url = Qt.resolvedUrl("download.zip")
            downLoadRequestedSpy.wait()
            compare(downLoadRequestedSpy.count, 1)
            compare(downloadUrl, webEngineView.url)
            compare(suggestedFileName, "download.zip")
            compare(downloadFinishedSpy.count, 1)
            tryCompare(downloadState, "1", WebEngineDownloadRequest.DownloadCancelled)
            tryCompare(webEngineView, "downloadInterruptReason", WebEngineDownloadRequest.UserCanceled)
        }

        function test_downloadLocation() {
            var tmpPath = urlToPath(StandardPaths.writableLocation(StandardPaths.TempLocation));
            var downloadPath = urlToPath(StandardPaths.writableLocation(StandardPaths.DownloadLocation));

            testDownloadProfile.downloadPath = tmpPath;
            compare(testDownloadProfile.downloadPath, tmpPath);

            testDownloadProfile.downloadPath = downloadPath;
            compare(testDownloadProfile.downloadPath, downloadPath);
        }

        function test_downloadToDirectoryWithFileName_data() {
            return [
                   { tag: "setDirectoryFirst", setDirectoryFirst: true },
                   { tag: "setFileNameFirst", setDirectoryFirst: false },
            ];
        }

        function test_downloadToDirectoryWithFileName(row) {
            compare(downLoadRequestedSpy.count, 0);
            compare(downloadDirectoryChanged, 0);
            compare(downloadFileNameChanged, 0);
            setDirectoryFirst = row.setDirectoryFirst;
            downloadDirectory = "/test_downloadToDirectoryWithFileName/";
            downloadFileName = "test.zip";
            webEngineView.url = Qt.resolvedUrl("download.zip");
            downLoadRequestedSpy.wait();
            compare(downLoadRequestedSpy.count, 1);
            compare(downloadUrl, webEngineView.url);
            compare(suggestedFileName, "download.zip");
            compare(downloadState[0], WebEngineDownloadRequest.DownloadRequested);
            tryCompare(downloadState, "1", WebEngineDownloadRequest.DownloadInProgress);
            compare(downloadedPath, testDownloadProfile.downloadPath + downloadDirectory + downloadFileName);
            compare(downloadDirectoryChanged, 1);
            compare(downloadFileNameChanged, 1);
            downloadFinishedSpy.wait();
            compare(totalBytes, receivedBytes);
            tryCompare(downloadState, "2", WebEngineDownloadRequest.DownloadCompleted);
            verify(!downloadInterruptReason);
        }

        function test_downloadToDirectoryWithSuggestedFileName() {
            // Download file to a custom download directory with suggested file name.
            compare(downLoadRequestedSpy.count, 0);
            compare(downloadDirectoryChanged, 0);
            compare(downloadFileNameChanged, 0);
            downloadDirectory = "/test_downloadToDirectoryWithSuggestedFileName/";
            webEngineView.url = Qt.resolvedUrl("download.zip");
            downLoadRequestedSpy.wait();
            compare(downLoadRequestedSpy.count, 1);
            compare(downloadUrl, webEngineView.url);
            compare(suggestedFileName, "download.zip");
            compare(downloadState[0], WebEngineDownloadRequest.DownloadRequested);
            tryCompare(downloadState, "1", WebEngineDownloadRequest.DownloadInProgress);
            compare(downloadedPath, testDownloadProfile.downloadPath + downloadDirectory + "download.zip");
            compare(downloadDirectoryChanged, 1);
            compare(downloadFileNameChanged, 0);
            downloadFinishedSpy.wait();
            compare(totalBytes, receivedBytes);
            tryCompare(downloadState, "2", WebEngineDownloadRequest.DownloadCompleted);
            verify(!downloadInterruptReason);

            // Download the same file to another directory with suggested file name.
            // The downloadFileNameChanged signal should not be emitted.
            downLoadRequestedSpy.clear();
            compare(downLoadRequestedSpy.count, 0);
            downloadDirectoryChanged = 0;
            downloadFileNameChanged = 0;
            downloadDirectory = "/test_downloadToDirectoryWithSuggestedFileName1/";
            webEngineView.url = Qt.resolvedUrl("download.zip");
            downLoadRequestedSpy.wait();
            compare(downLoadRequestedSpy.count, 1);
            compare(downloadUrl, webEngineView.url);
            compare(suggestedFileName, "download.zip");
            compare(downloadState[0], WebEngineDownloadRequest.DownloadRequested);
            tryCompare(downloadState, "1", WebEngineDownloadRequest.DownloadInProgress);
            compare(downloadedPath, testDownloadProfile.downloadPath + downloadDirectory + "download.zip");
            compare(downloadDirectoryChanged, 1);
            compare(downloadFileNameChanged, 0);
            downloadFinishedSpy.wait();
            compare(totalBytes, receivedBytes);
            tryCompare(downloadState, "2", WebEngineDownloadRequest.DownloadCompleted);
            verify(!downloadInterruptReason);

            // Download same file to same directory second time -> file name should be unified.
            // The downloadFileNameChanged signal should be emitted.
            downLoadRequestedSpy.clear();
            compare(downLoadRequestedSpy.count, 0);
            downloadDirectoryChanged = 0;
            downloadFileNameChanged = 0;
            downloadDirectory = "/test_downloadToDirectoryWithSuggestedFileName1/";
            webEngineView.url = Qt.resolvedUrl("download.zip");
            downLoadRequestedSpy.wait();
            compare(downLoadRequestedSpy.count, 1);
            compare(downloadUrl, webEngineView.url);
            compare(suggestedFileName, "download.zip");
            compare(downloadState[0], WebEngineDownloadRequest.DownloadRequested);
            tryCompare(downloadState, "1", WebEngineDownloadRequest.DownloadInProgress);
            compare(downloadedPath, testDownloadProfile.downloadPath + downloadDirectory + "download (1).zip");
            compare(downloadDirectoryChanged, 1);
            compare(downloadFileNameChanged, 1);
            downloadFinishedSpy.wait();
            compare(totalBytes, receivedBytes);
            tryCompare(downloadState, "2", WebEngineDownloadRequest.DownloadCompleted);
            verify(!downloadInterruptReason);
        }
    }
}
