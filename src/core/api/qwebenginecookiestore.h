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
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWEBENGINECOOKIESTORE_H
#define QWEBENGINECOOKIESTORE_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtWebEngineCore/qwebenginecallback.h>

#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qurl.h>
#include <QtNetwork/qnetworkcookie.h>

namespace QtWebEngineCore {
class BrowserContextAdapter;
class CookieMonsterDelegateQt;
}

QT_BEGIN_NAMESPACE

class QWebEngineCookieStorePrivate;
class QWEBENGINE_EXPORT QWebEngineCookieStore : public QObject {
    Q_OBJECT

public:
    virtual ~QWebEngineCookieStore();

    void setCookie(const QNetworkCookie &cookie, const QUrl &origin = QUrl());
    void deleteCookie(const QNetworkCookie &cookie, const QUrl &origin = QUrl());
    void deleteSessionCookies();
    void deleteAllCookies();
    void loadAllCookies();

Q_SIGNALS:
    void cookieAdded(const QNetworkCookie &cookie);
    void cookieRemoved(const QNetworkCookie &cookie);

private:
    explicit QWebEngineCookieStore(QObject *parent = 0);
    friend class QtWebEngineCore::BrowserContextAdapter;
    friend class QtWebEngineCore::CookieMonsterDelegateQt;
    Q_DISABLE_COPY(QWebEngineCookieStore)
    Q_DECLARE_PRIVATE(QWebEngineCookieStore)
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QWebEngineCookieStore*)

#endif // QWEBENGINECOOKIESTORE_H
