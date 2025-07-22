// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Fusion

Rectangle {
    id: downloadView
    color: "lightgray"
    property var pendingDownloadRequest: null

    ListModel {
        id: downloadModel
        property var downloads: []
    }

    function append(download) {
        downloadModel.append(download);
        downloadModel.downloads.push(download);
    }

    Component {
        id: downloadItemDelegate

        Rectangle {
            id: downloadItem
            width: listView.width
            height: childrenRect.height
            anchors.margins: 10
            radius: 3
            color: "transparent"
            border.color: "black"

            required property int index

            Rectangle {
                id: progressBar

                property real progress: {
                    let d = downloadModel.downloads[downloadItem.index]
                    return d ? d.receivedBytes / d.totalBytes : 0
                }

                radius: 3
                color: width === listView.width ? "green" : "#2b74c7"
                width: listView.width * progress
                height: cancelButton.height

                Behavior on width {
                    SmoothedAnimation { duration: 100 }
                }
            }
            Rectangle {
                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: 20
                }
                Label {
                    id: label
                    text: {
                        let d = downloadModel.downloads[downloadItem.index]
                        return d ? d.downloadDirectory + "/" + d.downloadFileName : qsTr("")
                    }
                    anchors {
                        verticalCenter: cancelButton.verticalCenter
                        left: parent.left
                        right: cancelButton.left
                    }
                }
                Button {
                    id: cancelButton
                    anchors.right: parent.right
                    icon.source: "icons/3rdparty/process-stop.png"
                    onClicked: {
                        var download = downloadModel.downloads[downloadItem.index];

                        download.cancel();

                        downloadModel.downloads = downloadModel.downloads.filter(function (el) {
                            return el.id !== download.id;
                        });
                        downloadModel.remove(downloadItem.index);
                    }
                }
            }
        }

    }
    ListView {
        id: listView
        anchors {
            topMargin: 10
            top: parent.top
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
        width: parent.width - 20
        spacing: 5

        model: downloadModel
        delegate: downloadItemDelegate

        Text {
            visible: !listView.count
            horizontalAlignment: Text.AlignHCenter
            height: 30
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            font.pixelSize: 20
            text: "No active downloads."
        }

        Rectangle {
            color: "gray"
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            height: 30
            Button {
                id: okButton
                text: "OK"
                anchors.centerIn: parent
                onClicked: {
                    downloadView.visible = false;
                }
            }
        }
    }
}
