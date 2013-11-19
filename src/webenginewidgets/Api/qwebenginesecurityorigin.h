/*
    Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QWEBENGINESECURITYORIGIN_H_
#define QWEBENGINESECURITYORIGIN_H_

#include <QtWebEngineWidgets/qtwebenginewidgetsglobal.h>

QT_BEGIN_NAMESPACE
class QWebEngineDatabase;
class QWebEngineSecurityOriginPrivate;

class QWEBENGINEWIDGETS_EXPORT QWebEngineSecurityOrigin {
public:
    static QList<QWebEngineSecurityOrigin> allOrigins();
    static void addLocalScheme(const QString& scheme);
    static void removeLocalScheme(const QString& scheme);
    static QStringList localSchemes();

    ~QWebEngineSecurityOrigin();

    QString scheme() const;
    QString host() const;
    int port() const;

    qint64 databaseUsage() const;
    qint64 databaseQuota() const;
    void setDatabaseQuota(qint64 quota);
    void setApplicationCacheQuota(qint64 quota);
    QList<QWebEngineDatabase> databases() const;
};

QT_END_NAMESPACE

#endif // QWEBENGINESECURITYORIGIN_H_
