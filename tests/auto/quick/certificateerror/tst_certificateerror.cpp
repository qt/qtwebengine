/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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
#include "testhandler.h"
#include <httpsserver.h>
#include <util.h>
#include <QtWebEngine/private/qquickwebenginecertificateerror_p.h>
#include <QQuickWebEngineProfile>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QtTest/QtTest>

class tst_CertificateError : public QObject
{
    Q_OBJECT
public:
    tst_CertificateError() { }

private Q_SLOTS:
    void initTestCase();
    void handleError_data();
    void handleError();

private:
    QScopedPointer<QQmlApplicationEngine> m_engine;
    QQuickWindow *m_widnow = nullptr;
    TestHandler *m_handler = nullptr;
};

void tst_CertificateError::initTestCase()
{
    QQuickWebEngineProfile::defaultProfile()->setOffTheRecord(true);
    qmlRegisterType<TestHandler>("io.qt.tester", 1, 0, "TestHandler");
    m_engine.reset(new QQmlApplicationEngine());
    m_engine->load(QUrl(QStringLiteral("qrc:/WebView.qml")));
    m_widnow = qobject_cast<QQuickWindow *>(m_engine->rootObjects().first());
    Q_ASSERT(m_widnow);
    m_handler = m_widnow->findChild<TestHandler *>(QStringLiteral("TestListner"));
    Q_ASSERT(m_handler);
}

void tst_CertificateError::handleError_data()
{
    QTest::addColumn<bool>("deferError");
    QTest::addColumn<bool>("acceptCertificate");
    QTest::addColumn<QString>("expectedContent");
    QTest::addRow("Reject") << false << false << QString();
    QTest::addRow("DeferReject") << true << false << QString();
    QTest::addRow("DeferAccept") << true << true << "TEST";
}

void tst_CertificateError::handleError()
{
    HttpsServer server;
    server.setExpectError(true);
    QVERIFY(server.start());

    connect(&server, &HttpsServer::newRequest, [&](HttpReqRep *rr) {
        rr->setResponseBody(QByteArrayLiteral("<html><body>TEST</body></html>"));
        rr->sendResponse();
    });

    QFETCH(bool, deferError);
    QFETCH(bool, acceptCertificate);
    QFETCH(QString, expectedContent);

    QSignalSpy certificateErrorSpy(m_handler, &TestHandler::certificateErrorChanged);
    m_handler->load(server.url());
    QTRY_COMPARE(certificateErrorSpy.count(), 1);
    QQuickWebEngineCertificateError *error = m_handler->certificateError();
    QVERIFY(error);

    if (deferError) {
        error->defer();
        return;
    }

    if (acceptCertificate)
        error->ignoreCertificateError();
    else
        error->rejectCertificate();

    QVERIFY(error->overridable());
}

static QByteArrayList params;
W_QTEST_MAIN(tst_CertificateError, params)
#include <tst_certificateerror.moc>
