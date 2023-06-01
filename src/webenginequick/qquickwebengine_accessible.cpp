// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwebengine_accessible.h"

#include <QQuickItem>
#include <QQuickWindow>

#include "api/qquickwebengineview_p.h"
#include "api/qquickwebengineview_p_p.h"
#include "web_contents_adapter.h"

QT_BEGIN_NAMESPACE
QQuickWebEngineViewAccessible::QQuickWebEngineViewAccessible(QQuickWebEngineView *o)
    : QAccessibleObject(o)
{}

bool QQuickWebEngineViewAccessible::isValid() const
{
    if (!QAccessibleObject::isValid())
        return false;

    if (!engineView() || !engineView()->d_func())
        return false;

    return true;
}

QAccessibleInterface *QQuickWebEngineViewAccessible::parent() const
{
    QQuickItem *parent = engineView()->parentItem();
    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(parent);
    if (!iface)
        return QAccessible::queryAccessibleInterface(engineView()->window());
    return iface;
}

QAccessibleInterface *QQuickWebEngineViewAccessible::focusChild() const
{
    if (child(0) && child(0)->focusChild())
        return child(0)->focusChild();
    return const_cast<QQuickWebEngineViewAccessible *>(this);
}

int QQuickWebEngineViewAccessible::childCount() const
{
    return child(0) ? 1 : 0;
}

QAccessibleInterface *QQuickWebEngineViewAccessible::child(int index) const
{
    if (index == 0 && isValid())
        return engineView()->d_func()->adapter->browserAccessible();
    return nullptr;
}

int QQuickWebEngineViewAccessible::indexOfChild(const QAccessibleInterface *c) const
{
    if (child(0) && c == child(0))
        return 0;
    return -1;
}

QString QQuickWebEngineViewAccessible::text(QAccessible::Text) const
{
    return QString();
}

QAccessible::Role QQuickWebEngineViewAccessible::role() const
{
    return QAccessible::Client;
}

QAccessible::State QQuickWebEngineViewAccessible::state() const
{
    QAccessible::State s;
    return s;
}

QQuickWebEngineView *QQuickWebEngineViewAccessible::engineView() const
{
    return static_cast<QQuickWebEngineView*>(object());
}

QT_END_NAMESPACE

namespace QtWebEngineCore {

RenderWidgetHostViewQtDelegateQuickAccessible::RenderWidgetHostViewQtDelegateQuickAccessible(QObject *o, QQuickWebEngineView *view)
    : QAccessibleObject(o)
    , m_view(view)
{
}

bool RenderWidgetHostViewQtDelegateQuickAccessible::isValid() const
{
    if (!viewAccessible() || !viewAccessible()->isValid())
        return false;

    return QAccessibleObject::isValid();
}

QAccessibleInterface *RenderWidgetHostViewQtDelegateQuickAccessible::parent() const
{
    return viewAccessible()->parent();
}

QString RenderWidgetHostViewQtDelegateQuickAccessible::text(QAccessible::Text) const
{
    return QString();
}

QAccessible::Role RenderWidgetHostViewQtDelegateQuickAccessible::role() const
{
    return QAccessible::Client;
}

QAccessible::State RenderWidgetHostViewQtDelegateQuickAccessible::state() const
{
    return viewAccessible()->state();
}

QAccessibleInterface *RenderWidgetHostViewQtDelegateQuickAccessible::focusChild() const
{
    return viewAccessible()->focusChild();
}

int RenderWidgetHostViewQtDelegateQuickAccessible::childCount() const
{
    return viewAccessible()->childCount();
}

QAccessibleInterface *RenderWidgetHostViewQtDelegateQuickAccessible::child(int index) const
{
    return viewAccessible()->child(index);
}

int RenderWidgetHostViewQtDelegateQuickAccessible::indexOfChild(const QAccessibleInterface *c) const
{
    return viewAccessible()->indexOfChild(c);
}

QQuickWebEngineViewAccessible *RenderWidgetHostViewQtDelegateQuickAccessible::viewAccessible() const
{
    return static_cast<QQuickWebEngineViewAccessible *>(QAccessible::queryAccessibleInterface(m_view));
}
} // namespace QtWebEngineCore
