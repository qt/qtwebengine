// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengine_accessible.h"

#include "qwebengineview.h"
#include "qwebengineview_p.h"

#include "web_contents_adapter.h"

QT_BEGIN_NAMESPACE

QWebEngineViewAccessible::QWebEngineViewAccessible(QWebEngineView *o) : QAccessibleWidget(o)
{
}

bool QWebEngineViewAccessible::isValid() const
{
    if (!QAccessibleWidget::isValid())
        return false;

    if (!view() || !view()->d_func() || !view()->d_func()->page || !view()->d_func()->page->d_func())
        return false;

    return true;
}

QAccessibleInterface *QWebEngineViewAccessible::focusChild() const
{
    if (child(0) && child(0)->focusChild())
        return child(0)->focusChild();
    return const_cast<QWebEngineViewAccessible *>(this);
}

int QWebEngineViewAccessible::childCount() const
{
    return child(0) ? 1 : 0;
}

QAccessibleInterface *QWebEngineViewAccessible::child(int index) const
{
    if (index == 0 && isValid())
        return view()->page()->d_func()->adapter->browserAccessible();
    return nullptr;
}

int QWebEngineViewAccessible::indexOfChild(const QAccessibleInterface *c) const
{
    if (child(0) && c == child(0))
        return 0;
    return -1;
}

QWebEngineView *QWebEngineViewAccessible::view() const
{
    return static_cast<QWebEngineView *>(object());
}

QT_END_NAMESPACE

namespace QtWebEngineCore {

RenderWidgetHostViewQtDelegateWidgetAccessible::RenderWidgetHostViewQtDelegateWidgetAccessible(QWidget *o, QWebEngineView *view)
    : QAccessibleWidget(o)
    , m_view(view)
{
}

bool RenderWidgetHostViewQtDelegateWidgetAccessible::isValid() const
{
    if (!viewAccessible() || !viewAccessible()->isValid())
        return false;

    return QAccessibleWidget::isValid();
}

QAccessibleInterface *RenderWidgetHostViewQtDelegateWidgetAccessible::focusChild() const
{
    return viewAccessible()->focusChild();
}

int RenderWidgetHostViewQtDelegateWidgetAccessible::childCount() const
{
    return viewAccessible()->childCount();
}

QAccessibleInterface *RenderWidgetHostViewQtDelegateWidgetAccessible::child(int index) const
{
    return viewAccessible()->child(index);
}

int RenderWidgetHostViewQtDelegateWidgetAccessible::indexOfChild(const QAccessibleInterface *c) const
{
    return viewAccessible()->indexOfChild(c);
}

QWebEngineViewAccessible *RenderWidgetHostViewQtDelegateWidgetAccessible::viewAccessible() const
{
    return static_cast<QWebEngineViewAccessible *>(QAccessible::queryAccessibleInterface(m_view));
}

} // namespace QtWebEngineCore
