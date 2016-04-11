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

#include "render_widget_host_view_qt_delegate_widget.h"

#include "qwebenginepage_p.h"
#include "qwebengineview.h"
#include <QGuiApplication>
#include <QLayout>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QResizeEvent>
#include <QSGAbstractRenderer>
#include <QSGNode>
#include <QWindow>
#include <private/qsgcontext_p.h>
#include <private/qsgengine_p.h>

namespace QtWebEngineCore {

static const int MaxTooltipLength = 1024;

RenderWidgetHostViewQtDelegateWidget::RenderWidgetHostViewQtDelegateWidget(RenderWidgetHostViewQtDelegateClient *client, QWidget *parent)
    : QOpenGLWidget(parent)
    , m_client(client)
    , m_rootNode(new QSGRootNode)
    , m_sgEngine(new QSGEngine)
    , m_isPopup(false)
    , m_clearColor(Qt::white)
{
    setFocusPolicy(Qt::StrongFocus);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);

    QOpenGLContext *globalSharedContext = QOpenGLContext::globalShareContext();
    if (globalSharedContext) {
        QSurfaceFormat sharedFormat = globalSharedContext->format();

#ifdef Q_OS_OSX
        // Check that the default QSurfaceFormat OpenGL profile is compatible with the global OpenGL
        // shared context profile, otherwise this could lead to a nasty crash.
        QSurfaceFormat defaultFormat = QSurfaceFormat::defaultFormat();

        if (defaultFormat.profile() != sharedFormat.profile()
            && defaultFormat.profile() == QSurfaceFormat::CoreProfile
            && defaultFormat.version() >= qMakePair(3, 2)) {
            qFatal("QWebEngine: Default QSurfaceFormat OpenGL profile is not compatible with the "
                   "global shared context OpenGL profile. Please make sure you set a compatible "
                   "QSurfaceFormat before the QtGui application instance is created.");
        }
#endif

        // Make sure the OpenGL profile of the QOpenGLWidget matches the shared context profile.
        if (sharedFormat.profile() == QSurfaceFormat::CoreProfile) {
            format.setMajorVersion(sharedFormat.majorVersion());
            format.setMinorVersion(sharedFormat.minorVersion());
            format.setProfile(sharedFormat.profile());
        }
    }

    setFormat(format);
#endif

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
        pagePrivate->view->setFocusProxy(this);
        show();
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

QRectF RenderWidgetHostViewQtDelegateWidget::contentsRect() const
{
    QPointF pos = mapToGlobal(QPoint(0, 0));
    return QRectF(pos.x(), pos.y(), width(), height());
}

void RenderWidgetHostViewQtDelegateWidget::setKeyboardFocus()
{
    setFocus();
}

bool RenderWidgetHostViewQtDelegateWidget::hasKeyboardFocus()
{
    return hasFocus();
}

void RenderWidgetHostViewQtDelegateWidget::lockMouse()
{
    grabMouse();
}

void RenderWidgetHostViewQtDelegateWidget::unlockMouse()
{
    releaseMouse();
}

void RenderWidgetHostViewQtDelegateWidget::show()
{
    // Check if we're attached to a QWebEngineView, we don't
    // want to show anything else than popups as top-level.
    if (parent() || m_isPopup) {
        QOpenGLWidget::show();
    }
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

QSGTexture *RenderWidgetHostViewQtDelegateWidget::createTextureFromImage(const QImage &image)
{
    return m_sgEngine->createTextureFromImage(image, QSGEngine::TextureCanUseAtlas);
}

QSGLayer *RenderWidgetHostViewQtDelegateWidget::createLayer()
{
    QSGEnginePrivate *enginePrivate = QSGEnginePrivate::get(m_sgEngine.data());
    return enginePrivate->sgContext->createLayer(enginePrivate->sgRenderContext.data());
}

QSGImageNode *RenderWidgetHostViewQtDelegateWidget::createImageNode()
{
    return QSGEnginePrivate::get(m_sgEngine.data())->sgContext->createImageNode();
}

void RenderWidgetHostViewQtDelegateWidget::update()
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 4, 0))
    updateGL();
#else
    QOpenGLWidget::update();
#endif
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
         wrappedTip = QLatin1String("<p>") % tooltip.toHtmlEscaped().left(MaxTooltipLength) % QLatin1String("</p>");
    setToolTip(wrappedTip);
}

void RenderWidgetHostViewQtDelegateWidget::setClearColor(const QColor &color)
{
    m_clearColor = color;
    // QOpenGLWidget is usually blended by punching holes into widgets
    // above it to simulate the visual stacking order. If we want it to be
    // transparent we have to throw away the proper stacking order and always
    // blend the complete normal widgets backing store under it.
    bool isTranslucent = color.alpha() < 255;
    setAttribute(Qt::WA_AlwaysStackOnTop, isTranslucent);
    setAttribute(Qt::WA_OpaquePaintEvent, !isTranslucent);
    update();
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

void RenderWidgetHostViewQtDelegateWidget::showEvent(QShowEvent *event)
{
    QOpenGLWidget::showEvent(event);
    // We don't have a way to catch a top-level window change with QWidget
    // but a widget will most likely be shown again if it changes, so do
    // the reconnection at this point.
    foreach (const QMetaObject::Connection &c, m_windowConnections)
        disconnect(c);
    m_windowConnections.clear();
    if (QWindow *w = window()) {
        m_windowConnections.append(connect(w, SIGNAL(xChanged(int)), SLOT(onWindowPosChanged())));
        m_windowConnections.append(connect(w, SIGNAL(yChanged(int)), SLOT(onWindowPosChanged())));
    }
    m_client->windowChanged();
    m_client->notifyShown();
}

void RenderWidgetHostViewQtDelegateWidget::hideEvent(QHideEvent *event)
{
    QOpenGLWidget::hideEvent(event);
    m_client->notifyHidden();
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
    m_sgEngine->initialize(QOpenGLContext::currentContext());
    m_sgRenderer.reset(m_sgEngine->createRenderer());
    m_sgRenderer->setRootNode(m_rootNode.data());
    m_sgRenderer->setClearColor(m_clearColor);

    // When RenderWidgetHostViewQt::GetScreenInfo is called for the first time, the associated
    // QWindow is NULL, and the screen device pixel ratio can not be queried.
    // Re-initialize the screen information after the QWindow handle is available,
    // so Chromium receives the correct device pixel ratio.
    m_client->windowChanged();
}

void RenderWidgetHostViewQtDelegateWidget::paintGL()
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 3, 1))
    // A workaround for a missing check in 5.3.0 when updating an unparented delegate.
    if (!QOpenGLContext::currentContext())
        return;
#endif
    QSGNode *paintNode = m_client->updatePaintNode(m_rootNode->firstChild());
    if (paintNode != m_rootNode->firstChild()) {
        delete m_rootNode->firstChild();
        m_rootNode->appendChildNode(paintNode);
    }

    QSize deviceSize = size() * devicePixelRatio();
    m_sgRenderer->setDeviceRect(deviceSize);
    m_sgRenderer->setViewportRect(deviceSize);
    m_sgRenderer->setProjectionMatrixToRect(QRectF(QPointF(), size()));

    m_sgRenderer->renderScene(defaultFramebufferObject());
}

void RenderWidgetHostViewQtDelegateWidget::onWindowPosChanged()
{
    m_client->windowBoundsChanged();
}

} // namespace QtWebEngineCore
