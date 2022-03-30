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
