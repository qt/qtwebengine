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

#include "qquickwebengineview_p.h"
#include "qquickwebengineview_p_p.h"

#include "web_contents_adapter.h"
#include "render_widget_host_view_qt_delegate_quick.h"

#include <QScreen>
#include <QUrl>

QT_BEGIN_NAMESPACE

static qreal initialDevicePixelRatio(QScreen *screen)
{
    // The gold standard for mobile web content is 160 dpi, and the devicePixelRatio expected
    // is the (possibly quantized) ratio of device dpi to 160 dpi.
    // However GUI toolkits on non-iOS platforms may be using different criteria than relative
    // DPI (depending on the history of that platform), dictating the choice of
    // QScreen::devicePixelRatio().
    // Where applicable (i.e. non-iOS mobile platforms), override QScreen::devicePixelRatio
    // and instead use a reasonable default value for viewport.devicePixelRatio to avoid every
    // app having to use this experimental API.
    QString platform = qApp->platformName().toLower();
    if (platform == QStringLiteral("qnx")) {
        qreal webPixelRatio = screen->physicalDotsPerInch() / 160;

        // Quantize devicePixelRatio to increments of 1 to allow JS and media queries to select
        // 1x, 2x, 3x etc assets that fit an integral number of pixels.
        return qMax(1, qRound(webPixelRatio));
    }

    // Use QScreen::devicePixelRatio()
    return 0.0;
}

QQuickWebEngineViewPrivate::QQuickWebEngineViewPrivate()
    : adapter(new WebContentsAdapter(qApp->property("QQuickWebEngineView_DisableHardwareAcceleration").toBool() ? SoftwareRenderingMode : HardwareAccelerationMode))
    , e(new QQuickWebEngineViewExperimental(this))
    , v(new QQuickWebEngineViewport(this))
    , loadProgress(0)
    , inspectable(false)
    , devicePixelRatio(0.0)
    , m_dpiScale(1.0)
{
    setDevicePixelRatio(initialDevicePixelRatio(QGuiApplication::primaryScreen()));
    adapter->initialize(this);
}

QQuickWebEngineViewExperimental *QQuickWebEngineViewPrivate::experimental() const
{
    return e.data();
}

QQuickWebEngineViewport *QQuickWebEngineViewPrivate::viewport() const
{
    return v.data();
}

RenderWidgetHostViewQtDelegate *QQuickWebEngineViewPrivate::CreateRenderWidgetHostViewQtDelegate(RenderWidgetHostViewQtDelegateClient *client, RenderingMode mode)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    if (mode == HardwareAccelerationMode)
        return new RenderWidgetHostViewQtDelegateQuick(client);
#endif
    return new RenderWidgetHostViewQtDelegateQuickPainted(client);
}

void QQuickWebEngineViewPrivate::titleChanged(const QString &title)
{
    Q_Q(QQuickWebEngineView);
    Q_UNUSED(title);
    Q_EMIT q->titleChanged();
}

void QQuickWebEngineViewPrivate::urlChanged(const QUrl &url)
{
    Q_Q(QQuickWebEngineView);
    Q_UNUSED(url);
    Q_EMIT q->urlChanged();
}

void QQuickWebEngineViewPrivate::iconChanged(const QUrl &url)
{
    Q_Q(QQuickWebEngineView);
    icon = url;
    Q_EMIT q->iconChanged();
}

void QQuickWebEngineViewPrivate::loadingStateChanged()
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->loadingStateChanged();
}

void QQuickWebEngineViewPrivate::loadProgressChanged(int progress)
{
    Q_Q(QQuickWebEngineView);
    loadProgress = progress;
    Q_EMIT q->loadProgressChanged();
}

QRectF QQuickWebEngineViewPrivate::viewportRect() const
{
    Q_Q(const QQuickWebEngineView);
    return QRectF(q->x(), q->y(), q->width(), q->height());
}

qreal QQuickWebEngineViewPrivate::dpiScale() const
{
    return m_dpiScale;
}

void QQuickWebEngineViewPrivate::loadFinished(bool success)
{
    Q_Q(QQuickWebEngineView);
    Q_UNUSED(success);
    Q_EMIT q->loadingStateChanged();
}

void QQuickWebEngineViewPrivate::focusContainer()
{
    Q_Q(QQuickWebEngineView);
    q->forceActiveFocus();
}

void QQuickWebEngineViewPrivate::adoptNewWindow(WebContentsAdapter *newWebContents, WindowOpenDisposition disposition, const QRect &)
{
    Q_UNUSED(newWebContents);
    Q_UNUSED(disposition);
    Q_UNREACHABLE();
}

void QQuickWebEngineViewPrivate::close()
{
    Q_UNREACHABLE();
}

void QQuickWebEngineViewPrivate::setDevicePixelRatio(qreal devicePixelRatio)
{
    this->devicePixelRatio = devicePixelRatio;
    if (devicePixelRatio) {
        QScreen *screen = window ? window->screen() : QGuiApplication::primaryScreen();
        m_dpiScale = devicePixelRatio / screen->devicePixelRatio();
    } else {
        m_dpiScale = 1.0;
    }
}

QQuickWebEngineView::QQuickWebEngineView(QQuickItem *parent)
    : QQuickItem(*(new QQuickWebEngineViewPrivate), parent)
{
    Q_D(const QQuickWebEngineView);
    d->e->q_ptr = this;
}

QQuickWebEngineView::~QQuickWebEngineView()
{
}

QUrl QQuickWebEngineView::url() const
{
    Q_D(const QQuickWebEngineView);
    return d->adapter->activeUrl();
}

