/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#include "qquickwebengine_accessible.h"

#include <QQuickItem>
#include <QQuickWindow>

#include "api/qquickwebengineview_p.h"
#include "api/qquickwebengineview_p_p.h"
#include "web_contents_adapter.h"


#if QT_CONFIG(accessibility)
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
#endif // QT_CONFIG(accessibility)
