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

#include "raster_window.h"

#include "backing_store_qt.h"
#include "render_widget_host_view_qt.h"
#include <QResizeEvent>
#include <QShowEvent>
#include <QPaintEvent>

RasterWindow::RasterWindow(content::RenderWidgetHostViewQt* view, QWidget *parent)
    : QWidget(parent)
    , m_painter(0)
    , m_backingStore(0)
    , m_view(view)
{
    setFocusPolicy(Qt::ClickFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void RasterWindow::setBackingStore(BackingStoreQt* backingStore)
{
    m_backingStore = backingStore;
    if (m_backingStore)
        m_backingStore->resize(size());
}

void RasterWindow::paintEvent(QPaintEvent * event)
{
    if (!m_backingStore)
        return;
    QPainter painter(this);
    m_backingStore->paintToTarget(&painter, event->rect());
}

QPainter* RasterWindow::painter()
{
    if (!m_painter)
        m_painter = new QPainter(this);
    return m_painter;
}

void RasterWindow::resizeEvent(QResizeEvent *resizeEvent)
{
    if (m_backingStore)
        m_backingStore->resize(resizeEvent->size());
    update();
}


bool RasterWindow::event(QEvent *event)
{
    if (!m_view || !m_view->handleEvent(event))
        return QWidget::event(event);
    return true;
}
