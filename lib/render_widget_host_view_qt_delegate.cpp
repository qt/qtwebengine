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

#include "render_widget_host_view_qt_delegate.h"

#include "backing_store_qt.h"
#include "render_widget_host_view_qt.h"

#include "base/command_line.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/public/common/content_switches.h"
#include <QPainter>

bool RenderWidgetHostViewQtDelegate::isCompositingModeForced()
{
    return CommandLine::ForCurrentProcess()->HasSwitch(switches::kForceCompositingMode);
}

RenderWidgetHostViewQtDelegate::RenderWidgetHostViewQtDelegate()
    : m_eventForwardingEnabled(false), m_view(0), m_backingStore(0)
{
}

RenderWidgetHostViewQtDelegate::~RenderWidgetHostViewQtDelegate()
{
}

void RenderWidgetHostViewQtDelegate::resetView(RenderWidgetHostViewQt* view)
{
    m_view.reset(view);
    m_eventForwardingEnabled = !!view;
}

void RenderWidgetHostViewQtDelegate::paint(QPainter *painter, const QRectF &boundingRect)
{
    if (m_backingStore)
        m_backingStore->paintToTarget(painter, boundingRect);
}

void RenderWidgetHostViewQtDelegate::fetchBackingStore()
{
    Q_ASSERT(m_view);
    m_backingStore = m_view->GetBackingStore();
}

void RenderWidgetHostViewQtDelegate::notifyResize()
{
    Q_ASSERT(m_view);
    m_view->GetRenderWidgetHost()->WasResized();
}

bool RenderWidgetHostViewQtDelegate::forwardEvent(QEvent *event)
{
    Q_ASSERT(m_view);
    return (m_view && m_view->handleEvent(event));
}
