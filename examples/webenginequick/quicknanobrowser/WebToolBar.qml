// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWebEngine

ToolBar {
    id: root
    required property WebEngineView currentWebView

    // When using style "mac", ToolButtons are not supposed to accept focus.
    property bool platformIsMac: Qt.platform.os === "osx"

    property alias loadImagesChecked: loadImages.checked
    property alias javaScriptEnabledChecked: javaScriptEnabledMenuItem.checked
    property alias errorPageEnabledChecked: errorPageEnabledMenuItem.checked
    property alias pluginsEnabledChecked: pluginsEnabledMenuItem.checked
    property alias fullScreenSupportEnabledChecked: fullScreenSupportEnabledMenuItem.checked
    property alias autoLoadIconsForPageChecked: autoLoadIconsForPageMenuItem.checked
    property alias touchIconsEnabledChecked: touchIconsEnabledMenuItem.checked
    property alias webRTCPublicInterfacesOnlyChecked: webRTCPublicInterfacesOnlyMenuItem.checked
    property alias devToolsEnabledChecked: devToolsEnabledMenuItem.checked
    property alias pdfViewerEnabledChecked: pdfViewerEnabledMenuItem.checked
    property alias javascriptCanAccessClipboardChecked: javascriptCanAccessClipboardMenuItem.checked
    property alias javascriptCanPasteChecked: javascriptCanPasteMenuItem.checked
    property alias disableImageAnimationChecked: disableImageAnimation.checked
    property alias allowImageAnimationChecked: allowImageAnimation.checked
    property alias animateImageOnceChecked: animateImageOnce.checked
    property alias addressBar: addressBar

    RowLayout {
        anchors.fill: parent
        ToolButton {
            enabled: root.currentWebView?.canGoBack || root.currentWebView?.canGoForward
            onClicked: historyMenu.open()
            text: qsTr("▼")
            Menu {
                id: historyMenu
                Instantiator {
                    model: root.currentWebView?.history?.items
                    MenuItem {
                        required property var model
                        text: model.title
                        onTriggered: root.currentWebView.goBackOrForward(model.offset)
                        checkable: !enabled
                        checked: !enabled
                        enabled: model.offset
                    }

                    onObjectAdded: function (index, object) {
                        historyMenu.insertItem(index, object);
                    }
                    onObjectRemoved: function (index, object) {
                        historyMenu.removeItem(object);
                    }
                }
            }
        }

        ToolButton {
            id: backButton
            icon.source: "icons/3rdparty/go-previous.png"
            onClicked: root.currentWebView.goBack()
            enabled: root.currentWebView?.canGoBack ?? false
            activeFocusOnTab: !root.platformIsMac
        }
        ToolButton {
            id: forwardButton
            icon.source: "icons/3rdparty/go-next.png"
            onClicked: root.currentWebView.goForward()
            enabled: root.currentWebView?.canGoForward ?? false
            activeFocusOnTab: !root.platformIsMac
        }
        ToolButton {
            id: reloadButton
            icon.source: root.currentWebView?.loading ? "icons/3rdparty/process-stop.png" : "icons/3rdparty/view-refresh.png"
            onClicked: root.currentWebView?.loading ? root.currentWebView.stop() : root.currentWebView.reload()
            activeFocusOnTab: !root.platformIsMac
        }
        TextField {
            id: addressBar
            Image {
                id: faviconImage
                anchors.verticalCenter: addressBar.verticalCenter
                x: 5
                z: 2
                width: 16
                height: 16
                sourceSize: Qt.size(width, height)
                source: root.currentWebView?.icon ? root.currentWebView.icon : ''
            }
            MouseArea {
                id: textFieldMouseArea
                acceptedButtons: Qt.RightButton
                anchors.fill: parent
                onClicked: {
                    var textSelectionStartPos = addressBar.selectionStart;
                    var textSelectionEndPos = addressBar.selectionEnd;
                    textFieldContextMenu.open();
                    addressBar.select(textSelectionStartPos, textSelectionEndPos);
                }
                Menu {
                    id: textFieldContextMenu
                    x: textFieldMouseArea.mouseX
                    y: textFieldMouseArea.mouseY
                    MenuItem {
                        text: qsTr("Cut")
                        onTriggered: addressBar.cut()
                        enabled: addressBar.selectedText.length > 0
                    }
                    MenuItem {
                        text: qsTr("Copy")
                        onTriggered: addressBar.copy()
                        enabled: addressBar.selectedText.length > 0
                    }
                    MenuItem {
                        text: qsTr("Paste")
                        onTriggered: addressBar.paste()
                        enabled: addressBar.canPaste
                    }
                    MenuItem {
                        text: qsTr("Delete")
                        onTriggered: addressBar.text = qsTr("")
                        enabled: addressBar.selectedText.length > 0
                    }
                    MenuSeparator {}
                    MenuItem {
                        text: qsTr("Select All")
                        onTriggered: addressBar.selectAll()
                        enabled: addressBar.text.length > 0
                    }
                }
            }
            leftPadding: 26
            focus: true
            Layout.fillWidth: true
            Binding on text {
                when: root.currentWebView
                value: root.currentWebView.url
            }
            onAccepted: root.currentWebView.url = Utils.fromUserInput(text)
            selectByMouse: true
        }
        ToolButton {
            id: settingsMenuButton
            text: qsTr("⋮")
            onClicked: settingsMenu.open()
            Menu {
                id: settingsMenu
                y: settingsMenuButton.height
                MenuItem {
                    id: loadImages
                    text: "Autoload images"
                    checkable: true
                    checked: WebEngine.settings.autoLoadImages
                }
                MenuItem {
                    id: javaScriptEnabledMenuItem
                    text: "JavaScript On"
                    checkable: true
                    checked: WebEngine.settings.javascriptEnabled
                }
                MenuItem {
                    id: errorPageEnabledMenuItem
                    text: "ErrorPage On"
                    checkable: true
                    checked: WebEngine.settings.errorPageEnabled
                }
                MenuItem {
                    id: pluginsEnabledMenuItem
                    text: "Plugins On"
                    checkable: true
                    checked: true
                }
                MenuItem {
                    id: fullScreenSupportEnabledMenuItem
                    text: "FullScreen On"
                    checkable: true
                    checked: WebEngine.settings.fullScreenSupportEnabled
                }
                MenuItem {
                    id: offTheRecordEnabled
                    text: "Off The Record"
                    checkable: true
                    checked: root.currentWebView?.profile === BrowserManager.otrPrototype.instance()
                    onToggled: function () {
                        if (root.currentWebView) {
                            root.currentWebView.profile = offTheRecordEnabled.checked ? BrowserManager.otrPrototype.instance() : BrowserManager.defaultProfilePrototype.instance();
                        }
                    }
                }
                MenuItem {
                    id: httpDiskCacheEnabled
                    text: "HTTP Disk Cache"
                    checkable: !root.currentWebView?.profile?.offTheRecord ?? false
                    checked: root.currentWebView?.profile.httpCacheType === WebEngineProfile.DiskHttpCache
                    onToggled: function () {
                        if (root.currentWebView) {
                            root.currentWebView.profile.httpCacheType = httpDiskCacheEnabled.checked ? WebEngineProfile.DiskHttpCache : WebEngineProfile.MemoryHttpCache;
                        }
                    }
                }
                MenuItem {
                    id: autoLoadIconsForPageMenuItem
                    text: "Icons On"
                    checkable: true
                    checked: WebEngine.settings.autoLoadIconsForPage
                }
                MenuItem {
                    id: touchIconsEnabledMenuItem
                    text: "Touch Icons On"
                    checkable: true
                    checked: WebEngine.settings.touchIconsEnabled
                    enabled: autoLoadIconsForPageMenuItem.checked
                }
                MenuItem {
                    id: webRTCPublicInterfacesOnlyMenuItem
                    text: "WebRTC Public Interfaces Only"
                    checkable: true
                    checked: WebEngine.settings.webRTCPublicInterfacesOnly
                }
                MenuItem {
                    id: devToolsEnabledMenuItem
                    text: "Open DevTools"
                    checkable: true
                    checked: false
                }
                MenuItem {
                    id: pdfViewerEnabledMenuItem
                    text: "PDF Viewer Enabled"
                    checkable: true
                    checked: WebEngine.settings.pdfViewerEnabled
                }
                Menu {
                    id: imageAnimationPolicyMenu
                    title: "Image Animation Policy"

                    MenuItem {
                        id: disableImageAnimation
                        text: "Disable All Image Animation"
                        checkable: true
                        autoExclusive: true
                        checked: WebEngine.settings.imageAnimationPolicy === WebEngineSettings.ImageAnimationPolicy.Disallow
                    }

                    MenuItem {
                        id: allowImageAnimation
                        text: "Allow All Animated Images"
                        checkable: true
                        autoExclusive: true
                        checked: WebEngine.settings.imageAnimationPolicy === WebEngineSettings.ImageAnimationPolicy.Allow
                    }

                    MenuItem {
                        id: animateImageOnce
                        text: "Animate Image Once"
                        checkable: true
                        autoExclusive: true
                        checked: WebEngine.settings.imageAnimationPolicy === WebEngineSettings.ImageAnimationPolicy.AnimateOnce
                    }
                }

                MenuItem {
                    id: javascriptCanAccessClipboardMenuItem
                    text: "JavaScript can access clipboard"
                    checkable: true
                    checked: WebEngine.settings.javascriptCanAccessClipboard
                }
                MenuItem {
                    id: javascriptCanPasteMenuItem
                    text: "JavaScript can paste"
                    checkable: true
                    checked: WebEngine.settings.javascriptCanPaste
                }
            }
        }
    }
    ProgressBar {
        id: progressBar
        height: 3
        anchors {
            left: parent.left
            top: parent.bottom
            right: parent.right
            leftMargin: parent.anchors.leftMargin
            rightMargin: parent.anchors.rightMargin
        }
        background: Item {}
        z: -2
        from: 0
        to: 100
        value: (root.currentWebView?.loadProgress < 100) ? root.currentWebView.loadProgress : 0
    }
}
