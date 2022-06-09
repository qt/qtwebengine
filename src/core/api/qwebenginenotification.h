// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINENOTIFICATION_H
#define QWEBENGINENOTIFICATION_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qurl.h>

namespace QtWebEngineCore {
class UserNotificationController;
}

QT_BEGIN_NAMESPACE

class QWebEngineNotificationPrivate;

class Q_WEBENGINECORE_EXPORT QWebEngineNotification : public QObject
{
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
