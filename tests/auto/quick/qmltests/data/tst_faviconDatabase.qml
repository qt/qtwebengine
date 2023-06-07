// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine
import Test.util
import "../../qmltests/data"

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400

    TempDir { id: tempDir }

    property QtObject defaultProfile: WebEngineProfile {
        offTheRecord: true
    }

    property QtObject nonOTRProfile: WebEngineProfile {
        persistentStoragePath: tempDir.path() + '/WebEngineFavicon'
        offTheRecord: false
    }

    function getFaviconPixel(faviconImage) {
        var grabImage = Qt.createQmlObject("
                import QtQuick\n
                Image { }", testCase)
        var faviconCanvas = Qt.createQmlObject("
                import QtQuick\n
                Canvas { }", testCase)

        testCase.tryVerify(function() { return faviconImage.status == Image.Ready });
        faviconImage.grabToImage(function(result) {
                grabImage.source = result.url
            });
        testCase.tryVerify(function() { return grabImage.status == Image.Ready });

        faviconCanvas.width = faviconImage.width;
        faviconCanvas.height = faviconImage.height;
        var ctx = faviconCanvas.getContext("2d");
        ctx.drawImage(grabImage, 0, 0, grabImage.width, grabImage.height);
        var imageData = ctx.getImageData(Math.round(faviconCanvas.width/2),
                                         Math.round(faviconCanvas.height/2),
                                         faviconCanvas.width,
                                         faviconCanvas.height);

        grabImage.destroy();
        faviconCanvas.destroy();

        return imageData.data;
    }

    SignalSpy {
        id: iconChangedSpy
        target: webEngineView
        signalName: "iconChanged"
    }

    TestCase {
        id: testCase
        name: "WebEngineFaviconDatabase"
        when: windowShown

        function init() {
            // It is worth to restore the initial state with loading a blank page before all test functions.
            webEngineView.url = 'about:blank';
            verify(webEngineView.waitForLoadSucceeded());
            iconChangedSpy.clear();
            webEngineView.settings.touchIconsEnabled = false;
            webEngineView.settings.autoLoadIconsForPage = true;
        }

        function cleanupTestCase() {
            tempDir.removeRecursive(nonOTRProfile.persistentStoragePath);
        }

        function test_iconDatabase_data() {
            return [
                   { tag: "OTR", profile: defaultProfile },
                   { tag: "non-OTR", profile: nonOTRProfile },
            ];
        }

        function test_iconDatabase(row)
        {
            if (Screen.devicePixelRatio !== 1.0)
                skip("This test is not supported on High DPI screens.");

            webEngineView.profile = row.profile;
            compare(iconChangedSpy.count, 0);

            var faviconImage = Qt.createQmlObject("
                    import QtQuick\n
                    Image { width: 16; height: 16; sourceSize: Qt.size(width, height); cache: false; }", testCase);

            var pixel;
            compare(iconChangedSpy.count, 0);

            webEngineView.url = Qt.resolvedUrl("favicon.html"); // favicon.png -> 165
            verify(webEngineView.waitForLoadSucceeded());

            iconChangedSpy.wait();
            compare(iconChangedSpy.count, 1);

            var previousIcon = webEngineView.icon;
            iconChangedSpy.clear();

            webEngineView.url = Qt.resolvedUrl("favicon-shortcut.html"); // qt32.ico -> 251
            verify(webEngineView.waitForLoadSucceeded());

            tryCompare(iconChangedSpy, "count", 2);

            // Icon database is not accessible with OTR profile.
            faviconImage.source = previousIcon;
            pixel = getFaviconPixel(faviconImage);
            compare(pixel[0], webEngineView.profile.offTheRecord ? 0 : 165);

            // This should pass with OTR too because icon is requested for the current page.
            faviconImage.source = "image://favicon/" + Qt.resolvedUrl("favicon-shortcut.html");
            pixel = getFaviconPixel(faviconImage);
            compare(pixel[0], 251);

            faviconImage.source = "image://favicon/" + Qt.resolvedUrl("favicon.html");
            pixel = getFaviconPixel(faviconImage);
            compare(pixel[0], webEngineView.profile.offTheRecord ? 0 : 165);

            faviconImage.destroy();
            webEngineView.profile = defaultProfile;
        }

        function test_iconDatabaseMultiView()
        {
            if (Screen.devicePixelRatio !== 1.0)
                skip("This test is not supported on High DPI screens.");

            var pixel;

            var faviconImage = Qt.createQmlObject("
                    import QtQuick\n
                    Image { width: 16; height: 16; sourceSize: Qt.size(width, height); cache: false; }", testCase);

            var webEngineView1 = Qt.createQmlObject("
                    import QtWebEngine\n
                    import Test.util\n
                    import '../../qmltests/data'\n
                    TestWebEngineView {\n
                        TempDir { id: tempDir }
                        profile: WebEngineProfile {\n
                            persistentStoragePath: tempDir.path() + '/WebEngineFavicon1'\n
                            offTheRecord: false\n
                        }\n
                    }", testCase);

            var webEngineView2 = Qt.createQmlObject("
                    import QtWebEngine\n
                    import Test.util\n
                    import '../../qmltests/data'\n
                    TestWebEngineView {\n
                        TempDir { id: tempDir }
                        profile: WebEngineProfile {\n
                            persistentStoragePath: tempDir.path() + '/WebEngineFavicon2'\n
                            offTheRecord: false\n
                        }\n
                    }", testCase);

            // Moke sure the icons have not been stored in the database yet.
            var icon1 = "image://favicon/" + Qt.resolvedUrl("icons/favicon.png");
            faviconImage.source = icon1;
            pixel = getFaviconPixel(faviconImage);
            compare(pixel[0], 0);

            var icon2 = "image://favicon/" + Qt.resolvedUrl("icons/qt32.ico");
            faviconImage.source = icon2;
            pixel = getFaviconPixel(faviconImage);
            compare(pixel[0], 0);

            webEngineView1.url = Qt.resolvedUrl("favicon.html"); // favicon.png -> 165
            verify(webEngineView1.waitForLoadSucceeded());
            tryCompare(webEngineView1, "icon", icon1);
            webEngineView1.url = "about:blank";
            verify(webEngineView1.waitForLoadSucceeded());

            webEngineView2.url = Qt.resolvedUrl("favicon-shortcut.html"); // qt32.ico -> 251
            verify(webEngineView2.waitForLoadSucceeded());
            tryCompare(webEngineView2, "icon", icon2);
            webEngineView2.url = "about:blank";
            verify(webEngineView2.waitForLoadSucceeded());

            faviconImage.source = "";
            compare(webEngineView1.icon, "");
            compare(webEngineView2.icon, "");

            faviconImage.source = icon1;
            pixel = getFaviconPixel(faviconImage);
            compare(pixel[0], 165);

            faviconImage.source = icon2;
            pixel = getFaviconPixel(faviconImage);
            compare(pixel[0], 251);

            faviconImage.source = "image://favicon/file:///does.not.exist.ico";
            pixel = getFaviconPixel(faviconImage);
            compare(pixel[0], 0);

            webEngineView1.destroy();
            webEngineView2.destroy();
            faviconImage.destroy();

            tempDir.removeRecursive(webEngineView1.profile.persistentStoragePath)
            tempDir.removeRecursive(webEngineView2.profile.persistentStoragePath)
        }
    }
}

