// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400

    TestCase {
        id: testCase
        name: "WebEngineFrame"
        when: windowShown

        function test_dummyFrame() {
            verify(webEngineView.mainFrame.isValid);
            compare(webEngineView.mainFrame.name,"");
            compare(webEngineView.mainFrame.htmlName,"");
            compare(webEngineView.mainFrame.children.length, 0);
            compare(webEngineView.mainFrame.url,"");
            compare(webEngineView.mainFrame.size, Qt.size(-1,-1));
            verify(webEngineView.mainFrame.isMainFrame);
        }

        function test_mainFrame() {
            webEngineView.url = Qt.resolvedUrl("iframes.html");
            verify(webEngineView.waitForLoadSucceeded());
            verify(webEngineView.mainFrame.isValid);
            compare(webEngineView.mainFrame.name,"test-main-frame");
            compare(webEngineView.mainFrame.htmlName,"");
            compare(webEngineView.mainFrame.url,Qt.resolvedUrl("iframes.html"));
            //compare(webEngineView.mainFrame.size, Qt.size(200,400)); // TODO: broken
            verify(webEngineView.mainFrame.isMainFrame);
            compare(webEngineView.mainFrame.children.length, 2);
            var subFrame = webEngineView.mainFrame.children[0];
            compare(subFrame.name,"test-subframe1");
            verify(subFrame.isValid);
            compare(subFrame.htmlName,"iframe1-300x200");
            compare(subFrame.children.length, 0);
            compare(subFrame.url,Qt.url("about:blank"));
            compare(subFrame.size, Qt.size(300,200));
            subFrame = webEngineView.mainFrame.children[1];
            compare(subFrame.name,"test-subframe2");
            verify(subFrame.isValid);
            compare(subFrame.htmlName,"iframe2-350x250");
            compare(subFrame.children.length, 0);
            compare(subFrame.url,Qt.url("about:blank"));
            compare(subFrame.size, Qt.size(350,250));
            verify(!subFrame.isMainFrame);
          }

        function test_findFrame() {
            webEngineView.url = Qt.resolvedUrl("iframes.html");
            verify(webEngineView.waitForLoadSucceeded());
            verify(webEngineView.mainFrame.isValid);
            compare(webEngineView.mainFrame.name,"test-main-frame");
            compare(webEngineView.mainFrame.children.length, 2);
            var subFrame = webEngineView.findFrameByName("test-subframe1");
            compare(subFrame.name,"test-subframe1");
            verify(subFrame.isValid);
            compare(subFrame.htmlName,"iframe1-300x200");
            compare(subFrame.children.length, 0);
            compare(subFrame.url,Qt.url("about:blank"));
            compare(subFrame.size, Qt.size(300,200));
            verify(!subFrame.isMainFrame);
            subFrame = webEngineView.findFrameByName("test-subframe2");
            compare(subFrame.name,"test-subframe2");
            verify(subFrame.isValid);
            compare(subFrame.htmlName,"iframe2-350x250");
            compare(subFrame.children.length, 0);
            compare(subFrame.url,Qt.url("about:blank"));
            compare(subFrame.size, Qt.size(350,250));
            verify(!subFrame.isMainFrame);
        }

        function test_runJavaScript() {
            var result1, result2;
            webEngineView.url = Qt.resolvedUrl("iframes.html")
            verify(webEngineView.waitForLoadSucceeded())
            verify(webEngineView.mainFrame.isValid);
            webEngineView.mainFrame.runJavaScript("window.frames[0].name", function(name){
                result1 = name;
            });
            webEngineView.mainFrame.runJavaScript("window.frames[1].name", function(name){
                result2 = name;
            });
            tryVerify(function() {
                return  result1 == "test-subframe1" && result2 == "test-subframe2"}
            );
        }
    }
}
