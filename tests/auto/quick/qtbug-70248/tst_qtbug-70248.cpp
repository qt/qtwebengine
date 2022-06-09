// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qtwebenginequickglobal.h"
#include <QQuickWebEngineProfile>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTest>
#include <QSignalSpy>

class tst_qtbug_70248: public QObject {
    Q_OBJECT
public:
    tst_qtbug_70248(){}
private slots:
    void test();
};

void tst_qtbug_70248::test()
{
    QtWebEngineQuick::initialize();
    QScopedPointer<QQmlApplicationEngine> engine;
    QQuickWebEngineProfile::defaultProfile()->setOffTheRecord(true);
    engine.reset(new QQmlApplicationEngine());
    engine->load(QUrl(QStringLiteral("qrc:/test.qml")));
    QQuickWindow *widnow = qobject_cast<QQuickWindow*>(engine->rootObjects().first());
    QVERIFY(widnow);
}

#include "tst_qtbug-70248.moc"
QTEST_MAIN(tst_qtbug_70248)

