/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
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
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickpdflinkmodel_p.h"
#include <QQuickItem>
#include <QQmlEngine>
#include <QStandardPaths>

QT_BEGIN_NAMESPACE

/*!
    \qmltype PdfLinkModel
    \instantiates QQuickPdfLinkModel
    \inqmlmodule QtQuick.Pdf
    \ingroup pdf
    \brief A representation of links within a PDF document.
    \since 5.15

    PdfLinkModel provides the geometry and the destination for each link
    that the specified \l page contains.

    The available model roles are:

    \value rect
        Bounding rectangle around the link.
    \value url
        If the link is a web link, the URL for that; otherwise an empty URL.
    \value page
        If the link is an internal link, the page number to which the link should jump; otherwise \c {-1}.
    \value location
        If the link is an internal link, the location on the page to which the link should jump.
    \value zoom
        If the link is an internal link, the intended zoom level on the destination page.

    Normally it will be used with \l {QtQuick::Repeater}{Repeater} to visualize
    the links and provide the ability to click them:

    \qml
    Repeater {
        model: PdfLinkModel {
            document: root.document
            page: image.currentFrame
        }
        delegate: Rectangle {
            color: "transparent"
            border.color: "lightgrey"
            x: rect.x
            y: rect.y
            width: rect.width
            height: rect.height
            HoverHandler { cursorShape: Qt.PointingHandCursor }
            TapHandler {
                onTapped: {
                    if (page >= 0)
                        image.currentFrame = page
                    else
                        Qt.openUrlExternally(url)
                }
            }
        }
    }
    \endqml

    \note General-purpose PDF viewing capabilities are provided by
    \l PdfScrollablePageView and \l PdfMultiPageView. PdfLinkModel is only needed
    when building PDF view components from scratch.
*/

QQuickPdfLinkModel::QQuickPdfLinkModel(QObject *parent)
    : QPdfLinkModel(parent)
{
}

/*!
    \qmlproperty PdfDocument PdfLinkModel::document

    This property holds the PDF document in which links are to be found.
*/
QQuickPdfDocument *QQuickPdfLinkModel::document() const
{
    return m_quickDocument;
}

void QQuickPdfLinkModel::setDocument(QQuickPdfDocument *document)
{
    if (document == m_quickDocument)
        return;
    m_quickDocument = document;
    QPdfLinkModel::setDocument(&document->m_doc);
}

/*!
    \qmlproperty int PdfLinkModel::page

    This property holds the page number on which links are to be found.
*/

QT_END_NAMESPACE
