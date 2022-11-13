// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKPDFDOCUMENT_P_H
#define QQUICKPDFDOCUMENT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtPdfQuick/private/qtpdfquickglobal_p.h>
#include <QtPdf/QPdfDocument>

#include <QtQml/QQmlEngine>
#include <QtQml/QQmlParserStatus>
#include <QtCore/QDateTime>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

class QPdfFile;

class Q_PDFQUICK_EXPORT QQuickPdfDocument : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(qreal maxPageWidth READ maxPageWidth NOTIFY metaDataChanged FINAL)
    Q_PROPERTY(qreal maxPageHeight READ maxPageHeight NOTIFY metaDataChanged FINAL)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged FINAL)

    Q_PROPERTY(QString title READ title NOTIFY metaDataChanged FINAL)
    Q_PROPERTY(QString subject READ subject NOTIFY metaDataChanged FINAL)
    Q_PROPERTY(QString author READ author NOTIFY metaDataChanged FINAL)
    Q_PROPERTY(QString keywords READ keywords NOTIFY metaDataChanged FINAL)
    Q_PROPERTY(QString producer READ producer NOTIFY metaDataChanged FINAL)
    Q_PROPERTY(QString creator READ creator NOTIFY metaDataChanged FINAL)
    Q_PROPERTY(QDateTime creationDate READ creationDate NOTIFY metaDataChanged FINAL)
    Q_PROPERTY(QDateTime modificationDate READ modificationDate NOTIFY metaDataChanged FINAL)
    QML_NAMED_ELEMENT(PdfDocument)
    QML_EXTENDED(QPdfDocument)
    QML_ADDED_IN_VERSION(5, 15)

public:
    explicit QQuickPdfDocument(QObject *parent = nullptr);
    ~QQuickPdfDocument() override;

    void classBegin() override;
    void componentComplete() override {}

    QUrl source() const { return m_source; }
    void setSource(QUrl source);
    QUrl resolvedSource() const { return m_resolvedSource; }

    QString error() const;

    QString title() { return m_doc->metaData(QPdfDocument::MetaDataField::Title).toString(); }
    QString author() { return m_doc->metaData(QPdfDocument::MetaDataField::Author).toString(); }
    QString subject() { return m_doc->metaData(QPdfDocument::MetaDataField::Subject).toString(); }
    QString keywords() { return m_doc->metaData(QPdfDocument::MetaDataField::Keywords).toString(); }
    QString producer() { return m_doc->metaData(QPdfDocument::MetaDataField::Producer).toString(); }
    QString creator() { return m_doc->metaData(QPdfDocument::MetaDataField::Creator).toString(); }
    QDateTime creationDate() { return m_doc->metaData(QPdfDocument::MetaDataField::CreationDate).toDateTime(); }
    QDateTime modificationDate() { return m_doc->metaData(QPdfDocument::MetaDataField::ModificationDate).toDateTime(); }

    qreal maxPageWidth() const;
    qreal maxPageHeight() const;

Q_SIGNALS:
    void sourceChanged();
    void errorChanged();
    void metaDataChanged();

private:
    QPdfDocument *document() const;
    QPdfFile *carrierFile();
    void updateMaxPageSize() const;

private:
    QUrl m_source;
    QUrl m_resolvedSource;
    QPdfDocument *m_doc = nullptr;
    QPdfFile *m_carrierFile = nullptr;
    mutable QSizeF m_maxPageWidthHeight;

    friend class QQuickPdfBookmarkModel;
    friend class QQuickPdfLinkModel;
    friend class QQuickPdfPageImage;
    friend class QQuickPdfSearchModel;
    friend class QQuickPdfSelection;

    Q_DISABLE_COPY(QQuickPdfDocument)
};

QT_END_NAMESPACE

#endif // QQUICKPDFDOCUMENT_P_H
