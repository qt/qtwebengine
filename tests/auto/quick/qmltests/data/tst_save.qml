import QtQuick
import QtTest
import QtWebEngine
import Test.util

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 200
    profile: testSaveProfile

    property url downloadUrl: ""
    property int totalBytes: 0
    property int receivedBytes: 0
    property string downloadDir: ""
    property string downloadFileName: ""
    property bool isSavePageDownload: false
    property var downloadState: []
    property int savePageFormat: WebEngineDownloadRequest.MimeHtmlSaveFormat;
    property bool autoCancel: false

    TempDir {
        id: tempDir
    }

    SignalSpy {
        id: downLoadRequestedSpy
        target: testSaveProfile
        signalName: "downloadRequested"
    }

    SignalSpy {
        id: downloadFinishedSpy
        target: testSaveProfile
        signalName: "downloadFinished"
    }

    WebEngineProfile {
        id: testSaveProfile

        onDownloadRequested: function(download) {
            downloadState.push(download.state)
            downloadUrl = download.url
            savePageFormat = download.savePageFormat
            downloadDir = download.downloadDirectory;
            downloadFileName = download.downloadFileName
            isSavePageDownload = download.isSavePageDownload

            if (autoCancel)
                download.cancel()
        }
        onDownloadFinished: function(download) {
            receivedBytes = download.receivedBytes
            totalBytes = download.totalBytes
            downloadState.push(download.state)
        }
    }

    TestCase {
        name: "WebEngineViewSave"

        function verifyData() {
            var isDataValid = false
            webEngineView.runJavaScript("(function() {" +
                                        "var title = document.title.toString();" +
                                        "var body = document.body.innerText;" +
                                        " return title === \"Test page 1\" && body.includes(\"Hello.\")" +
                                        "})();", function(result) {
                isDataValid = result;
            });
            tryVerify(function() { return isDataValid });
            return isDataValid;
        }

        function init() {
            downLoadRequestedSpy.clear()
            downloadFinishedSpy.clear()
            totalBytes = 0
            receivedBytes = 0
            downloadDir = ""
            downloadFileName = ""
            isSavePageDownload = false
            downloadState = []
            downloadUrl = ""
            autoCancel = false
        }

        function test_savePage_data() {
            return [
                   { tag: "SingleHtmlSaveFormat", savePageFormat: WebEngineDownloadRequest.SingleHtmlSaveFormat },
                   { tag: "CompleteHtmlSaveFormat", savePageFormat: WebEngineDownloadRequest.CompleteHtmlSaveFormat },
                   { tag: "MimeHtmlSaveFormat", savePageFormat: WebEngineDownloadRequest.MimeHtmlSaveFormat },
            ];
        }

        function test_savePage(row) {
            var saveFormat = row.savePageFormat

            var fileDir = tempDir.path()
            var fileName = "saved_page.html"
            var filePath = fileDir + "/"+ fileName

            // load data to view
            webEngineView.url = Qt.resolvedUrl("test1.html")
            verify(webEngineView.waitForLoadSucceeded())
            verify(verifyData())

            webEngineView.save(filePath, saveFormat)
            downLoadRequestedSpy.wait()
            compare(downLoadRequestedSpy.count, 1)
            compare(downloadUrl, webEngineView.url)
            compare(savePageFormat, saveFormat)
            compare(downloadDir, fileDir)
            compare(downloadFileName, fileName)
            compare(isSavePageDownload, true)
            compare(downloadState[0], WebEngineDownloadRequest.DownloadInProgress)
            downloadFinishedSpy.wait()
            compare(downloadFinishedSpy.count, 1)
            compare(totalBytes, receivedBytes)
            compare(downloadState[1], WebEngineDownloadRequest.DownloadCompleted)

            // load some other data
            webEngineView.url = Qt.resolvedUrl("about:blank")
            verify(webEngineView.waitForLoadSucceeded())

            // load save file to view
            webEngineView.url = Qt.resolvedUrl(filePath)
            verify(webEngineView.waitForLoadSucceeded())
            verify(verifyData())
        }

        function test_saveImage_data() {
            return [
                   { tag: "Auto accept", autoCancel: false },
                   { tag: "Cancel", autoCancel: true },
            ];
        }

        function test_saveImage(row) {
            autoCancel = row.autoCancel

            var fileDir = tempDir.path()
            var fileName = "favicon.png"
            var filePath = fileDir + "/"+ fileName

            // Load an image
            webEngineView.url = Qt.resolvedUrl("icons/favicon.png")
            verify(webEngineView.waitForLoadSucceeded())

            webEngineView.save(filePath)
            downLoadRequestedSpy.wait()
            compare(downLoadRequestedSpy.count, 1)
            compare(downloadUrl, webEngineView.url)
            compare(downloadDir, fileDir)
            compare(downloadFileName, fileName)
            compare(isSavePageDownload, true)
            compare(downloadState[0], WebEngineDownloadRequest.DownloadInProgress)
            downloadFinishedSpy.wait()
            compare(downloadFinishedSpy.count, 1)
            if (autoCancel) {
                compare(receivedBytes, 0)
                compare(downloadState[1], WebEngineDownloadRequest.DownloadCancelled)
            } else {
                compare(totalBytes, receivedBytes)
                compare(downloadState[1], WebEngineDownloadRequest.DownloadCompleted)
            }
        }

        function test_saveWebAction() {
            // Load an image
            webEngineView.url = Qt.resolvedUrl("icons/favicon.png")
            verify(webEngineView.waitForLoadSucceeded())

            // Saving without specifying path shouldn't be auto accepted
            webEngineView.triggerWebAction(WebEngineView.SavePage)
            downLoadRequestedSpy.wait()
            compare(downLoadRequestedSpy.count, 1)
            compare(downloadUrl, webEngineView.url)
            compare(isSavePageDownload, true)
            // The initial download request starts from DownloadRequested state,
            // which means it wasn't automatically accepted.
            compare(downloadState[0], WebEngineDownloadRequest.DownloadRequested)
        }
    }
}
