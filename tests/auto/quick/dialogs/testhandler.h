// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TESTHANDLER_H
#define TESTHANDLER_H

#include <QObject>

class TestHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* request READ request WRITE setRequest NOTIFY requestChanged)
    Q_PROPERTY(bool ready READ ready WRITE setReady NOTIFY readyChanged)
public:
    explicit TestHandler(QObject *parent = nullptr);
    QObject* request() const;

    bool ready() const;
    void setReady(bool ready);
    void setRequest(QObject *request);
    void runJavaScript(const QString &script);
    void load(const QUrl &page);

signals:
    void loadPage(const QUrl &page);
    void javaScript(const QString &script);
    void requestChanged(QObject *request);
    void readyChanged(bool ready);

private:
    QObject *m_request = nullptr;
    bool m_ready = false;
};

#endif // TESTHANDLER_H
