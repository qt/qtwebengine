/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef QWEBENGINENOTIFICATION_H
#define QWEBENGINENOTIFICATION_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QSharedPointer>
#include <QtCore/QUrl>

namespace QtWebEngineCore {
class UserNotificationController;
}

QT_BEGIN_NAMESPACE

class QWebEngineNotificationPrivate;

class Q_WEBENGINECORE_EXPORT QWebEngineNotification : public QObject {
    Q_OBJECT
    Q_PROPERTY(QUrl origin READ origin CONSTANT FINAL)
    Q_PROPERTY(QString title READ title CONSTANT FINAL)
    Q_PROPERTY(QString message READ message CONSTANT FINAL)
    Q_PROPERTY(QString tag READ tag CONSTANT FINAL)
    Q_PROPERTY(QString language READ language CONSTANT FINAL)
    Q_PROPERTY(Qt::LayoutDirection direction READ direction CONSTANT FINAL)

public:
    virtual ~QWebEngineNotification() override;

    bool matches(const QWebEngineNotification *other) const;

    QUrl origin() const;
    QImage icon() const;
    QString title() const;
    QString message() const;
    QString tag() const;
    QString language() const;
    Qt::LayoutDirection direction() const;

public Q_SLOTS:
    void show() const;
    void click() const;
    void close() const;

Q_SIGNALS:
    void closed();

private:
    Q_DISABLE_COPY(QWebEngineNotification)
    Q_DECLARE_PRIVATE(QWebEngineNotification)
    QWebEngineNotification(const QSharedPointer<QtWebEngineCore::UserNotificationController> &controller);
    QScopedPointer<QWebEngineNotificationPrivate> d_ptr;
    friend class QQuickWebEngineProfilePrivate;
    friend class QWebEngineProfilePrivate;
};

QT_END_NAMESPACE

#endif // QWEBENGINENOTIFICATION_H
