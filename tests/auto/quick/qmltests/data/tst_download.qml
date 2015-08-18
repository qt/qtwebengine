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
    width: 200
    height: 200
    profile: testDownloadProfile

    property int totalBytes: 0
    property int receivedBytes: 0
    property bool cancelDownload: false
    property var downloadState: []

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

    WebEngineProfile {
        id: testDownloadProfile

        onDownloadRequested: {
            downloadState.push(download.state)
            if (cancelDownload) {
                download.cancel()
                downloadState.push(download.state)
            } else {
                totalBytes = download.totalBytes
                download.path = "testfile.zip"
                download.accept()
                downloadState.push(download.state)
            }
        }
        onDownloadFinished: {
            downloadState.push(download.state)
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
            downloadState = []
        }

        function test_downloadRequest() {
            compare(downLoadRequestedSpy.count, 0)
            webEngineView.url = Qt.resolvedUrl("download.zip")
            downLoadRequestedSpy.wait()
            compare(downLoadRequestedSpy.count, 1)
            compare(downloadState[0], WebEngineDownloadItem.DownloadRequested)
        }

        function test_totalFileLength() {
            compare(downLoadRequestedSpy.count, 0)
            webEngineView.url = Qt.resolvedUrl("download.zip")
            downLoadRequestedSpy.wait()
            compare(downLoadRequestedSpy.count, 1)
            compare(totalBytes, 325)
        }

        function test_downloadSucceeded() {
            compare(downLoadRequestedSpy.count, 0)
            webEngineView.url = Qt.resolvedUrl("download.zip")
            downLoadRequestedSpy.wait()
            compare(downLoadRequestedSpy.count, 1)
            compare(downloadState[1], WebEngineDownloadItem.DownloadInProgress)
            downloadFinishedSpy.wait()
            compare(totalBytes, receivedBytes)
            compare(downloadState[2], WebEngineDownloadItem.DownloadCompleted)
        }

        function test_downloadCancelled() {
            compare(downLoadRequestedSpy.count, 0)
            cancelDownload = true
            webEngineView.url = Qt.resolvedUrl("download.zip")
            downLoadRequestedSpy.wait()
            compare(downLoadRequestedSpy.count, 1)
            compare(downloadFinishedSpy.count, 1)
            compare(downloadState[2], WebEngineDownloadItem.DownloadCancelled)
        }
    }
}
