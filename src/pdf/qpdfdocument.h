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

    QPdfDocument() : QPdfDocument(nullptr) {}
    explicit QPdfDocument(QObject *parent);
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
