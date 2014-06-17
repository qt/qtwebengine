/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQUICKWEBENGINELOADREQUEST_P_H
#define QQUICKWEBENGINELOADREQUEST_P_H

#include "qtwebengineglobal_p.h"
#include "qquickwebengineview_p.h"

QT_BEGIN_NAMESPACE

class QQuickWebEngineLoadRequestPrivate;

class Q_WEBENGINE_EXPORT QQuickWebEngineLoadRequest : public QObject {
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url)
    Q_PROPERTY(QQuickWebEngineView::LoadStatus status READ status)
    Q_PROPERTY(QString errorString READ errorString)
    Q_PROPERTY(QQuickWebEngineView::ErrorDomain errorDomain READ errorDomain)
    Q_PROPERTY(int errorCode READ errorCode)
    Q_PROPERTY(QString errorName READ errorName)

public:
    QQuickWebEngineLoadRequest(const QUrl& url, QQuickWebEngineView::LoadStatus status, const QString& errorString = QString(), int errorCode = 0, QQuickWebEngineView::ErrorDomain errorDomain = QQuickWebEngineView::NoErrorDomain
            , const QString &errorName = QString(), QObject* parent = 0);
    ~QQuickWebEngineLoadRequest();
    QUrl url() const;
    QQuickWebEngineView::LoadStatus status() const;
    QString errorString() const;
    QQuickWebEngineView::ErrorDomain errorDomain() const;
    int errorCode() const;
    QString errorName() const;

private:
    QScopedPointer<QQuickWebEngineLoadRequestPrivate> d;
};

class QQuickWebEngineLoadRequestPrivate {
public:
    QQuickWebEngineLoadRequestPrivate(const QUrl& url, QQuickWebEngineView::LoadStatus status, const QString& errorString, int errorCode, QQuickWebEngineView::ErrorDomain errorDomain, const QString &errorName)
        : url(url)
        , status(status)
        , errorString(errorString)
        , errorCode(errorCode)
        , errorDomain(errorDomain)
        , errorName(errorName)
    {
    }

    QUrl url;
    QQuickWebEngineView::LoadStatus status;
    QString errorString;
    int errorCode;
    QQuickWebEngineView::ErrorDomain errorDomain;
    QString errorName;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickWebEngineLoadRequest)

#endif // QQUICKWEBENGINELOADREQUEST_P_H
