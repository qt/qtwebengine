/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#include "qquickpdfdocument_p.h"
#include <QtCore/qstandardpaths.h>
#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlengine.h>
#include <QtQuick/qquickitem.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype PdfDocument
//!    \instantiates QQuickPdfDocument
    \inqmlmodule QtQuick.Pdf
    \ingroup pdf
    \brief A representation of a PDF document.
    \since 5.15

    PdfDocument provides access to PDF document meta-information.
    It is not necessary for rendering, as it is enough to use an
    \l Image with source set to the URL of the PDF.
*/

/*
    Constructs a PDF document.
*/
QQuickPdfDocument::QQuickPdfDocument(QObject *parent)
    : QObject(parent)
{
    connect(&m_doc, &QPdfDocument::passwordChanged, this, &QQuickPdfDocument::passwordChanged);
    connect(&m_doc, &QPdfDocument::passwordRequired, this, &QQuickPdfDocument::passwordRequired);
    connect(&m_doc, &QPdfDocument::statusChanged, [this] (QPdfDocument::Status status) {
        emit statusChanged();
        if (status == QPdfDocument::Ready)
            emit metaDataChanged();
    });
    connect(&m_doc, &QPdfDocument::pageCountChanged, this, &QQuickPdfDocument::pageCountChanged);
}

/*!
    \internal
*/
QQuickPdfDocument::~QQuickPdfDocument() = default;

void QQuickPdfDocument::componentComplete()
{
    if (m_doc.error() == QPdfDocument::IncorrectPasswordError)
        emit passwordRequired();
}

/*!
    \qmlproperty url PdfDocument::source

    This property holds a URL pointing to the PDF file to be loaded.

    \note At this time, only local filesystem URLs are supported.
*/
void QQuickPdfDocument::setSource(QUrl source)
{
    if (m_source == source)
        return;

    m_source = source;
    m_maxPageWidthHeight = QSizeF();
    emit sourceChanged();
    const QQmlContext *context = qmlContext(this);
    m_resolvedSource = context ? context->resolvedUrl(source) : source;
    if (source.scheme() == QLatin1String("qrc"))
        m_doc.load(QLatin1Char(':') + m_resolvedSource.path());
    else
        m_doc.load(m_resolvedSource.toLocalFile());
}

/*!
    \qmlproperty string PdfDocument::error

    This property holds a translated string representation of the current
    error, if any.

    \sa status
*/
QString QQuickPdfDocument::error() const
{
    switch (m_doc.error()) {
    case QPdfDocument::NoError:
        return tr("no error");
        break;
    case QPdfDocument::UnknownError:
        break;
    case QPdfDocument::DataNotYetAvailableError:
        return tr("data not yet available");
        break;
    case QPdfDocument::FileNotFoundError:
        return tr("file not found");
        break;
    case QPdfDocument::InvalidFileFormatError:
        return tr("invalid file format");
        break;
    case QPdfDocument::IncorrectPasswordError:
        return tr("incorrect password");
        break;
    case QPdfDocument::UnsupportedSecuritySchemeError:
        return tr("unsupported security scheme");
        break;
    }
    return tr("unknown error");
}

/*!
    \qmlproperty string PdfDocument::password

    This property holds the document password. If the passwordRequired()
    signal is emitted, the UI should prompt the user and then set this
    property so that document opening can continue.
*/
void QQuickPdfDocument::setPassword(const QString &password)
{
    if (m_doc.password() == password)
        return;
    m_doc.setPassword(password);
    if (resolvedSource().isValid() && resolvedSource().isLocalFile())
        m_doc.load(resolvedSource().path());
}

/*!
    \qmlproperty int PdfDocument::pageCount

    This property holds the number of pages the PDF contains.
*/

/*!
    \qmlsignal PdfDocument::passwordRequired()

    This signal is emitted when the PDF requires a password in order to open.
    The UI in a typical PDF viewer should prompt the user for the password
    and then set the password property when the user has provided it.
*/

/*!
    \qmlmethod size PdfDocument::pagePointSize(int page)

    Returns the size of the given \a page in points.
*/
QSizeF QQuickPdfDocument::pagePointSize(int page) const
{
    return m_doc.pageSize(page);
}

qreal QQuickPdfDocument::maxPageWidth() const
{
    const_cast<QQuickPdfDocument *>(this)->updateMaxPageSize();
    return m_maxPageWidthHeight.width();
}

qreal QQuickPdfDocument::maxPageHeight() const
{
    const_cast<QQuickPdfDocument *>(this)->updateMaxPageSize();
    return m_maxPageWidthHeight.height();
}

void QQuickPdfDocument::updateMaxPageSize()
{
    if (m_maxPageWidthHeight.isValid())
        return;
    qreal w = 0;
    qreal h = 0;
    const int count = pageCount();
    for (int i = 0; i < count; ++i) {
        auto size = pagePointSize(i);
        w = qMax(w, size.width());
        h = qMax(w, size.height());
    }
    m_maxPageWidthHeight = QSizeF(w, h);
}

/*!
    \qmlproperty real PdfDocument::maxPageWidth

    This property holds the width of the widest page in the document, in points.
*/

/*!
    \qmlproperty real PdfDocument::maxPageHeight

    This property holds the height of the tallest page in the document, in points.
*/

/*!
    \qmlproperty string PdfDocument::title

    This property holds the document's title. A typical viewer UI can bind this
    to the \c Window.title property.
*/

/*!
    \qmlproperty string PdfDocument::author

    This property holds the name of the person who created the document.
*/

/*!
    \qmlproperty string PdfDocument::subject

    This property holds the subject of the document.
*/

/*!
    \qmlproperty string PdfDocument::keywords

    This property holds the keywords associated with the document.
*/

/*!
    \qmlproperty string PdfDocument::creator

    If the document was converted to PDF from another format, this property
    holds the name of the software that created the original document.
*/

/*!
    \qmlproperty string PdfDocument::producer

    If the document was converted to PDF from another format, this property
    holds the name of the software that converted it to PDF.
*/

/*!
    \qmlproperty date PdfDocument::creationDate

    This property holds the date and time the document was created.
*/

/*!
    \qmlproperty date PdfDocument::modificationDate

    This property holds the date and time the document was most recently
    modified.
*/

/*!
    \qmlproperty enum PdfDocument::status

    This property tells the current status of the document. The possible values are:

    \value PdfDocument.Null The initial status after the document has been created or after it has been closed.
    \value PdfDocument.Loading The status after load() has been called and before the document is fully loaded.
    \value PdfDocument.Ready The status when the document is fully loaded and its data can be accessed.
    \value PdfDocument.Unloading The status after close() has been called on an open document.
            At this point the document is still valid and all its data can be accessed.
    \value PdfDocument.Error The status after Loading, if loading has failed.
*/

QT_END_NAMESPACE
