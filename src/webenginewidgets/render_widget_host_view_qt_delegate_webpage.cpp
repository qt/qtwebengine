/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include "render_widget_host_view_qt_delegate_webpage.h"

#include "qwebengineview.h"
#include "qwebenginepage.h"
#include "qwebenginepage_p.h"
#include <QtGlobal>
#include <QResizeEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QWindow>
#include <QtWidgets/QApplication>

RenderWidgetHostViewQtDelegateWebPage::RenderWidgetHostViewQtDelegateWebPage(RenderWidgetHostViewQtDelegateClient *client)
    : m_client(client)
    , m_page(0)
{
}

void RenderWidgetHostViewQtDelegateWebPage::initAsChild(WebContentsAdapterClient* container)
{
    QWebEnginePagePrivate *pagePrivate = static_cast<QWebEnginePagePrivate *>(container);
    pagePrivate->m_rwhvDelegate = this;
    setPage(pagePrivate->q_func());
}

QRectF RenderWidgetHostViewQtDelegateWebPage::screenRect() const
{
    if (m_page && m_page->view())
        return m_page->view()->rect();
    // FIXME: figure out what to do with QWebFrame::contentsSize vs. preferedContentsSize
    return QRectF(0, 0, 800, 600);
}

void RenderWidgetHostViewQtDelegateWebPage::setKeyboardFocus()
{
    if (m_page)
        m_page->setFocus();
    if (m_page && m_page->view())
        m_page->view()->setFocus();
}

bool RenderWidgetHostViewQtDelegateWebPage::hasKeyboardFocus()
{
    return m_page && m_page->hasFocus();
}

bool RenderWidgetHostViewQtDelegateWebPage::isVisible() const
{
    if (m_page && m_page->view())
        return m_page->view()->isVisible();
    return false;
}

QWindow* RenderWidgetHostViewQtDelegateWebPage::window() const
{
    if (!m_page || !m_page->view())
        return Q_NULLPTR;
    return m_page->view()->window()->windowHandle();
}

void RenderWidgetHostViewQtDelegateWebPage::update(const QRect& rect)
{
    if (m_page->view())
        m_page->view()->update(rect);
}

void RenderWidgetHostViewQtDelegateWebPage::updateCursor(const QCursor &cursor)
{
    if (m_page->view())
        m_page->view()->setCursor(cursor);
}

void RenderWidgetHostViewQtDelegateWebPage::resize(int width, int height)
{
    Q_UNUSED(width)
    Q_UNUSED(height)
    QT_NOT_YET_IMPLEMENTED;
}

void RenderWidgetHostViewQtDelegateWebPage::inputMethodStateChanged(bool editorVisible)
{
    if (qApp->inputMethod()->isVisible() == editorVisible)
        return;

    if (m_page && m_page->view())
        m_page->view()->setAttribute(Qt::WA_InputMethodEnabled, editorVisible);
    qApp->inputMethod()->update(Qt::ImQueryInput | Qt::ImEnabled | Qt::ImHints);
    qApp->inputMethod()->setVisible(editorVisible);
}

QVariant RenderWidgetHostViewQtDelegateWebPage::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return m_client->inputMethodQuery(query);
}

bool RenderWidgetHostViewQtDelegateWebPage::supportsHardwareAcceleration() const
{
    return false;
}

void RenderWidgetHostViewQtDelegateWebPage::paint(QPainter *painter, const QRectF &boundingRect)
{
    m_client->fetchBackingStore();
    m_client->paint(painter, boundingRect);
}

void RenderWidgetHostViewQtDelegateWebPage::notifyResize()
{
    m_client->notifyResize();
}

bool RenderWidgetHostViewQtDelegateWebPage::forwardEvent(QEvent *event)
{
    return m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateWebPage::setPage(QWebEnginePage *page)
{
    m_page = page;
}
