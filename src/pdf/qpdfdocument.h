// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFDOCUMENT_H
#define QPDFDOCUMENT_H

#include <QtPdf/qtpdfglobal.h>

#include <QtCore/qobject.h>
#include <QtCore/QAbstractListModel>
#include <QtGui/qimage.h>
#include <QtPdf/qpdfdocumentrenderoptions.h>
#include <QtPdf/qpdfselection.h>

QT_BEGIN_NAMESPACE

class QPdfDocumentPrivate;
class QNetworkReply;

class Q_PDF_EXPORT QPdfDocument : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged FINAL)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged FINAL)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged FINAL)
    Q_PROPERTY(QAbstractListModel* pageModel READ pageModel NOTIFY pageModelChanged FINAL)

public:
    enum class Status {
        Null,
        Loading,
        Ready,
        Unloading,
        Error
    };
    Q_ENUM(Status)

    enum class Error {
        None,
        Unknown,
        DataNotYetAvailable,
        FileNotFound,
        InvalidFileFormat,
        IncorrectPassword,
        UnsupportedSecurityScheme
    };
    Q_ENUM(Error)

    enum class MetaDataField {
        Title,
        Subject,
        Author,
        Keywords,
        Producer,
        Creator,
        CreationDate,
        ModificationDate
    };
    Q_ENUM(MetaDataField)

    enum class PageModelRole {
        Label = Qt::UserRole,
        PointSize,
        NRoles
    };
    Q_ENUM(PageModelRole)

    QPdfDocument() : QPdfDocument(nullptr) {}
    explicit QPdfDocument(QObject *parent);
    ~QPdfDocument() override;

    Error load(const QString &fileName);

    Status status() const;

    void load(QIODevice *device);
    void setPassword(const QString &password);
    QString password() const;

    QVariant metaData(MetaDataField field) const;

    Error error() const;

    void close();

    int pageCount() const;

    Q_INVOKABLE QSizeF pagePointSize(int page) const;

    Q_INVOKABLE QString pageLabel(int page);
    Q_INVOKABLE int pageIndexForLabel(const QString &label);

    QAbstractListModel *pageModel();

    QImage render(int page, QSize imageSize, QPdfDocumentRenderOptions options = QPdfDocumentRenderOptions());

    Q_INVOKABLE QPdfSelection getSelection(int page, QPointF start, QPointF end);
    Q_INVOKABLE QPdfSelection getSelectionAtIndex(int page, int startIndex, int maxLength);
    Q_INVOKABLE QPdfSelection getAllText(int page);

Q_SIGNALS:
    void passwordChanged();
    void passwordRequired();
    void statusChanged(QPdfDocument::Status status);
    void pageCountChanged(int pageCount);
    void pageModelChanged();

private:
    friend struct QPdfBookmarkModelPrivate;
    friend class QPdfFile;
    friend class QPdfLinkModelPrivate;
    friend class QPdfPageModel;
    friend class QPdfSearchModel;
    friend class QPdfSearchModelPrivate;
    friend class QQuickPdfSelection;

    QString fileName() const;

    Q_PRIVATE_SLOT(d, void _q_tryLoadingWithSizeFromContentHeader())
    Q_PRIVATE_SLOT(d, void _q_copyFromSequentialSourceDevice())
    QScopedPointer<QPdfDocumentPrivate> d;
};

QT_END_NAMESPACE

#endif // QPDFDOCUMENT_H
