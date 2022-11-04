// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickpdfbookmarkmodel_p.h"
#include <QLoggingCategory>

QT_BEGIN_NAMESPACE

/*!
    \qmltype PdfBookmarkModel
//!    \instantiates QQuickPdfBookmarkModel
    \inqmlmodule QtQuick.Pdf
    \ingroup pdf
    \brief A tree of links (anchors) within a PDF document, such as the table of contents.
    \since 6.4

    A PDF document can contain a hierarchy of link destinations, usually
    representing the table of contents, to be shown in a sidebar in a PDF
    viewer, so that the user can quickly jump to those locations in the
    document. This QAbstractItemModel holds the information in a form
    suitable for display with TreeView, ListView, QTreeView or QListView.
*/

QQuickPdfBookmarkModel::QQuickPdfBookmarkModel(QObject *parent)
    : QPdfBookmarkModel(parent)
{
}

/*!
    \internal
*/
QQuickPdfBookmarkModel::~QQuickPdfBookmarkModel() = default;

/*!
    \qmlproperty PdfDocument PdfBookmarkModel::document

    This property holds the PDF document in which bookmarks are to be found.
*/
QQuickPdfDocument *QQuickPdfBookmarkModel::document() const
{
    return m_quickDocument;
}

void QQuickPdfBookmarkModel::setDocument(QQuickPdfDocument *document)
{
    if (document == m_quickDocument)
        return;
    m_quickDocument = document;
    QPdfBookmarkModel::setDocument(document->document());
    emit documentChanged();
}

QT_END_NAMESPACE

#include "moc_qquickpdfbookmarkmodel_p.cpp"
