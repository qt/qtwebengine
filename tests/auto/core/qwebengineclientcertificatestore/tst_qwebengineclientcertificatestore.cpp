// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebengineclientcertificatestore.h>
#include <QtWebEngineCore/qwebengineprofile.h>

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
    QWebEngineProfile::defaultProfile()->clientCertificateStore()->add(cert, sslKey);
    QWebEngineProfile::defaultProfile()->clientCertificateStore()->add(certSecond, sslKeySecond);

    QCOMPARE(2, QWebEngineProfile::defaultProfile()->clientCertificateStore()->certificates().size());
}

void tst_QWebEngineClientCertificateStore::removeAndClearCertificates()
{
    QCOMPARE(2, QWebEngineProfile::defaultProfile()->clientCertificateStore()->certificates().size());

    // Remove one certificate from in-memory store
    auto list = QWebEngineProfile::defaultProfile()->clientCertificateStore()->certificates();
    QWebEngineProfile::defaultProfile()->clientCertificateStore()->remove(list[0]);
    QCOMPARE(1, QWebEngineProfile::defaultProfile()->clientCertificateStore()->certificates().size());

    // Remove all certificates in-memory store
    QWebEngineProfile::defaultProfile()->clientCertificateStore()->clear();
    QCOMPARE(0, QWebEngineProfile::defaultProfile()->clientCertificateStore()->certificates().size());
}

QTEST_MAIN(tst_QWebEngineClientCertificateStore)
#include "tst_qwebengineclientcertificatestore.moc"
