// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINE_ACCESSIBLE_H
#define QQUICKWEBENGINE_ACCESSIBLE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qpointer.h>
#include <QtGui/qaccessibleobject.h>

QT_BEGIN_NAMESPACE
class QQuickWebEngineView;

class QQuickWebEngineViewAccessible : public QAccessibleObject
{
public:
    QQuickWebEngineViewAccessible(QQuickWebEngineView *o);
    bool isValid() const override;
    QAccessibleInterface *parent() const override;
    QAccessibleInterface *focusChild() const override;
    int childCount() const override;
    QAccessibleInterface *child(int index) const override;
    int indexOfChild(const QAccessibleInterface *) const override;
    QString text(QAccessible::Text) const override;
    QAccessible::Role role() const override;
    QAccessible::State state() const override;

private:
    QQuickWebEngineView *engineView() const;
};

QT_END_NAMESPACE

namespace QtWebEngineCore {
class RenderWidgetHostViewQtDelegateQuickAccessible : public QAccessibleObject
{
public:
    RenderWidgetHostViewQtDelegateQuickAccessible(QObject *o, QQuickWebEngineView *view);

    bool isValid() const override;
    QAccessibleInterface *parent() const override;
    QString text(QAccessible::Text t) const override;
    QAccessible::Role role() const override;
    QAccessible::State state() const override;

    QAccessibleInterface *focusChild() const override;
    int childCount() const override;
    QAccessibleInterface *child(int index) const override;
    int indexOfChild(const QAccessibleInterface *) const override;

private:
    QQuickWebEngineViewAccessible *viewAccessible() const;
    QPointer<QQuickWebEngineView> m_view;
};
} // namespace QtWebEngineCore

#endif // QQUICKWEBENGINE_ACCESSIBLE_H
