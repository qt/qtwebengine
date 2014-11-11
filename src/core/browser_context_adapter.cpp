/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "browser_context_adapter.h"

#include "browser_context_qt.h"
#include "web_engine_context.h"
#include "web_engine_visited_links_manager.h"

#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QStringBuilder>
#include <QStandardPaths>

namespace {
inline QString buildLocationFromStandardPath(const QString &standardPath) {
    QString location = standardPath;
    if (location.isEmpty())
        location = QDir::homePath() % QDir::separator() % QChar::fromLatin1('.') % QCoreApplication::applicationName();

    location.append(QDir::separator() % QLatin1String("QtWebEngine")
                         % QDir::separator() % QLatin1String("Default"));
    return location;
}
}

BrowserContextAdapter::BrowserContextAdapter(bool offTheRecord)
    : m_offTheRecord(offTheRecord)
    , m_browserContext(new BrowserContextQt(this))
    , m_visitedLinksManager(new WebEngineVisitedLinksManager(this))
{
}

BrowserContextAdapter::~BrowserContextAdapter()
{
}

BrowserContextQt *BrowserContextAdapter::browserContext()
{
    return m_browserContext.data();
}

WebEngineVisitedLinksManager *BrowserContextAdapter::visitedLinksManager()
{
    return m_visitedLinksManager.data();
}

BrowserContextAdapter* BrowserContextAdapter::defaultContext()
{
    return WebEngineContext::current()->defaultBrowserContext();
}

BrowserContextAdapter* BrowserContextAdapter::offTheRecordContext()
{
    return WebEngineContext::current()->offTheRecordBrowserContext();
}

QString BrowserContextAdapter::dataPath() const
{
    return buildLocationFromStandardPath(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
}

QString BrowserContextAdapter::cachePath() const
{
    return buildLocationFromStandardPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
}
