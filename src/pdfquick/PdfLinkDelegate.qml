/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
import QtQuick
import QtQuick.Controls

/*!
    \qmltype PdfLinkDelegate
    \inqmlmodule QtQuick.Pdf
    \brief A component to decorate hyperlinks on a PDF page.

    PdfLinkDelegate provides the component that QML-based PDF viewers
    instantiate on top of each hyperlink that is found on each PDF page.

    This component does not provide any visual decoration, because often the
    hyperlinks will already be formatted in a distinctive way; but when the
    mouse cursor hovers, it changes to Qt::PointingHandCursor, and a tooltip
    appears after a delay. Clicking emits the goToLocation() signal if the link
    is internal, or calls Qt.openUrlExternally() if the link contains a URL.

    \sa PdfPageView, PdfScrollablePageView, PdfMultiPageView
*/
Item {
    id: root
    required property var link
    required property rect rectangle
    required property url url
    required property int page
    required property point location
    required property real zoom

    /*!
        \qmlsignal PdfLinkDelegate::tapped(link)

        Emitted on mouse click or touch tap.
    */
    signal tapped(var link)

    /*!
        \qmlsignal PdfLinkDelegate::contextMenuRequested(link)

        Emitted on mouse right-click or touch long-press.
    */
    signal contextMenuRequested(var link)

    HoverHandler {
        id: linkHH
        cursorShape: Qt.PointingHandCursor
    }
    TapHandler {
        onTapped: root.tapped(link)
    }
    TapHandler {
        acceptedButtons: Qt.RightButton
        onTapped: root.contextMenuRequested(link)
    }
    TapHandler {
        acceptedDevices: PointerDevice.TouchScreen
        onTapped: root.contextMenuRequested(link)
    }
    ToolTip {
        visible: linkHH.hovered
        delay: 1000
        property string destFormat: qsTr("Page %1 location %2, %3 zoom %4")
        text: page >= 0 ?
                  destFormat.arg(page + 1).arg(location.x.toFixed(1)).arg(location.y.toFixed(1)).arg(zoom) :
                  url
    }
}
