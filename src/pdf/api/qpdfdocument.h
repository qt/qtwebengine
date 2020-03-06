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

#ifndef QPDFDOCUMENT_H
#define QPDFDOCUMENT_H

#include <QtPdf/qtpdfglobal.h>

#include <QtCore/qobject.h>
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

public:
    enum Status {
        Null,
        Loading,
        Ready,
        Unloading,
        Error
    };
    Q_ENUM(Status)

    enum DocumentError {
        NoError,
        UnknownError,
        DataNotYetAvailableError,
        FileNotFoundError,
        InvalidFileFormatError,
        IncorrectPasswordError,
        UnsupportedSecuritySchemeError
    };
    Q_ENUM(DocumentError)

    enum MetaDataField {
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

    explicit QPdfDocument(QObject *parent = nullptr);
    ~QPdfDocument();

    DocumentError load(const QString &fileName);

    Status status() const;

    void load(QIODevice *device);
    void setPassword(const QString &password);
    QString password() const;

    QVariant metaData(MetaDataField field) const;

    DocumentError error() const;

    void close();

    int pageCount() const;

    QSizeF pageSize(int page) const;

    QImage render(int page, QSize imageSize, QPdfDocumentRenderOptions options = QPdfDocumentRenderOptions());

    Q_INVOKABLE QPdfSelection getSelection(int page, QPointF start, QPointF end);
    Q_INVOKABLE QPdfSelection getSelectionAtIndex(int page, int startIndex, int maxLength);
    Q_INVOKABLE QPdfSelection getAllText(int page);

Q_SIGNALS:
    void passwordChanged();
    void passwordRequired();
    void statusChanged(QPdfDocument::Status status);
    void pageCountChanged(int pageCount);

private:
    friend class QPdfBookmarkModelPrivate;
    friend class QPdfLinkModelPrivate;
    friend class QPdfSearchModel;
    friend class QPdfSearchModelPrivate;
    friend class QQuickPdfSelection;

    Q_PRIVATE_SLOT(d, void _q_tryLoadingWithSizeFromContentHeader())
    Q_PRIVATE_SLOT(d, void _q_copyFromSequentialSourceDevice())
    QScopedPointer<QPdfDocumentPrivate> d;
};

QT_END_NAMESPACE

#endif // QPDFDOCUMENT_H
