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

#include "qquickpdfdocument_p.h"
#include <QQuickItem>
#include <QQmlEngine>
#include <QStandardPaths>
#include <private/qguiapplication_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Document
    \instantiates QQuickPdfDocument
    \inqmlmodule QtQuick.Pdf
    \ingroup pdf
    \brief A representation of a PDF document.
    \since 5.15

    A Document provides access to PDF document meta-information.
    It is not necessary for rendering, as it is enough to use an
    \l Image with source set to the URL of the PDF.
*/

/*!
    Constructs a PDF document.
*/
QQuickPdfDocument::QQuickPdfDocument(QObject *parent)
    : QObject(parent)
{
    connect(&m_doc, &QPdfDocument::passwordChanged, this, &QQuickPdfDocument::passwordChanged);
    connect(&m_doc, &QPdfDocument::passwordRequired, this, &QQuickPdfDocument::passwordRequired);
    connect(&m_doc, &QPdfDocument::statusChanged, [=] (QPdfDocument::Status status) {
        emit statusChanged();
        if (status == QPdfDocument::Ready)
            emit metaDataChanged();
    });
    connect(&m_doc, &QPdfDocument::pageCountChanged, this, &QQuickPdfDocument::pageCountChanged);
}

void QQuickPdfDocument::componentComplete()
{
    if (m_doc.error() == QPdfDocument::IncorrectPasswordError)
        emit passwordRequired();
}

/*!
    \qmlproperty url Document::source

    This property holds a URL pointing to the PDF file to be loaded.

    \note At this time, only local filesystem URLs are supported.
*/
void QQuickPdfDocument::setSource(QUrl source)
{
    if (m_source == source)
        return;

    m_source = source;
    emit sourceChanged();
    m_doc.load(source.path());
}

/*!
    \qmlproperty bool Document::password

    This property holds the document password. If the passwordRequired()
    signal is emitted, the UI should prompt the user and then set this
    property so that document opening can continue.
*/
void QQuickPdfDocument::setPassword(const QString &password)
{
    if (m_doc.password() == password)
        return;
    m_doc.setPassword(password);
    if (source().isValid() && source().isLocalFile())
        m_doc.load(source().path());
}

/*!
    \qmlproperty int Document::pageCount

    This property holds the number of pages the PDF contains.
*/

/*!
    \qmlsignal Document::passwordRequired()

    This signal is emitted when the PDF requires a password in order to open.
    The UI in a typical PDF viewer should prompt the user for the password
    and then set the password property when the user has provided it.
*/

/*!
    \qmlmethod size Document::pagePointSize(int page)

    Returns the size of the given \a page in points.
*/
QSizeF QQuickPdfDocument::pagePointSize(int page) const
{
    return m_doc.pageSize(page);
}

/*!
    \qmlproperty string Document::title

    This property holds the document's title. A typical viewer UI can bind this
    to the \c Window.title property.
*/

/*!
    \qmlproperty string Document::author

    This property holds the name of the person who created the document.
*/

/*!
    \qmlproperty string Document::subject

    This property holds the subject of the document.
*/

/*!
    \qmlproperty string Document::keywords

    This property holds the keywords associated with the document.
*/

/*!
    \qmlproperty string Document::creator

    If the document was converted to PDF from another format, this property
    holds the name of the software that created the original document.
*/

/*!
    \qmlproperty string Document::producer

    If the document was converted to PDF from another format, this property
    holds the name of the software that converted it to PDF.
*/

/*!
    \qmlproperty string Document::creationDate

    This property holds the date and time the document was created.
*/

/*!
    \qmlproperty string Document::modificationDate

    This property holds the date and time the document was most recently
    modified.
*/

/*!
    \qmlproperty enum Document::status

    This property tells the current status of the document. The possible values are:

    \value PdfDocument.Null The initial status after the document has been created or after it has been closed.
    \value PdfDocument.Loading The status after load() has been called and before the document is fully loaded.
    \value PdfDocument.Ready The status when the document is fully loaded and its data can be accessed.
    \value PdfDocument.Unloading The status after close() has been called on an open document.
            At this point the document is still valid and all its data can be accessed.
    \value PdfDocument.Error The status after Loading, if loading has failed.
*/

QT_END_NAMESPACE
