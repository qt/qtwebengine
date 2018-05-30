/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

import QtQuick 2.2
import QtTest 1.0
import QtWebEngine 1.8

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 400

    Component {
        id: signalSpy
        SignalSpy { }
    }

    TestCase {
        id: actionTests
        name: "WebEngineAction"
        when: windowShown

        function test_actions_data() {
            return [
                   { webAction: WebEngineView.Back, text: "Back", iconText: "go-previous", enabled: false },
                   { webAction: WebEngineView.Forward, text: "Forward", iconText: "go-next", enabled: false },
                   { webAction: WebEngineView.Stop, text: "Stop", iconText: "", enabled: false },
                   { webAction: WebEngineView.Reload, text: "Reload", iconText: "view-refresh", enabled: true },
                   { webAction: WebEngineView.Cut, text: "Cut", iconText: "Cut", enabled: true },
                   { webAction: WebEngineView.Copy, text: "Copy", iconText: "", enabled: true },
                   { webAction: WebEngineView.Paste, text: "Paste", iconText: "", enabled: true },
                   { webAction: WebEngineView.Undo, text: "Undo", iconText: "", enabled: true },
                   { webAction: WebEngineView.Redo, text: "Redo", iconText: "", enabled: true },
                   { webAction: WebEngineView.SelectAll, text: "Select all", iconText: "", enabled: true },
                   { webAction: WebEngineView.ReloadAndBypassCache, text: "Reload and Bypass Cache", iconText: "", enabled: true },
                   { webAction: WebEngineView.PasteAndMatchStyle, text: "Paste and match style", iconText: "", enabled: true },
                   { webAction: WebEngineView.OpenLinkInThisWindow, text: "Open link in this window", iconText: "", enabled: true },
                   { webAction: WebEngineView.OpenLinkInNewWindow, text: "Open link in new window", iconText: "", enabled: true },
                   { webAction: WebEngineView.OpenLinkInNewTab, text: "Open link in new tab", iconText: "", enabled: true },
                   { webAction: WebEngineView.CopyLinkToClipboard, text: "Copy link address", iconText: "", enabled: true },
                   { webAction: WebEngineView.DownloadLinkToDisk, text: "Save link", iconText: "", enabled: true },
                   { webAction: WebEngineView.CopyImageToClipboard, text: "Copy image", iconText: "", enabled: true },
                   { webAction: WebEngineView.CopyImageUrlToClipboard, text: "Copy image address", iconText: "", enabled: true },
                   { webAction: WebEngineView.DownloadImageToDisk, text: "Save image", iconText: "", enabled: true },
                   { webAction: WebEngineView.CopyMediaUrlToClipboard, text: "Copy media address", iconText: "", enabled: true },
                   { webAction: WebEngineView.ToggleMediaControls, text: "Show controls", iconText: "", enabled: true },
                   { webAction: WebEngineView.ToggleMediaLoop, text: "Loop", iconText: "", enabled: true },
                   { webAction: WebEngineView.ToggleMediaPlayPause, text: "Toggle Play/Pause", iconText: "", enabled: true },
                   { webAction: WebEngineView.ToggleMediaMute, text: "Toggle Mute", iconText: "", enabled: true },
                   { webAction: WebEngineView.DownloadMediaToDisk, text: "Save media", iconText: "", enabled: true },
                   { webAction: WebEngineView.InspectElement, text: "Inspect", iconText: "", enabled: true },
                   { webAction: WebEngineView.ExitFullScreen, text: "Exit full screen", iconText: "", enabled: true },
                   { webAction: WebEngineView.RequestClose, text: "Close Page", iconText: "", enabled: true },
                   { webAction: WebEngineView.Unselect, text: "Unselect", iconText: "", enabled: true },
                   { webAction: WebEngineView.SavePage, text: "Save page", iconText: "", enabled: true },
                   { webAction: WebEngineView.ViewSource, text: "View page source", iconText: "view-source", enabled: true },
                   { webAction: WebEngineView.ToggleBold, text: "&Bold", iconText: "", enabled: true },
                   { webAction: WebEngineView.ToggleItalic, text: "&Italic", iconText: "", enabled: true },
                   { webAction: WebEngineView.ToggleUnderline, text: "&Underline", iconText: "", enabled: true },
                   { webAction: WebEngineView.ToggleStrikethrough, text: "&Strikethrough", iconText: "", enabled: true },
                   { webAction: WebEngineView.AlignLeft, text: "Align &Left", iconText: "", enabled: true },
                   { webAction: WebEngineView.AlignCenter, text: "Align &Center", iconText: "", enabled: true },
                   { webAction: WebEngineView.AlignRight, text: "Align &Right", iconText: "", enabled: true },
                   { webAction: WebEngineView.AlignJustified, text: "Align &Justified", iconText: "", enabled: true },
                   { webAction: WebEngineView.Indent, text: "&Indent", iconText: "", enabled: true },
                   { webAction: WebEngineView.Outdent, text: "&Outdent", iconText: "", enabled: true },
                   { webAction: WebEngineView.InsertOrderedList, text: "Insert &Ordered List", iconText: "", enabled: true },
                   { webAction: WebEngineView.InsertUnorderedList, text: "Insert &Unordered List", iconText: "", enabled: true }
            ];
        }

        function test_actions(row) {
            webEngineView.url = Qt.resolvedUrl("test1.html");
            verify(webEngineView.waitForLoadSucceeded());

            var action = webEngineView.action(row.webAction);
            verify(action);

            compare(action.text, row.text);
            compare(action.iconText, row.iconText);
            compare(action.enabled, row.enabled);
        }

        function test_trigger() {
            webEngineView.url = Qt.resolvedUrl("test1.html");
            verify(webEngineView.waitForLoadSucceeded());

            var copyAction = webEngineView.action(WebEngineView.Copy);
            verify(copyAction);

            var stopAction = webEngineView.action(WebEngineView.Stop);
            verify(stopAction);

            var triggerSpy = createTemporaryObject(signalSpy, actionTests, {target: copyAction, signalName: "triggered"});
            var stopTriggerSpy = createTemporaryObject(signalSpy, actionTests, {target: stopAction, signalName: "triggered"});

            verify(copyAction.enabled);
            copyAction.trigger();
            compare(triggerSpy.count, 1);

            verify(!stopAction.enabled);
            stopAction.trigger();
            compare(stopTriggerSpy.count, 0);
        }
    }
}
