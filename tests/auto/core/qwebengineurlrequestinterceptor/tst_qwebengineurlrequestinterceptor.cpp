/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
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
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "../../widgets/util.h"
#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebengineurlrequestinterceptor.h>
#include <QtWebEngineWidgets/qwebenginepage.h>
#include <QtWebEngineWidgets/qwebengineprofile.h>
#include <QtWebEngineWidgets/qwebengineview.h>

class tst_QWebEngineUrlRequestInterceptor : public QObject
{
    Q_OBJECT

public:
    tst_QWebEngineUrlRequestInterceptor();
    ~tst_QWebEngineUrlRequestInterceptor();

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void interceptRequest();
};

tst_QWebEngineUrlRequestInterceptor::tst_QWebEngineUrlRequestInterceptor()
{
}

tst_QWebEngineUrlRequestInterceptor::~tst_QWebEngineUrlRequestInterceptor()
{
}

void tst_QWebEngineUrlRequestInterceptor::init()
{
}

void tst_QWebEngineUrlRequestInterceptor::cleanup()
{
}

void tst_QWebEngineUrlRequestInterceptor::initTestCase()
{
}

void tst_QWebEngineUrlRequestInterceptor::cleanupTestCase()
{
}

class TestRequestInterceptor: public QWebEngineUrlRequestInterceptor
{
public:
    QList<QUrl> observedUrls;
    bool shouldIntercept;

    bool interceptRequest(QWebEngineUrlRequestInfo &info) override
    {
        info.blockRequest(info.method() != QByteArrayLiteral("GET"));
        if (info.url().toString().endsWith(QLatin1String("__placeholder__")))
            info.redirectTo(QUrl("qrc:///resources/content.html"));

        observedUrls.append(info.url());
        return shouldIntercept;
    }
    TestRequestInterceptor(bool intercept)
        : shouldIntercept(intercept)
    {
    }
};

void tst_QWebEngineUrlRequestInterceptor::interceptRequest()
{
    QWebEngineView view;
    TestRequestInterceptor interceptor(/* intercept */ true);

    QSignalSpy loadSpy(&view, SIGNAL(loadFinished(bool)));
    view.page()->profile()->setRequestInterceptor(&interceptor);
    view.load(QUrl("qrc:///resources/index.html"));
    QTRY_COMPARE(loadSpy.count(), 1);
    QVariant success = loadSpy.takeFirst().takeFirst();
    QVERIFY(success.toBool());
    loadSpy.clear();
    QVariant ok;

    view.page()->runJavaScript("post();", [&ok](const QVariant result){ ok = result; });
    QTRY_VERIFY(ok.toBool());
    QTRY_COMPARE(loadSpy.count(), 1);
    success = loadSpy.takeFirst().takeFirst();
    // We block non-GET requests, so this should not succeed.
    QVERIFY(!success.toBool());
    loadSpy.clear();

    view.load(QUrl("qrc:///resources/__placeholder__"));
    QTRY_COMPARE(loadSpy.count(), 1);
    success = loadSpy.takeFirst().takeFirst();
    // The redirection for __placeholder__ should succeed.
    QVERIFY(success.toBool());
    loadSpy.clear();
    QCOMPARE(interceptor.observedUrls.count(), 4);


    // Make sure that registering an observer does not modify the request.
    TestRequestInterceptor observer(/* intercept */ false);
    view.page()->profile()->setRequestInterceptor(&observer);
    view.load(QUrl("qrc:///resources/__placeholder__"));
    QTRY_COMPARE(loadSpy.count(), 1);
    success = loadSpy.takeFirst().takeFirst();
    // Since we do not intercept, loading an invalid path should not succeed.
    QVERIFY(!success.toBool());
    QCOMPARE(observer.observedUrls.count(), 1);
}

QTEST_MAIN(tst_QWebEngineUrlRequestInterceptor)
#include "tst_qwebengineurlrequestinterceptor.moc"