void QQuickWebEngineView::setUrl(const QUrl& url)
{
    Q_D(QQuickWebEngineView);
    d->adapter->load(url);
}

QUrl QQuickWebEngineView::icon() const
{
    Q_D(const QQuickWebEngineView);
    return d->icon;
}

void QQuickWebEngineView::goBack()
{
    Q_D(QQuickWebEngineView);
    d->adapter->navigateToOffset(-1);
}

void QQuickWebEngineView::goForward()
{
    Q_D(QQuickWebEngineView);
    d->adapter->navigateToOffset(1);
}

void QQuickWebEngineView::reload()
{
    Q_D(QQuickWebEngineView);
    d->adapter->reload();
}

void QQuickWebEngineView::stop()
{
    Q_D(QQuickWebEngineView);
    d->adapter->stop();
}

bool QQuickWebEngineView::isLoading() const
{
    Q_D(const QQuickWebEngineView);
    return d->adapter->isLoading();
}

int QQuickWebEngineView::loadProgress() const
{
    Q_D(const QQuickWebEngineView);
    return d->loadProgress;
}

QString QQuickWebEngineView::title() const
{
    Q_D(const QQuickWebEngineView);
    return d->adapter->pageTitle();
}

bool QQuickWebEngineView::canGoBack() const
{
    Q_D(const QQuickWebEngineView);
    return d->adapter->canGoBack();
}

bool QQuickWebEngineView::canGoForward() const
{
    Q_D(const QQuickWebEngineView);
    return d->adapter->canGoForward();
}

bool QQuickWebEngineView::inspectable() const
{
    Q_D(const QQuickWebEngineView);
    return d->inspectable;
}

void QQuickWebEngineView::setInspectable(bool enable)
{
    Q_D(QQuickWebEngineView);
    d->inspectable = enable;
    d->adapter->enableInspector(enable);
}

void QQuickWebEngineView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    Q_FOREACH(QQuickItem *child, childItems()) {
        Q_ASSERT(
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
            qobject_cast<RenderWidgetHostViewQtDelegateQuick *>(child) ||
#endif
            qobject_cast<RenderWidgetHostViewQtDelegateQuickPainted *>(child));
        child->setSize(newGeometry.size());
    }
}

void QQuickWebEngineView::itemChange(ItemChange change, const ItemChangeData &value)
{
    Q_D(QQuickWebEngineView);
    if (change == ItemSceneChange)
        d->v->setWindow(value.window);
    QQuickItem::itemChange(change, value);
}

QQuickWebEngineViewExperimental::QQuickWebEngineViewExperimental(QQuickWebEngineViewPrivate *viewPrivate)
    : q_ptr(0)
    , d_ptr(viewPrivate)
{
}

QQuickWebEngineViewport *QQuickWebEngineViewExperimental::viewport() const
{
    Q_D(const QQuickWebEngineView);
    return d->viewport();
}

QQuickWebEngineViewport::QQuickWebEngineViewport(QQuickWebEngineViewPrivate *viewPrivate)
    : d_ptr(viewPrivate)
    , m_window(0)
    , m_screen(0)
{
}

qreal QQuickWebEngineViewport::devicePixelRatio() const
{
    Q_D(const QQuickWebEngineView);
    if (d->devicePixelRatio)
        return d->devicePixelRatio;
    QScreen* screen = d->window ? d->window->screen() : QGuiApplication::primaryScreen();
    return screen->devicePixelRatio();
}

void QQuickWebEngineViewport::setDevicePixelRatio(qreal devicePixelRatio)
{
    Q_D(QQuickWebEngineView);
    // Valid range is [1, inf)
    devicePixelRatio = qMax(1.0, devicePixelRatio);
    if (d->devicePixelRatio == devicePixelRatio)
        return;
    d->setDevicePixelRatio(devicePixelRatio);
    d->adapter->screenChanged();
    Q_EMIT devicePixelRatioChanged();
}

void QQuickWebEngineViewport::resetDevicePixelRatio()
{
    Q_D(QQuickWebEngineView);
    qreal previousDevicePixelRatio = d->devicePixelRatio;
    d->setDevicePixelRatio(initialDevicePixelRatio(m_screen));
    if (d->devicePixelRatio != previousDevicePixelRatio) {
        d->adapter->screenChanged();
        Q_EMIT devicePixelRatioChanged();
    }
}

void QQuickWebEngineViewport::setWindow(QQuickWindow* window)
{
    disconnect(m_window, SIGNAL(screenChanged(QScreen*)), this, SLOT(screenChanged(QScreen*)));
    m_window = window;
    screenChanged(window ? window->screen() : NULL);
    if (window)
        connect(window, SIGNAL(screenChanged(QScreen*)), this, SLOT(screenChanged(QScreen*)));
}

void QQuickWebEngineViewport::screenChanged(QScreen* screen)
{
    if (screen == m_screen)
        return;

    Q_D(QQuickWebEngineView);
    qreal previousScreenDevicePixelRatio = m_screen ? m_screen->devicePixelRatio() : QGuiApplication::primaryScreen()->devicePixelRatio();
    m_screen = screen;
    qreal screenDevicePixelRatio = m_screen ? m_screen->devicePixelRatio() : QGuiApplication::primaryScreen()->devicePixelRatio();
    if (screenDevicePixelRatio != previousScreenDevicePixelRatio)
        d->setDevicePixelRatio(d->devicePixelRatio);
    d->adapter->screenChanged();
    if (d->devicePixelRatio == 0 && screenDevicePixelRatio != previousScreenDevicePixelRatio)
        Q_EMIT devicePixelRatioChanged();
}

QT_END_NAMESPACE
