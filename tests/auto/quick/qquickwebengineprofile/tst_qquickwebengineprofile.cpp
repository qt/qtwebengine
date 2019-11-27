/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include <QtQml/QQmlEngine>
#include <QtTest/QtTest>
#include <QtWebEngine/QQuickWebEngineProfile>

class tst_QQuickWebEngineProfile : public QObject {
    Q_OBJECT
public:
    tst_QQuickWebEngineProfile();

    // TODO: Many tests missings
    void usedForGlobalCertificateVerification();

private Q_SLOTS:
    void init();
    void cleanup();
};

tst_QQuickWebEngineProfile::tst_QQuickWebEngineProfile()
{
    QtWebEngine::initialize();
    QQuickWebEngineProfile::defaultProfile()->setOffTheRecord(true);
}


void tst_QQuickWebEngineProfile::init()
{
}

void tst_QQuickWebEngineProfile::cleanup()
{
}

void tst_QQuickWebEngineProfile::usedForGlobalCertificateVerification()
{
    QQuickWebEngineProfile *profile1 = new QQuickWebEngineProfile();
    QQuickWebEngineProfile *profile2 = new QQuickWebEngineProfile();
    QVERIFY(!profile1->isUsedForGlobalVerification());
    QVERIFY(!profile2->isUsedForGlobalVerification());


}


QTEST_MAIN(tst_QQuickWebEngineProfile)
#include "tst_qquickwebengineprofile.moc"
