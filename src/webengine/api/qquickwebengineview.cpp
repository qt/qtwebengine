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

#include "qquickwebengineloadrequest_p.h"
#include "web_contents_adapter.h"
#include "web_engine_error.h"
#include "render_widget_host_view_qt_delegate_quick.h"

#include <QScreen>
#include <QUrl>
#include <QQmlEngine>

QT_BEGIN_NAMESPACE

QQuickWebEngineViewPrivate::QQuickWebEngineViewPrivate()
    : adapter(new WebContentsAdapter(qApp->property("QQuickWebEngineView_DisableHardwareAcceleration").toBool() ? SoftwareRenderingMode : HardwareAccelerationMode))
    , e(new QQuickWebEngineViewExperimental(this))
    , v(new QQuickWebEngineViewport(this))
    , loadProgress(0)
    , inspectable(false)
    , m_isLoading(false)
    , devicePixelRatio(QGuiApplication::primaryScreen()->devicePixelRatio())
    , m_dpiScale(1.0)
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
        qreal webPixelRatio = QGuiApplication::primaryScreen()->physicalDotsPerInch() / 160;

        // Quantize devicePixelRatio to increments of 1 to allow JS and media queries to select
        // 1x, 2x, 3x etc assets that fit an integral number of pixels.
        setDevicePixelRatio(qMax(1, qRound(webPixelRatio)));
    }

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
    const bool wasLoading = m_isLoading;
    m_isLoading = adapter->isLoading();
    // TODO: m_wasLoading == true -> m_isLoading == false
    if (m_isLoading != wasLoading) {
        if (m_isLoading) {
            QQuickWebEngineLoadRequest loadRequest(q->url(), QQuickWebEngineView::LoadStartedStatus);
            Q_EMIT q->loadingStateChanged(&loadRequest);
        }
    }
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

void QQuickWebEngineViewPrivate::loadFinished(bool success, int error_code, const QString &error_description)
{
    Q_Q(QQuickWebEngineView);
    if (success) {
        QQuickWebEngineLoadRequest loadRequest(q->url(), QQuickWebEngineView::LoadSucceededStatus);
        Q_EMIT q->loadingStateChanged(&loadRequest);
    } else {
        if (error_code) {
            QQuickWebEngineLoadRequest loadRequest(q->url(), QQuickWebEngineView::LoadFailedStatus, error_description, error_code, static_cast<QQuickWebEngineView::ErrorDomain>(WebEngineError::toQtErrorDomain(error_code)));
            Q_EMIT q->loadingStateChanged(&loadRequest);
        }

    // TODO: LoadStoppedStatus should be emitted when the request is cancelled.
    // QQuickWebEngineLoadRequest loadRequest(q->url(), QQuickWebEngineView::LoadStoppedStatus);
    // Q_EMIT q->loadingStateChanged(loadRequest);
    }
}

void QQuickWebEngineViewPrivate::focusContainer()
{
    Q_Q(QQuickWebEngineView);
    q->forceActiveFocus();
}

void QQuickWebEngineViewPrivate::adoptNewWindow(WebContentsAdapter *newWebContents, WindowOpenDisposition disposition, const QRect &)
{
    Q_Q(QQuickWebEngineView);
    QQmlEngine *engine = QtQml::qmlEngine(q);
    // This is currently only supported for QML instantiated WebEngineViews.
    // We could emit a QObject* and set JavaScriptOwnership explicitly on it
    // but this would make the signal cumbersome to use in C++ where one, and
    // only one, of the connected slots would have to destroy the given handle.
    // A virtual method instead of a signal would work better in this case.
    if (!engine)
        return;
    static const QMetaMethod createWindowSignal = QMetaMethod::fromSignal(&QQuickWebEngineViewExperimental::createWindow);
    if (!e->isSignalConnected(createWindowSignal))
        return;

    QQuickWebEngineViewHandle *handle = new QQuickWebEngineViewHandle;
    // This increases the ref-count of newWebContents and will tell Chromium
    // to start loading it and possibly return it to its parent page window.open().
    handle->adapter = newWebContents;
    // Clearly mark our wrapper as owned by JavaScript, we then depend on it
    // being adopted or else eventually cleaned up by the GC.
    QJSValue jsHandle = engine->newQObject(handle);

    QString dispositionString;
    switch (disposition) {
    case WebContentsAdapterClient::NewForegroundTabDisposition:
    case WebContentsAdapterClient::NewBackgroundTabDisposition:
        dispositionString = QStringLiteral("tab");
        break;
    case WebContentsAdapterClient::NewPopupDisposition:
        dispositionString = QStringLiteral("popup");
        break;
    case WebContentsAdapterClient::NewWindowDisposition:
        dispositionString = QStringLiteral("window");
        break;
    default:
        Q_UNREACHABLE();
    }

    emit e->createWindow(jsHandle, dispositionString);

    // We currently require the adoption to happen within the signal handler to avoid having
    // to support a null WebContentsAdapterClient for too long after having returned.
    handle->adapter.reset();
}

