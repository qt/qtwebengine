/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWEBENGINENAVIGATIONREQUEST_H
#define QWEBENGINENAVIGATIONREQUEST_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

class QWebEngineNavigationRequestPrivate;

class Q_WEBENGINECORE_EXPORT QWebEngineNavigationRequest : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url CONSTANT FINAL)
    Q_PROPERTY(bool isMainFrame READ isMainFrame CONSTANT FINAL)
    Q_PROPERTY(NavigationType navigationType READ navigationType CONSTANT FINAL)

public:
    ~QWebEngineNavigationRequest();

    // must match WebContentsAdapterClient::NavigationType
    enum NavigationType {
        LinkClickedNavigation,
        TypedNavigation,
        FormSubmittedNavigation,
        BackForwardNavigation,
        ReloadNavigation,
        OtherNavigation,
        RedirectNavigation,
    };
    Q_ENUM(NavigationType)

    QUrl url() const;
    bool isMainFrame() const;
    NavigationType navigationType() const;

    Q_INVOKABLE void accept();
    Q_INVOKABLE void reject();

#if QT_DEPRECATED_SINCE(6, 2)
    enum NavigationRequestAction {
        AcceptRequest,
        IgnoreRequest = 0xFF
    };
    Q_ENUM(NavigationRequestAction)

private:
    Q_PROPERTY(NavigationRequestAction action READ action WRITE setAction NOTIFY actionChanged FINAL)

    QT_DEPRECATED NavigationRequestAction action() const;
    QT_DEPRECATED_X("Use accept/reject methods to handle the request")
    void setAction(NavigationRequestAction action);

Q_SIGNALS:
    QT_DEPRECATED void actionChanged();
#endif

private:
    QWebEngineNavigationRequest(const QUrl &url, NavigationType navigationType, bool mainFrame,
                                QObject *parent = nullptr);

    friend class QWebEnginePagePrivate;
    friend class QQuickWebEngineViewPrivate;
    bool isAccepted() const;

    Q_DECLARE_PRIVATE(QWebEngineNavigationRequest)
    QScopedPointer<QWebEngineNavigationRequestPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBENGINENAVIGATIONREQUEST_H
