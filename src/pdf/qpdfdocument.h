/******************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt PDF Module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#ifndef QPDFDOCUMENT_H
#define QPDFDOCUMENT_H

#include <QObject>
#include <QImage>
#include "qtpdfglobal.h"

QT_BEGIN_NAMESPACE

class QPdfDocumentPrivate;
class QNetworkReply;

class Q_PDF_EXPORT QPdfDocument : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged FINAL)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged FINAL)
    Q_PROPERTY(bool loading READ isLoading FINAL)

public:

    enum Error {
        NoError,
        UnknownError,
        FileNotFoundError,
        InvalidFileFormatError,
        IncorrectPasswordError,
        UnsupportedSecuritySchemeError
    };

    explicit QPdfDocument(QObject *parent = Q_NULLPTR);
    ~QPdfDocument();

    Error load(const QString &fileName);

    bool isLoading() const;

    void load(QIODevice *device);
    void setPassword(const QString &password);
    QString password() const;

    Error error() const;

    void close();

    int pageCount() const;

    QSizeF pageSize(int page) const;

    QImage render(int page, const QSizeF &pageSize);

Q_SIGNALS:
    void passwordChanged();
    void passwordRequired();
    void documentLoadStarted();
    void documentLoadFinished();
    void aboutToBeClosed();
    void pageCountChanged();

private:
    Q_PRIVATE_SLOT(d, void _q_tryLoadingWithSizeFromContentHeader())
    Q_PRIVATE_SLOT(d, void _q_copyFromSequentialSourceDevice())
    QScopedPointer<QPdfDocumentPrivate> d;
};

QT_END_NAMESPACE

#endif // QPDFDOCUMENT_H
