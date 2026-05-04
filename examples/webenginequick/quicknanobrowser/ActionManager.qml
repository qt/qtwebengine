// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Fusion
import QtWebEngine

Item {
    id: root
    required property ApplicationWindow browserWindow
    required property WebEngineView webView
    required property DownloadView downloads
    required property WebToolBar toolBar
    required property FindBar searchBar
    required property FullScreenNotification fullScreenNotifications

    signal newTabRequested(WebEngineProfile profile)

    Action {
        shortcut: "Ctrl+D"
        onTriggered: {
            root.downloads.visible = !root.downloads.visible;
        }
    }
    Action {
        shortcut: "Ctrl+L"
        onTriggered: {
            root.toolBar.addressBar.forceActiveFocus();
            root.toolBar.addressBar.selectAll();
        }
    }
    Action {
        shortcut: StandardKey.Refresh
        onTriggered: {
            if (root.webView)
                root.webView.reload();
        }
    }
    Action {
        shortcut: StandardKey.AddTab
        onTriggered: {
            root.newTabRequested();
            root.toolBar.addressBar.forceActiveFocus();
            root.toolBar.addressBar.selectAll();
        }
    }
    Action {
        shortcut: StandardKey.Close
        onTriggered: {
            root.webView.triggerWebAction(WebEngineView.RequestClose);
        }
    }
    Action {
        shortcut: StandardKey.Quit
        onTriggered: root.browserWindow.close()
    }
    Action {
        shortcut: "Escape"
        onTriggered: {
            if (root.webView.state === "FullScreen") {
                root.fullScreenNotifications.hide();
                root.webView.triggerWebAction(WebEngineView.ExitFullScreen);
            }

            if (root.searchBar.visible)
                root.searchBar.visible = false;
        }
    }
    Action {
        shortcut: "Ctrl+0"
        onTriggered: root.webView.zoomFactor = 1.0
    }
    Action {
        shortcut: StandardKey.ZoomOut
        onTriggered: root.webView.zoomFactor -= 0.1
    }
    Action {
        shortcut: StandardKey.ZoomIn
        onTriggered: root.webView.zoomFactor += 0.1
    }

    Action {
        shortcut: StandardKey.Copy
        onTriggered: root.webView.triggerWebAction(WebEngineView.Copy)
    }
    Action {
        shortcut: StandardKey.Cut
        onTriggered: root.webView.triggerWebAction(WebEngineView.Cut)
    }
    Action {
        shortcut: StandardKey.Paste
        onTriggered: root.webView.triggerWebAction(WebEngineView.Paste)
    }
    Action {
        shortcut: "Shift+" + StandardKey.Paste
        onTriggered: root.webView.triggerWebAction(WebEngineView.PasteAndMatchStyle)
    }
    Action {
        shortcut: StandardKey.SelectAll
        onTriggered: root.webView.triggerWebAction(WebEngineView.SelectAll)
    }
    Action {
        shortcut: StandardKey.Undo
        onTriggered: root.webView.triggerWebAction(WebEngineView.Undo)
    }
    Action {
        shortcut: StandardKey.Redo
        onTriggered: root.webView.triggerWebAction(WebEngineView.Redo)
    }
    Action {
        shortcut: StandardKey.Back
        onTriggered: root.webView.triggerWebAction(WebEngineView.Back)
    }
    Action {
        shortcut: StandardKey.Forward
        onTriggered: root.webView.triggerWebAction(WebEngineView.Forward)
    }
    Action {
        shortcut: StandardKey.Find
        onTriggered: {
            if (!root.searchBar.visible)
                root.searchBar.visible = true;
        }
    }
    Action {
        shortcut: StandardKey.FindNext
        onTriggered: root.searchBar.findNext()
    }
    Action {
        shortcut: StandardKey.FindPrevious
        onTriggered: root.searchBar.findPrevious()
    }
}
