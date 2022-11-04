// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINEACTION_P_H
#define QQUICKWEBENGINEACTION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qtwebenginequickglobal_p.h"
#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>
#include <QtQml/qqml.h>
#include <QtQml/qqmlregistration.h>

namespace QtWebEngineCore {
    class UIDelegatesManager;
}

QT_BEGIN_NAMESPACE

class QQuickWebEngineActionPrivate;

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineAction : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text CONSTANT FINAL)
    Q_PROPERTY(QString iconName READ iconName CONSTANT FINAL)
    Q_PROPERTY(bool enabled READ isEnabled NOTIFY enabledChanged FINAL)
    QML_NAMED_ELEMENT(WebEngineAction)
    QML_ADDED_IN_VERSION(1, 8)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")

public:
    QQuickWebEngineAction(const QVariant &data, const QString &text, const QString &iconName, bool enabled, QObject *parent);
    QQuickWebEngineAction(QObject *parent);
    ~QQuickWebEngineAction();

    QString text() const;
    QString iconName() const;
    bool isEnabled() const;

public Q_SLOTS:
    Q_INVOKABLE void trigger();

Q_SIGNALS:
    void triggered();
    void enabledChanged();

private:
    Q_DECLARE_PRIVATE(QQuickWebEngineAction)
    friend class QQuickWebEngineViewPrivate;
    friend class QQuickContextMenuBuilder;

    QScopedPointer<QQuickWebEngineActionPrivate> d_ptr;
};


QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickWebEngineAction)

#endif // QQUICKWEBENGINEACTION_P_H
