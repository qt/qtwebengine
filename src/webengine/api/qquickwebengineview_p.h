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

#ifndef QQUICKWEBENGINEVIEW_P_H
#define QQUICKWEBENGINEVIEW_P_H

#include <private/qtwebengineglobal_p.h>
#include <QQuickItem>

QT_BEGIN_NAMESPACE

class QQuickWebEngineViewPrivate;
class QQuickWebEngineLoadRequest;

class Q_WEBENGINE_PRIVATE_EXPORT QQuickWebEngineView : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QUrl icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(int loadProgress READ loadProgress NOTIFY loadProgressChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY loadingChanged)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY loadingChanged)
    Q_PROPERTY(bool inspectable READ inspectable WRITE setInspectable)
    Q_ENUMS(LoadStatus);
    Q_ENUMS(ErrorDomain);
    Q_ENUMS(NewViewDestination);
    Q_ENUMS(JavaScriptConsoleMessageLevel);

public:
    QQuickWebEngineView(QQuickItem *parent = 0);
    ~QQuickWebEngineView();

    QUrl url() const;
    void setUrl(const QUrl&);
    QUrl icon() const;
    bool isLoading() const;
    int loadProgress() const;
    QString title() const;
    bool canGoBack() const;
    bool canGoForward() const;
    bool inspectable() const;
    void setInspectable(bool);
    void forceActiveFocus();

    enum LoadStatus {
        LoadStartedStatus,
        LoadStoppedStatus,
        LoadSucceededStatus,
        LoadFailedStatus
    };

    enum ErrorDomain {
         NoErrorDomain,
         InternalErrorDomain,
         ConnectionErrorDomain,
         CertificateErrorDomain,
         HttpErrorDomain,
         FtpErrorDomain,
         DnsErrorDomain
    };

    enum NewViewDestination {
        NewViewInWindow,
        NewViewInTab,
        NewViewInDialog
    };

    // must match WebContentsAdapterClient::JavaScriptConsoleMessageLevel
    enum JavaScriptConsoleMessageLevel {
        Info = 0,
        Warning,
        Error
    };

public Q_SLOTS:
    void loadHtml(const QString &html, const QUrl &baseUrl = QUrl(), const QUrl &unreachableUrl = QUrl());
    void goBack();
    void goForward();
    void reload();
    void stop();

Q_SIGNALS:
    void titleChanged();
    void urlChanged();
    void iconChanged();
    void loadingChanged(QQuickWebEngineLoadRequest *loadRequest);
    void loadProgressChanged();
    void linkHovered(const QUrl &hoveredUrl);
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID);

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
    void itemChange(ItemChange, const ItemChangeData &);

private:
    Q_DECLARE_PRIVATE(QQuickWebEngineView)
    friend class QQuickWebEngineViewExperimental;
    friend class QQuickWebEngineViewExperimentalExtension;
    friend class QQuickWebEngineNewViewRequest;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickWebEngineView)

#endif // QQUICKWEBENGINEVIEW_P_H