void QQuickWebEngineViewPrivate::close()
{
    Q_UNREACHABLE();
}

void QQuickWebEngineViewPrivate::setDevicePixelRatio(qreal devicePixelRatio)
{
    this->devicePixelRatio = devicePixelRatio;
    QScreen *screen = window ? window->screen() : QGuiApplication::primaryScreen();
    m_dpiScale = devicePixelRatio / screen->devicePixelRatio();
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

QQuickWebEngineViewHandle::QQuickWebEngineViewHandle()
{
}

QQuickWebEngineViewHandle::~QQuickWebEngineViewHandle()
{
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
{
}

qreal QQuickWebEngineViewport::devicePixelRatio() const
{
    Q_D(const QQuickWebEngineView);
    return d->devicePixelRatio;
}

void QQuickWebEngineViewport::setDevicePixelRatio(qreal devicePixelRatio)
{
    Q_D(QQuickWebEngineView);
    // Valid range is [1, inf)
    devicePixelRatio = qMax(qreal(1.0), devicePixelRatio);
    if (d->devicePixelRatio == devicePixelRatio)
        return;
    d->setDevicePixelRatio(devicePixelRatio);
    d->adapter->dpiScaleChanged();
    Q_EMIT devicePixelRatioChanged();
}

void QQuickWebEngineViewExperimental::adoptHandle(QQuickWebEngineViewHandle *viewHandle)
{
    if (!viewHandle || !viewHandle->adapter) {
        qWarning("Trying to adopt an empty handle, it was either already adopted or was invalidated."
            "\nYou must do the adoption synchronously within the createWindow signal handler."
            " If the handle hasn't been adopted before returning, it will be invalidated.");
        return;
    }

    Q_Q(QQuickWebEngineView);
    Q_D(QQuickWebEngineView);

    // This throws away the WebContentsAdapter that has been used until now.
    // All its states, particularly the loading URL, are replaced by the adopted WebContentsAdapter.
    d->adapter = viewHandle->adapter;
    viewHandle->adapter.reset();

    d->adapter->initialize(d);

    // Emit signals for values that might be different from the previous WebContentsAdapter.
    emit q->titleChanged();
    emit q->urlChanged();
    emit q->iconChanged();
    // FIXME: This is a workaround for fix compilation failure with the QQuickWebEngineLoadRequest change.
    // The loadingStateChanged signal has a QQuickWebEngineLoadRequest as an argument. In this fix the
    // state of the QQuickWebEngineLoadRequest is always LoadSucceededStatus.
    // New implementation is needed: the state should be stored in the WebContentAdapter
    // and check whether the signal emission is really necessary (state is not changed).
    QQuickWebEngineLoadRequest loadRequest(viewHandle->adapter->activeUrl(), QQuickWebEngineView::LoadSucceededStatus);
    emit q->loadingStateChanged(&loadRequest);
    emit q->loadProgressChanged();
}

QT_END_NAMESPACE

#include "moc_qquickwebengineview_p.cpp"
#include "moc_qquickwebengineview_p_p.cpp"
