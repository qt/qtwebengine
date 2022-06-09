// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINECLIENTCERTSELECTION_H
#define QWEBENGINECLIENTCERTSELECTION_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtNetwork/qtnetwork-config.h>
#include <QtCore/qlist.h>
#include <QtCore/qscopedpointer.h>
#include <QtNetwork/qsslcertificate.h>

namespace QtWebEngineCore {
class ClientCertSelectController;
}

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_EXPORT QWebEngineClientCertificateSelection
{
public:
    QWebEngineClientCertificateSelection(const QWebEngineClientCertificateSelection &);
    ~QWebEngineClientCertificateSelection();

    QWebEngineClientCertificateSelection &operator=(const QWebEngineClientCertificateSelection &);

    QUrl host() const;

    void select(const QSslCertificate &certificate);
    void selectNone();
    QList<QSslCertificate> certificates() const;

private:
    friend class QWebEnginePagePrivate;

    QWebEngineClientCertificateSelection(
            QSharedPointer<QtWebEngineCore::ClientCertSelectController>);

    QSharedPointer<QtWebEngineCore::ClientCertSelectController> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBENGINECLIENTCERTSELECTION_H
