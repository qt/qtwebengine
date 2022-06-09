// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickpdflinkmodel_p.h"
#include <QQuickItem>
#include <QQmlEngine>
#include <QStandardPaths>

QT_BEGIN_NAMESPACE

/*!
    \qmltype PdfLinkModel
//!    \instantiates QQuickPdfLinkModel
    \inqmlmodule QtQuick.Pdf
    \ingroup pdf
    \brief A representation of links within a PDF document.
    \since 5.15

    PdfLinkModel provides the geometry and the destination for each link
    that the specified \l page contains.

    The available model roles are:

    \value rectangle
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
            required property rect rectangle
            required property url url
            required property int page
            color: "transparent"
            border.color: "lightgrey"
            x: rectangle.x
            y: rectangle.y
            width: rectangle.width
            height: rectangle.height
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
    \c PdfScrollablePageView and \c PdfMultiPageView. PdfLinkModel is only needed
    when building PDF view components from scratch.
*/

QQuickPdfLinkModel::QQuickPdfLinkModel(QObject *parent)
    : QPdfLinkModel(parent)
{
}

/*!
    \internal
*/
QQuickPdfLinkModel::~QQuickPdfLinkModel() = default;

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
    if (document)
        QPdfLinkModel::setDocument(document->document());
}

/*!
    \qmlproperty int PdfLinkModel::page

    This property holds the page number on which links are to be found.
*/

QT_END_NAMESPACE

#include "moc_qquickpdflinkmodel_p.cpp"
