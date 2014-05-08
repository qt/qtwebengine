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

#include "render_widget_host_view_qt_delegate_widget.h"

#include "qwebenginepage_p.h"
#include "qwebengineview.h"
#include <QLayout>
#include <QSGNode>
#include <private/qsgadaptationlayer_p.h>
#include <private/qsgcontext_p.h>
#include <private/qsgrenderer_p.h>
#include <private/qwidget_p.h>

RenderWidgetHostViewQtDelegateWidget::RenderWidgetHostViewQtDelegateWidget(RenderWidgetHostViewQtDelegateClient *client, QWidget *parent)
    : QOpenGLWidget(parent)
    , m_client(client)
    , rootNode(new QSGRootNode)
    , sgContext(QSGContext::createDefaultContext())
    , sgRenderContext(new QSGRenderContext(sgContext.data()))
    , m_isPopup(false)
{
    setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);
    setAttribute(Qt::WA_AcceptTouchEvents);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_AlwaysShowToolTips);
}

void RenderWidgetHostViewQtDelegateWidget::initAsChild(WebContentsAdapterClient* container)
{
    QWebEnginePagePrivate *pagePrivate = static_cast<QWebEnginePagePrivate *>(container);
    if (pagePrivate->view) {
        pagePrivate->view->layout()->addWidget(this);
        QOpenGLWidget::show();
    } else
        setParent(0);
}

void RenderWidgetHostViewQtDelegateWidget::initAsPopup(const QRect& screenRect)
{
    m_isPopup = true;
    // The keyboard events are supposed to go to the parent RenderHostView
    // so the WebUI popups should never have focus. Besides, if the parent view
    // loses focus, WebKit will cause its associated popups (including this one)
    // to be destroyed.
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFocusPolicy(Qt::NoFocus);
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);

    setGeometry(screenRect);
    show();
}

QRectF RenderWidgetHostViewQtDelegateWidget::screenRect() const
{
    return QRectF(x(), y(), width(), height());
}

void RenderWidgetHostViewQtDelegateWidget::setKeyboardFocus()
{
    setFocus();
}

bool RenderWidgetHostViewQtDelegateWidget::hasKeyboardFocus()
{
    return hasFocus();
}

void RenderWidgetHostViewQtDelegateWidget::show()
{
    // Check if we're attached to a QWebEngineView, we don't
    // want to show anything else than popups as top-level.
    if (parent() || m_isPopup)
        QOpenGLWidget::show();
}

void RenderWidgetHostViewQtDelegateWidget::hide()
{
    QOpenGLWidget::hide();
}

bool RenderWidgetHostViewQtDelegateWidget::isVisible() const
{
    return QOpenGLWidget::isVisible();
}

QWindow* RenderWidgetHostViewQtDelegateWidget::window() const
{
    const QWidget* root = QOpenGLWidget::window();
    return root ? root->windowHandle() : 0;
}

void RenderWidgetHostViewQtDelegateWidget::update()
{
    updateGL();
}

void RenderWidgetHostViewQtDelegateWidget::updateCursor(const QCursor &cursor)
{
    QOpenGLWidget::setCursor(cursor);
}

void RenderWidgetHostViewQtDelegateWidget::resize(int width, int height)
{
    QOpenGLWidget::resize(width, height);
}

void RenderWidgetHostViewQtDelegateWidget::move(const QPoint &screenPos)
{
    Q_ASSERT(m_isPopup);
    QOpenGLWidget::move(screenPos);
}

void RenderWidgetHostViewQtDelegateWidget::inputMethodStateChanged(bool editorVisible)
{
    if (qApp->inputMethod()->isVisible() == editorVisible)
        return;

    QOpenGLWidget::setAttribute(Qt::WA_InputMethodEnabled, editorVisible);
    qApp->inputMethod()->update(Qt::ImQueryInput | Qt::ImEnabled | Qt::ImHints);
    qApp->inputMethod()->setVisible(editorVisible);
}

void RenderWidgetHostViewQtDelegateWidget::setTooltip(const QString &tooltip)
{
    QString wrappedTip;
    if (!tooltip.isEmpty())
         wrappedTip = QStringLiteral("<p>") % tooltip % QStringLiteral("</p>");
    setToolTip(wrappedTip);
}

QVariant RenderWidgetHostViewQtDelegateWidget::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return m_client->inputMethodQuery(query);
}

void RenderWidgetHostViewQtDelegateWidget::resizeEvent(QResizeEvent *resizeEvent)
{
    QOpenGLWidget::resizeEvent(resizeEvent);
    m_client->notifyResize();
}

bool RenderWidgetHostViewQtDelegateWidget::event(QEvent *event)
{
    bool handled = false;
    if (event->type() == QEvent::MouseButtonDblClick) {
        // QWidget keeps the Qt4 behavior where the DblClick event would replace the Press event.
        // QtQuick is different by sending both the Press and DblClick events for the second press
        // where we can simply ignore the DblClick event.
        QMouseEvent *dblClick = static_cast<QMouseEvent *>(event);
        QMouseEvent press(QEvent::MouseButtonPress, dblClick->localPos(), dblClick->windowPos(), dblClick->screenPos(),
            dblClick->button(), dblClick->buttons(), dblClick->modifiers());
        press.setTimestamp(dblClick->timestamp());
        handled = m_client->forwardEvent(&press);
    } else
        handled = m_client->forwardEvent(event);

    if (!handled)
        return QOpenGLWidget::event(event);
    return true;
}

void RenderWidgetHostViewQtDelegateWidget::initializeGL()
{
    sgRenderContext->initialize(QOpenGLContext::currentContext());
    sgRenderer.reset(sgRenderContext->createRenderer());
    sgRenderer->setRootNode(rootNode.data());
}

void RenderWidgetHostViewQtDelegateWidget::paintGL()
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 3, 1))
    // A workaround for a missing check in 5.3.0 when updating an unparented delegate.
    if (!QOpenGLContext::currentContext())
        return;
#endif
    QSGNode *paintNode = m_client->updatePaintNode(rootNode->firstChild(), sgRenderContext.data());
    if (paintNode != rootNode->firstChild()) {
        delete rootNode->firstChild();
        rootNode->appendChildNode(paintNode);
    }

    rootNode->markDirty(QSGNode::DirtyForceUpdate); // Force matrix, clip and opacity update.
    sgRenderer->nodeChanged(rootNode.data(), QSGNode::DirtyForceUpdate); // Force render list update.

    sgRenderer->setDeviceRect(size());
    sgRenderer->setViewportRect(size());
    sgRenderer->setProjectionMatrixToRect(QRectF(QPointF(), size()));
    sgRenderer->setClearColor(Qt::transparent);

    sgRenderContext->renderNextFrame(sgRenderer.data(), defaultFramebufferObject());
}
