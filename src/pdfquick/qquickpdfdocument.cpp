// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickpdfdocument_p.h"
#include <private/qpdffile_p.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qstandardpaths.h>
#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlengine.h>
#include <QtQuick/qquickitem.h>
#include <QtQml/qqmlfile.h>

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
}

/*!
    \internal
*/
QQuickPdfDocument::~QQuickPdfDocument()
{
    delete m_carrierFile;
};

void QQuickPdfDocument::classBegin()
{
    m_doc = static_cast<QPdfDocument *>(qmlExtendedObject(this));
    Q_ASSERT(m_doc);
    connect(m_doc, &QPdfDocument::passwordChanged, this, [this]() -> void {
        if (resolvedSource().isValid())
            m_doc->load(QQmlFile::urlToLocalFileOrQrc(resolvedSource()));
    });
    connect(m_doc, &QPdfDocument::statusChanged, this, [this] (QPdfDocument::Status status) {
        emit errorChanged();
        if (status == QPdfDocument::Status::Ready)
            emit metaDataChanged();
    });
    if (m_doc->error() == QPdfDocument::Error::IncorrectPassword)
        emit m_doc->passwordRequired();
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
    m_carrierFile->deleteLater();
    m_carrierFile = nullptr;
    emit sourceChanged();
    const QQmlContext *context = qmlContext(this);
    m_resolvedSource = context ? context->resolvedUrl(source) : source;
    if (m_resolvedSource.isValid())
        m_doc->load(QQmlFile::urlToLocalFileOrQrc(m_resolvedSource));
}

/*!
    \qmlproperty string PdfDocument::error

    This property holds a translated string representation of the current
    error, if any.

    \sa status
*/
QString QQuickPdfDocument::error() const
{
    switch (m_doc->error()) {
    case QPdfDocument::Error::None:
        return tr("no error");
        break;
    case QPdfDocument::Error::Unknown:
        break;
    case QPdfDocument::Error::DataNotYetAvailable:
        return tr("data not yet available");
        break;
    case QPdfDocument::Error::FileNotFound:
        return tr("file not found");
        break;
    case QPdfDocument::Error::InvalidFileFormat:
        return tr("invalid file format");
        break;
    case QPdfDocument::Error::IncorrectPassword:
        return tr("incorrect password");
        break;
    case QPdfDocument::Error::UnsupportedSecurityScheme:
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

qreal QQuickPdfDocument::maxPageWidth() const
{
    updateMaxPageSize();
    return m_maxPageWidthHeight.width();
}

qreal QQuickPdfDocument::maxPageHeight() const
{
    updateMaxPageSize();
    return m_maxPageWidthHeight.height();
}

QPdfDocument *QQuickPdfDocument::document() const
{
    return m_doc;
}

/*!
    \internal
    Returns a QPdfFile instance that can carry this document down into
    QPdfIOHandler::load(QIODevice *). It should not be used for other purposes.
*/
QPdfFile *QQuickPdfDocument::carrierFile()
{
    if (!m_carrierFile)
        m_carrierFile = new QPdfFile(m_doc);
    return m_carrierFile;
}

void QQuickPdfDocument::updateMaxPageSize() const
{
    if (m_maxPageWidthHeight.isValid())
        return;
    qreal w = 0;
    qreal h = 0;
    const int count = m_doc->pageCount();
    for (int i = 0; i < count; ++i) {
        auto size = m_doc->pagePointSize(i);
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

#include "moc_qquickpdfdocument_p.cpp"
