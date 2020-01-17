/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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
import QtWebEngine 1.10
import Qt.labs.platform 1.0
import Test.util 1.0

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
    property int downloadPathChanged: 0
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
        onStateChanged: downloadState.push(target.state)
        onInterruptReasonChanged: downloadInterruptReason = target.interruptReason
        onDownloadDirectoryChanged: downloadDirectoryChanged++
        onDownloadFileNameChanged: downloadFileNameChanged++
        onPathChanged: downloadPathChanged++
    }

    WebEngineProfile {
        id: testDownloadProfile

        onDownloadRequested: {
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
        onDownloadFinished: {
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
            downloadPathChanged = 0
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
            compare(downloadState[0], WebEngineDownloadItem.DownloadRequested)
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
            compare(downloadState[0], WebEngineDownloadItem.DownloadRequested)
            tryCompare(downloadState, "1", WebEngineDownloadItem.DownloadInProgress)
            downloadFinishedSpy.wait()
            compare(totalBytes, receivedBytes)
            tryCompare(downloadState, "2", WebEngineDownloadItem.DownloadCompleted)
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
            tryCompare(downloadState, "1", WebEngineDownloadItem.DownloadCancelled)
            tryCompare(webEngineView, "downloadInterruptReason", WebEngineDownloadItem.UserCanceled)
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
            compare(downloadState[0], WebEngineDownloadItem.DownloadRequested);
            tryCompare(downloadState, "1", WebEngineDownloadItem.DownloadInProgress);
            compare(downloadedPath, testDownloadProfile.downloadPath + downloadDirectory + downloadFileName);
            compare(downloadDirectoryChanged, 1);
            compare(downloadFileNameChanged, 1);
            compare(downloadPathChanged, 2);
            downloadFinishedSpy.wait();
            compare(totalBytes, receivedBytes);
            tryCompare(downloadState, "2", WebEngineDownloadItem.DownloadCompleted);
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
            compare(downloadState[0], WebEngineDownloadItem.DownloadRequested);
            tryCompare(downloadState, "1", WebEngineDownloadItem.DownloadInProgress);
            compare(downloadedPath, testDownloadProfile.downloadPath + downloadDirectory + "download.zip");
            compare(downloadDirectoryChanged, 1);
            compare(downloadFileNameChanged, 0);
            compare(downloadPathChanged, 1);
            downloadFinishedSpy.wait();
            compare(totalBytes, receivedBytes);
            tryCompare(downloadState, "2", WebEngineDownloadItem.DownloadCompleted);
            verify(!downloadInterruptReason);

            // Download the same file to another directory with suggested file name.
            // The downloadFileNameChanged signal should not be emitted.
            downLoadRequestedSpy.clear();
            compare(downLoadRequestedSpy.count, 0);
            downloadDirectoryChanged = 0;
            downloadFileNameChanged = 0;
            downloadPathChanged = 0;
            downloadDirectory = "/test_downloadToDirectoryWithSuggestedFileName1/";
            webEngineView.url = Qt.resolvedUrl("download.zip");
            downLoadRequestedSpy.wait();
            compare(downLoadRequestedSpy.count, 1);
            compare(downloadUrl, webEngineView.url);
            compare(suggestedFileName, "download.zip");
            compare(downloadState[0], WebEngineDownloadItem.DownloadRequested);
            tryCompare(downloadState, "1", WebEngineDownloadItem.DownloadInProgress);
            compare(downloadedPath, testDownloadProfile.downloadPath + downloadDirectory + "download.zip");
            compare(downloadDirectoryChanged, 1);
            compare(downloadFileNameChanged, 0);
            compare(downloadPathChanged, 1);
            downloadFinishedSpy.wait();
            compare(totalBytes, receivedBytes);
            tryCompare(downloadState, "2", WebEngineDownloadItem.DownloadCompleted);
            verify(!downloadInterruptReason);

            // Download same file to same directory second time -> file name should be unified.
            // The downloadFileNameChanged signal should be emitted.
            downLoadRequestedSpy.clear();
            compare(downLoadRequestedSpy.count, 0);
            downloadDirectoryChanged = 0;
            downloadFileNameChanged = 0;
            downloadPathChanged = 0;
            downloadDirectory = "/test_downloadToDirectoryWithSuggestedFileName1/";
            webEngineView.url = Qt.resolvedUrl("download.zip");
            downLoadRequestedSpy.wait();
            compare(downLoadRequestedSpy.count, 1);
            compare(downloadUrl, webEngineView.url);
            compare(suggestedFileName, "download.zip");
            compare(downloadState[0], WebEngineDownloadItem.DownloadRequested);
            tryCompare(downloadState, "1", WebEngineDownloadItem.DownloadInProgress);
            compare(downloadedPath, testDownloadProfile.downloadPath + downloadDirectory + "download (1).zip");
            compare(downloadDirectoryChanged, 1);
            compare(downloadFileNameChanged, 1);
            compare(downloadPathChanged, 1);
            downloadFinishedSpy.wait();
            compare(totalBytes, receivedBytes);
            tryCompare(downloadState, "2", WebEngineDownloadItem.DownloadCompleted);
            verify(!downloadInterruptReason);
}

        function test_downloadWithSetPath() {
            compare(downLoadRequestedSpy.count, 0);
            compare(downloadDirectoryChanged, 0);
            compare(downloadFileNameChanged, 0);
            downloadedSetPath = "/test_downloadWithSetPath/test.zip";
            webEngineView.url = Qt.resolvedUrl("download.zip");
            downLoadRequestedSpy.wait();
            compare(downLoadRequestedSpy.count, 1);
            compare(downloadUrl, webEngineView.url);
            compare(suggestedFileName, "download.zip");
            compare(downloadState[0], WebEngineDownloadItem.DownloadRequested);
            tryCompare(downloadState, "1", WebEngineDownloadItem.DownloadInProgress);
            compare(downloadedPath, testDownloadProfile.downloadPath + downloadedSetPath);
            compare(downloadDirectoryChanged, 1);
            compare(downloadFileNameChanged, 1);
            compare(downloadPathChanged, 2);
            downloadFinishedSpy.wait();
            compare(totalBytes, receivedBytes);
            tryCompare(downloadState, "2", WebEngineDownloadItem.DownloadCompleted);
            verify(!downloadInterruptReason);
        }
    }
}
