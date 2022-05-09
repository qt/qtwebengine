/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "qwebengine_accessible.h"

#include "qwebengineview.h"
#include "qwebengineview_p.h"

#include "web_contents_adapter.h"

#if QT_CONFIG(accessibility)

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

#endif // QT_CONFIG(accessibility)
