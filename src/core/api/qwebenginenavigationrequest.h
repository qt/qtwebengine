// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
