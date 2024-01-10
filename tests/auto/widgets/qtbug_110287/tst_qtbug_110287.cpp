// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QSignalSpy>
#include <QTest>
#include <QWebEngineView>

class tst_qtbug_110287 : public QObject
{
    Q_OBJECT
public:
    tst_qtbug_110287() { }

private slots:
    void getAddrInfo();
};

void tst_qtbug_110287::getAddrInfo()
{
    QNetworkAccessManager nam;
    QSignalSpy namSpy(&nam, &QNetworkAccessManager::finished);

    QString address("http://www.example.com");
    QScopedPointer<QNetworkReply> reply(nam.get(QNetworkRequest(address)));

    if (!namSpy.wait(25000) || reply->error() != QNetworkReply::NoError)
        QSKIP("Couldn't load page from network, skipping test.");

    QWebEngineView view;
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));

    // load() will trigger system DNS resolution that uses getaddrinfo()
    view.load(QUrl(address));
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size() > 0, true, 30000);
    QTRY_COMPARE(loadFinishedSpy[0][0].toBool(), true);
}

#include "tst_qtbug_110287.moc"
QTEST_MAIN(tst_qtbug_110287)
