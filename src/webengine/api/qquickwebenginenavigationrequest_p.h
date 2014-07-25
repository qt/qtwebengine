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

#ifndef QQUICKWEBENGINENAVIGATIONREQUEST_P_H
#define QQUICKWEBENGINENAVIGATIONREQUEST_P_H

#include "qtwebengineglobal_p.h"
#include "qquickwebengineview_p.h"

#include <QtCore/QObject>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

class QQuickWebEngineNavigationRequestPrivate;

class Q_WEBENGINE_EXPORT QQuickWebEngineNavigationRequest : public QObject {
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url CONSTANT FINAL)
    Q_PROPERTY(QQuickWebEngineView::NavigationRequestAction action READ action WRITE setAction NOTIFY actionChanged FINAL)
    Q_PROPERTY(QQuickWebEngineView::NavigationType navigationType READ navigationType CONSTANT FINAL)

public:
    QQuickWebEngineNavigationRequest(const QUrl& url, QQuickWebEngineView::NavigationType navigationType, QObject* parent = 0);
    ~QQuickWebEngineNavigationRequest();

    QUrl url() const;
    QQuickWebEngineView::NavigationRequestAction action() const;

    void setAction(QQuickWebEngineView::NavigationRequestAction action);
    QQuickWebEngineView::NavigationType navigationType() const;

public Q_SLOTS:
    void accept();
    void ignore();
    void redirect(const QUrl &url);

Q_SIGNALS:
    void actionChanged();

private:
    QScopedPointer<QQuickWebEngineNavigationRequestPrivate> d;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickWebEngineNavigationRequest)

#endif // QQUICKWEBENGINENAVIGATIONREQUEST_P_H
