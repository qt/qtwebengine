// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#include <QtTest/QtTest>

#include <qwebenginepage.h>
#include <qwebengineview.h>
#include <QDebug>

class tst_Shutdown : public QObject
{
    Q_OBJECT

public:
    tst_Shutdown();
    virtual ~tst_Shutdown();

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void dummyTest();

private:


private:
    QWebEngineView* m_view;
    QWebEnginePage* m_page;
};

tst_Shutdown::tst_Shutdown()
{
}

tst_Shutdown::~tst_Shutdown()
{
}

void tst_Shutdown::init()
{
    m_view = new QWebEngineView();
    m_page = m_view->page();
}

void tst_Shutdown::cleanup()
{
    delete m_view;
}

void tst_Shutdown::dummyTest()
{
    QVERIFY(m_view);
}

QTEST_MAIN(tst_Shutdown)
#include "tst_shutdown.moc"
