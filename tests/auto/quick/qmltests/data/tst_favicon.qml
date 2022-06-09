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

    function removeFaviconProviderPrefix(url) {
        return url.toString().substring(16)
    }

    SignalSpy {
        id: iconChangedSpy
        target: webEngineView
        signalName: "iconChanged"
    }

    Image {
        id: favicon
        source: webEngineView.icon
    }

    TestCase {
        id: testCase
        name: "WebEngineFavicon"
        when: windowShown


        function init() {
            // It is worth to restore the initial state with loading a blank page before all test functions.
            webEngineView.url = 'about:blank';
            verify(webEngineView.waitForLoadSucceeded());
            iconChangedSpy.clear();
            webEngineView.settings.touchIconsEnabled = false;
            webEngineView.settings.autoLoadIconsForPage = true;
        }

        function test_faviconLoad_data() {
            return [
                   { tag: "OTR", profile: defaultProfile },
                   { tag: "non-OTR", profile: nonOTRProfile },
            ];
        }

        function test_faviconLoad(row) {
            webEngineView.profile = row.profile
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("favicon.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            compare(iconChangedSpy.count, 1)

            tryCompare(favicon, "status", Image.Ready)
            compare(favicon.width, 32)
            compare(favicon.height, 32)
        }

        function test_faviconLoadEncodedUrl_data() {
            return [
                   { tag: "OTR", profile: defaultProfile },
                   { tag: "non-OTR", profile: nonOTRProfile },
            ];
        }

        function test_faviconLoadEncodedUrl(row) {
            webEngineView.profile = row.profile
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("favicon2.html?favicon=load should work with#whitespace!")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            compare(iconChangedSpy.count, 1)

            tryCompare(favicon, "status", Image.Ready)
            compare(favicon.width, 32)
            compare(favicon.height, 32)
        }

        function test_faviconLoadAfterHistoryNavigation_data() {
            return [
                   { tag: "OTR", profile: defaultProfile },
                   { tag: "non-OTR", profile: nonOTRProfile },
            ];
        }

        function test_faviconLoadAfterHistoryNavigation(row) {
            webEngineView.profile = row.profile
            compare(iconChangedSpy.count, 0)

            var iconUrl

            webEngineView.url = Qt.resolvedUrl("favicon.html")
            verify(webEngineView.waitForLoadSucceeded())
            tryCompare(iconChangedSpy, "count", 1)
            iconUrl = removeFaviconProviderPrefix(webEngineView.icon)
            compare(iconUrl, Qt.resolvedUrl("icons/favicon.png"))

            iconChangedSpy.clear()
            webEngineView.url = Qt.resolvedUrl("favicon-shortcut.html")
            verify(webEngineView.waitForLoadSucceeded())
            tryCompare(iconChangedSpy, "count", 2)
            iconUrl = removeFaviconProviderPrefix(webEngineView.icon)
            compare(iconUrl, Qt.resolvedUrl("icons/qt32.ico"))

            iconChangedSpy.clear()
            webEngineView.goBack();
            verify(webEngineView.waitForLoadSucceeded())
            tryCompare(iconChangedSpy, "count", 2)
            iconUrl = removeFaviconProviderPrefix(webEngineView.icon)
            compare(iconUrl, Qt.resolvedUrl("icons/favicon.png"))

            iconChangedSpy.clear()
            webEngineView.goForward();
            verify(webEngineView.waitForLoadSucceeded())
            tryCompare(iconChangedSpy, "count", 2)
            iconUrl = removeFaviconProviderPrefix(webEngineView.icon)
            compare(iconUrl, Qt.resolvedUrl("icons/qt32.ico"))
        }

        function test_faviconLoadPushState_data() {
            return [
                   { tag: "OTR", profile: defaultProfile },
                   { tag: "non-OTR", profile: nonOTRProfile },
            ];
        }

        function test_faviconLoadPushState(row) {
            webEngineView.profile = row.profile;
            compare(iconChangedSpy.count, 0);

            var iconUrl;

            webEngineView.url = Qt.resolvedUrl("favicon.html");
            verify(webEngineView.waitForLoadSucceeded());
            tryCompare(iconChangedSpy, "count", 1);
            iconUrl = removeFaviconProviderPrefix(webEngineView.icon);
            compare(iconUrl, Qt.resolvedUrl("icons/favicon.png"));

            iconChangedSpy.clear();

            // pushState() is a same document navigation and should not reset or
            // update favicon.
            compare(webEngineView.history.items.rowCount(), 1);
            runJavaScript("history.pushState('', '')");
            tryVerify(function() { return webEngineView.history.items.rowCount() === 2; });

            // Favicon change is not expected.
            compare(iconChangedSpy.count, 0);
            iconUrl = removeFaviconProviderPrefix(webEngineView.icon);
            compare(iconUrl, Qt.resolvedUrl("icons/favicon.png"));
        }

        function test_noFavicon_data() {
            return [
                   { tag: "OTR", profile: defaultProfile },
                   { tag: "non-OTR", profile: nonOTRProfile },
            ];
        }

        function test_noFavicon(row) {
            webEngineView.profile = row.profile
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("test1.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            compare(iconChangedSpy.count, 0)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }

        function test_aboutBlank_data() {
            return [
                   { tag: "OTR", profile: defaultProfile },
                   { tag: "non-OTR", profile: nonOTRProfile },
            ];
        }

        function test_aboutBlank(row) {
            webEngineView.profile = row.profile
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("about:blank")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            compare(iconChangedSpy.count, 0)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }

        function test_unavailableFavicon_data() {
            return [
                   { tag: "OTR", profile: defaultProfile },
                   { tag: "non-OTR", profile: nonOTRProfile },
            ];
        }

        function test_unavailableFavicon(row) {
            webEngineView.profile = row.profile
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("favicon-unavailable.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            compare(iconChangedSpy.count, 0)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }

        function test_errorPageEnabled_data() {
            return [
                   { tag: "OTR", profile: defaultProfile },
                   { tag: "non-OTR", profile: nonOTRProfile },
            ];
        }

        function test_errorPageEnabled(row) {
            webEngineView.profile = row.profile
            webEngineView.settings.errorPageEnabled = true

            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("http://url.invalid")
            webEngineView.url = url
            verify(webEngineView.waitForLoadFailed(20000))

            compare(iconChangedSpy.count, 0)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }

        function test_errorPageDisabled_data() {
            return [
                   { tag: "OTR", profile: defaultProfile },
                   { tag: "non-OTR", profile: nonOTRProfile },
            ];
        }

        function test_errorPageDisabled(row) {
            webEngineView.profile = row.profile
            webEngineView.settings.errorPageEnabled = false

            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("http://url.invalid")
            webEngineView.url = url
            verify(webEngineView.waitForLoadFailed(20000))

            compare(iconChangedSpy.count, 0)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }

        function test_bestFavicon_data() {
            return [
                   { tag: "OTR", profile: defaultProfile },
                   { tag: "non-OTR", profile: nonOTRProfile },
            ];
        }

        function test_bestFavicon(row) {
            webEngineView.profile = row.profile
            compare(iconChangedSpy.count, 0)
            var url, iconUrl

            url = Qt.resolvedUrl("favicon-misc.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            compare(iconChangedSpy.count, 1)

            iconUrl = removeFaviconProviderPrefix(webEngineView.icon)
            // Touch icon is ignored
            compare(iconUrl, Qt.resolvedUrl("icons/qt32.ico"))
            tryCompare(favicon, "status", Image.Ready)
            compare(favicon.width, 32)
            compare(favicon.height, 32)

            iconChangedSpy.clear()

            url = Qt.resolvedUrl("favicon-shortcut.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            tryCompare(iconChangedSpy, "count", 2)
            iconUrl = removeFaviconProviderPrefix(webEngineView.icon)

            // If touch icon is disabled, FaviconHandler propagates the icon closest to size 16x16
            compare(iconUrl, Qt.resolvedUrl("icons/qt32.ico"))
            tryCompare(favicon, "status", Image.Ready)
            compare(favicon.width, 32)
            compare(favicon.height, 32)
        }

        function test_touchIcon_data() {
            return [
                   { tag: "OTR", profile: defaultProfile },
                   { tag: "non-OTR", profile: nonOTRProfile },
            ];
        }

        function test_touchIcon(row) {
            webEngineView.profile = row.profile
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("favicon-touch.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            compare(iconChangedSpy.count, 0)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
            compare(favicon.width, 0)
            compare(favicon.height, 0)

            webEngineView.settings.touchIconsEnabled = true

            url = Qt.resolvedUrl("favicon-touch.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            iconUrl = removeFaviconProviderPrefix(webEngineView.icon)
            compare(iconUrl, Qt.resolvedUrl("icons/qt144.png"))
            compare(iconChangedSpy.count, 1)
            tryCompare(favicon, "status", Image.Ready)
            compare(favicon.width, 144)
            compare(favicon.height, 144)
        }

        function test_multiIcon_data() {
            return [
                   { tag: "OTR", profile: defaultProfile },
                   { tag: "non-OTR", profile: nonOTRProfile },
            ];
        }

        function test_multiIcon(row) {
            webEngineView.profile = row.profile
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("favicon-multi.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            compare(iconChangedSpy.count, 1)
            tryCompare(favicon, "status", Image.Ready)
            compare(favicon.width, 32)
            compare(favicon.height, 32)
        }

        function test_dynamicFavicon_data() {
            return [
                   { tag: "OTR", profile: defaultProfile },
                   { tag: "non-OTR", profile: nonOTRProfile },
            ];
        }

        function test_dynamicFavicon(row) {
            webEngineView.profile = row.profile
            compare(iconChangedSpy.count, 0)

            var faviconImage = Qt.createQmlObject("
                    import QtQuick\n
                    Image { width: 16; height: 16; sourceSize: Qt.size(width, height); objectName: 'image' }", testCase)
            faviconImage.source = Qt.binding(function() { return webEngineView.icon; });

            var colors = [
                {"url": "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8DwHwAFBQIAX8jx0gAAAABJRU5ErkJggg==", "r": 255, "g": 0, "b": 0},
                {"url": "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M/wHwAEBgIApD5fRAAAAABJRU5ErkJggg==", "r": 0, "g": 255, "b": 0},
                {"url": "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPj/HwADBwIAMCbHYQAAAABJRU5ErkJggg==", "r": 0, "g": 0, "b": 255},
            ];
            var pixel;

            compare(iconChangedSpy.count, 0);
            webEngineView.loadHtml(
                        "<html>" +
                        "<link rel='icon' type='image/png' href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII='/>" +
                        "</html>"
            );
            verify(webEngineView.waitForLoadSucceeded());
            tryCompare(iconChangedSpy, "count", 1);

            pixel = getItemPixel(faviconImage);
            compare(pixel[0], 0);
            compare(pixel[1], 0);
            compare(pixel[2], 0);

            for (var i = 0; i < colors.length; ++i) {
                iconChangedSpy.clear();
                runJavaScript("document.getElementsByTagName('link')[0].href = 'data:image/png;base64," + colors[i]["url"] + "';");
                tryCompare(faviconImage, "source", "image://favicon/data:image/png;base64," + colors[i]["url"]);
                compare(iconChangedSpy.count, 1);

                pixel = getItemPixel(faviconImage);
                compare(pixel[0], colors[i]["r"]);
                compare(pixel[1], colors[i]["g"]);
                compare(pixel[2], colors[i]["b"]);
            }

            faviconImage.destroy()
        }

        function test_touchIconWithSameURL_data() {
            return [
                   { tag: "OTR", profile: defaultProfile },
                   { tag: "non-OTR", profile: nonOTRProfile },
            ];
        }

        function test_touchIconWithSameURL(row)
        {
            webEngineView.profile = row.profile;
            compare(iconChangedSpy.count, 0);

            var icon = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=";

            webEngineView.loadHtml(
                        "<html>" +
                        "<link rel='icon' type='image/png' href='" + icon + "'/>" +
                        "<link rel='apple-touch-icon' type='image/png' href='" + icon + "'/>" +
                        "</html>"
            );
            verify(webEngineView.waitForLoadSucceeded());

            // The default favicon has to be loaded even if its URL is also set as a touch icon while touch icons are disabled.
            tryCompare(iconChangedSpy, "count", 1);
            compare(webEngineView.icon.toString().replace(/^image:\/\/favicon\//, ''), icon);

            iconChangedSpy.clear();

            webEngineView.loadHtml(
                        "<html>" +
                        "<link rel='apple-touch-icon' type='image/png' href='" + icon + "'/>" +
                        "</html>"
            );
            verify(webEngineView.waitForLoadSucceeded());

            // This page only has a touch icon. With disabled touch icons we don't expect any icon to be shown even if the same icon
            // was loaded previously.
            tryCompare(iconChangedSpy, "count", 1);
            verify(!webEngineView.icon.toString().replace(/^image:\/\/favicon\//, ''));
        }

        function test_iconsDisabled_data() {
            return [
                   { tag: "misc", url: Qt.resolvedUrl("favicon-misc.html") },
                   { tag: "shortcut", url: Qt.resolvedUrl("favicon-shortcut.html") },
                   { tag: "single", url: Qt.resolvedUrl("favicon-single.html") },
                   { tag: "touch", url: Qt.resolvedUrl("favicon-touch.html") },
                   { tag: "unavailable", url: Qt.resolvedUrl("favicon-unavailable.html") },
            ];
        }

        function test_iconsDisabled(row) {
            webEngineView.settings.autoLoadIconsForPage = false
            webEngineView.profile = defaultProfile
            compare(iconChangedSpy.count, 0)

            webEngineView.url = row.url
            verify(webEngineView.waitForLoadSucceeded())

            compare(iconChangedSpy.count, 0)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }

        function test_touchIconsEnabled_data() {
            return [
                   { tag: "misc", url: Qt.resolvedUrl("favicon-misc.html"), expectedIconUrl: Qt.resolvedUrl("icons/qt144.png") },
                   { tag: "shortcut", url: Qt.resolvedUrl("favicon-shortcut.html"), expectedIconUrl: Qt.resolvedUrl("icons/qt144.png") },
                   { tag: "single", url: Qt.resolvedUrl("favicon-single.html"), expectedIconUrl: Qt.resolvedUrl("icons/qt32.ico") },
                   { tag: "touch", url: Qt.resolvedUrl("favicon-touch.html"), expectedIconUrl: Qt.resolvedUrl("icons/qt144.png") },
            ];
        }

        function test_touchIconsEnabled(row) {
            webEngineView.settings.touchIconsEnabled = true
            webEngineView.profile = defaultProfile
            compare(iconChangedSpy.count, 0)

            webEngineView.url = row.url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            compare(iconChangedSpy.count, 1)

            var iconUrl = removeFaviconProviderPrefix(webEngineView.icon)
            compare(iconUrl, row.expectedIconUrl)
        }
    }
}
