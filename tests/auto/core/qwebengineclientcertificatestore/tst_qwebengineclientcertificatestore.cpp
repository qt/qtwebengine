/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebengineclientcertificatestore.h>

class tst_QWebEngineClientCertificateStore : public QObject
{
    Q_OBJECT

public:
    tst_QWebEngineClientCertificateStore();
    ~tst_QWebEngineClientCertificateStore();

private Q_SLOTS:
    void addAndListCertificates();
    void removeAndClearCertificates();
};

tst_QWebEngineClientCertificateStore::tst_QWebEngineClientCertificateStore()
{
}

tst_QWebEngineClientCertificateStore::~tst_QWebEngineClientCertificateStore()
{
}

void tst_QWebEngineClientCertificateStore::addAndListCertificates()
{
    // Load QSslCertificate
    QFile certFile(":/resources/certificate.crt");
    certFile.open(QIODevice::ReadOnly);
    const QSslCertificate cert(certFile.readAll(), QSsl::Pem);

    // Load QSslKey
    QFile keyFile(":/resources/privatekey.key");
    keyFile.open(QIODevice::ReadOnly);
    const QSslKey sslKey(keyFile.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "");

    // Load second QSslCertificate
    QFile certFileSecond(":/resources/certificate1.crt");
    certFileSecond.open(QIODevice::ReadOnly);
    const QSslCertificate certSecond(certFileSecond.readAll(), QSsl::Pem);

    // Load second QSslKey
    QFile keyFileSecond(":/resources/privatekey1.key");
    keyFileSecond.open(QIODevice::ReadOnly);
    const QSslKey sslKeySecond(keyFileSecond.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "");

    // Add certificates to in-memory store
    QWebEngineClientCertificateStore::getInstance()->add(cert, sslKey);
    QWebEngineClientCertificateStore::getInstance()->add(certSecond, sslKeySecond);

    QCOMPARE(2, QWebEngineClientCertificateStore::getInstance()->toList().length());
}

void tst_QWebEngineClientCertificateStore::removeAndClearCertificates()
{
    QCOMPARE(2, QWebEngineClientCertificateStore::getInstance()->toList().length());

    // Remove one certificate from in-memory store
    auto list = QWebEngineClientCertificateStore::getInstance()->toList();
    QWebEngineClientCertificateStore::getInstance()->remove(list[0].certificate);
    QCOMPARE(1, QWebEngineClientCertificateStore::getInstance()->toList().length());

    // Remove all certificates in-memory store
    QWebEngineClientCertificateStore::getInstance()->clear();
    QCOMPARE(0, QWebEngineClientCertificateStore::getInstance()->toList().length());
}

QTEST_MAIN(tst_QWebEngineClientCertificateStore)
#include "tst_qwebengineclientcertificatestore.moc"
