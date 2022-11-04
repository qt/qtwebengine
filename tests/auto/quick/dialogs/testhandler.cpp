// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "testhandler.h"

TestHandler::TestHandler(QObject *parent) : QObject(parent)
{
    setObjectName(QStringLiteral("TestListener"));
}

QObject* TestHandler::request() const
{
    return m_request;
}

void TestHandler::setRequest(QObject *request)
{
    if (m_request == request)
        return;

    m_request = request;
    emit requestChanged(m_request);
}

void TestHandler::runJavaScript(const QString &script)
{
    m_ready = false;
    emit javaScript(script);
}

void TestHandler::load(const QUrl &page)
{
    m_ready = false;
    emit loadPage(page);
}

bool TestHandler::ready() const
{
    return m_ready;
}

void TestHandler::setReady(bool ready)
{
    if (m_ready == ready)
        return;

    m_ready = ready;
    emit readyChanged(ready);
}
