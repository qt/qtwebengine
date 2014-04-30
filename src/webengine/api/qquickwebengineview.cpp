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

#include "javascript_dialog_controller.h"
#include "qquickwebenginehistory_p.h"
#include "qquickwebengineloadrequest_p.h"
#include "qquickwebenginenewviewrequest_p.h"
#include "render_widget_host_view_qt_delegate_quick.h"
#include "render_widget_host_view_qt_delegate_quickwindow.h"
#include "ui_delegates_manager.h"
#include "web_contents_adapter.h"
#include "web_engine_error.h"

#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QScreen>
#include <QStringBuilder>
#include <QUrl>
#include <private/qqmlmetatype_p.h>
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>

QT_BEGIN_NAMESPACE

QQuickWebEngineViewPrivate::QQuickWebEngineViewPrivate()
    : adapter(new WebContentsAdapter)
    , e(new QQuickWebEngineViewExperimental(this))
    , v(new QQuickWebEngineViewport(this))
    , m_history(new QQuickWebEngineHistory(this))
    , contextMenuExtraItems(0)
    , loadProgress(0)
    , inspectable(false)
    , m_isFullScreen(false)
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
}

QQuickWebEngineViewPrivate::~QQuickWebEngineViewPrivate()
{
}

QQuickWebEngineViewExperimental *QQuickWebEngineViewPrivate::experimental() const
{
    return e.data();
}

QQuickWebEngineViewport *QQuickWebEngineViewPrivate::viewport() const
{
    return v.data();
}

UIDelegatesManager *QQuickWebEngineViewPrivate::ui()
{
    Q_Q(QQuickWebEngineView);
    if (m_uIDelegatesManager.isNull())
        m_uIDelegatesManager.reset(new UIDelegatesManager(q));
    return m_uIDelegatesManager.data();
}

RenderWidgetHostViewQtDelegate *QQuickWebEngineViewPrivate::CreateRenderWidgetHostViewQtDelegate(RenderWidgetHostViewQtDelegateClient *client)
{
    return new RenderWidgetHostViewQtDelegateQuick(client, /*isPopup = */ false);
}

RenderWidgetHostViewQtDelegate *QQuickWebEngineViewPrivate::CreateRenderWidgetHostViewQtDelegateForPopup(RenderWidgetHostViewQtDelegateClient *client)
{
    Q_Q(QQuickWebEngineView);
    const bool hasWindowCapability = QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::MultipleWindows);
    RenderWidgetHostViewQtDelegateQuick *quickDelegate = new RenderWidgetHostViewQtDelegateQuick(client, /*isPopup = */ true);
    if (hasWindowCapability) {
        RenderWidgetHostViewQtDelegateQuickWindow *wrapperWindow = new RenderWidgetHostViewQtDelegateQuickWindow(quickDelegate);
        quickDelegate->setParentItem(wrapperWindow->contentItem());
        return wrapperWindow;
    }
    quickDelegate->setParentItem(q);
    return quickDelegate;
}

bool QQuickWebEngineViewPrivate::contextMenuRequested(const WebEngineContextMenuData &data)
{
    Q_Q(QQuickWebEngineView);

    QObject *menu = ui()->addMenu(0, QString(), data.pos);
    if (!menu)
        return false;

    // Populate our menu
    MenuItemHandler *item = 0;

    if (data.selectedText.isEmpty()) {
        item = new MenuItemHandler(menu);
        QObject::connect(item, &MenuItemHandler::triggered, q, &QQuickWebEngineView::goBack);
        ui()->addMenuItem(item, QObject::tr("Back"), QStringLiteral("go-previous"), q->canGoBack());

        item = new MenuItemHandler(menu);
        QObject::connect(item, &MenuItemHandler::triggered, q, &QQuickWebEngineView::goForward);
        ui()->addMenuItem(item, QObject::tr("Forward"), QStringLiteral("go-next"), q->canGoForward());

        item = new MenuItemHandler(menu);
        QObject::connect(item, &MenuItemHandler::triggered, q, &QQuickWebEngineView::reload);
        ui()->addMenuItem(item, QObject::tr("Reload"), QStringLiteral("view-refresh"));
    } else {
        item = new CopyMenuItem(menu, data.selectedText);
        ui()->addMenuItem(item, QObject::tr("Copy..."));
    }

    if (!data.linkText.isEmpty() && data.linkUrl.isValid()) {
        item = new NavigateMenuItem(menu, adapter, data.linkUrl);
        ui()->addMenuItem(item, QObject::tr("Navigate to..."));
        item = new CopyMenuItem(menu, data.linkUrl.toString());
        ui()->addMenuItem(item, QObject::tr("Copy link address"));
    }

    // FIXME: expose the context menu data as an attached property to make this more useful
    if (contextMenuExtraItems) {
        ui()->addMenuSeparator(menu);
        if (QObject* menuExtras = contextMenuExtraItems->create(ui()->creationContextForComponent(contextMenuExtraItems))) {
            menuExtras->setParent(menu);
            QQmlListReference entries(menu, QQmlMetaType::defaultProperty(menu).name(), qmlEngine(q));
            if (entries.isValid())
                entries.append(menuExtras);
        }
    }

    // Now fire the popup() method on the top level menu
    QMetaObject::invokeMethod(menu, "popup");
    return true;
}

void QQuickWebEngineViewPrivate::javascriptDialog(QSharedPointer<JavaScriptDialogController> dialog)
{
    ui()->showDialog(dialog);
}


void QQuickWebEngineViewPrivate::runFileChooser(FileChooserMode mode, const QString &defaultFileName, const QStringList &acceptedMimeTypes)
{
    ui()->showFilePicker(mode, defaultFileName, acceptedMimeTypes, adapter);
}

void QQuickWebEngineViewPrivate::passOnFocus(bool reverse)
{
    Q_Q(QQuickWebEngineView);
    focusNextPrev(q, !reverse);
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

void QQuickWebEngineViewPrivate::loadProgressChanged(int progress)
{
    Q_Q(QQuickWebEngineView);
    loadProgress = progress;
    Q_EMIT q->loadProgressChanged();
}

void QQuickWebEngineViewPrivate::didUpdateTargetURL(const QUrl &hoveredUrl)
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->linkHovered(hoveredUrl);
}

QRectF QQuickWebEngineViewPrivate::viewportRect() const
{
    Q_Q(const QQuickWebEngineView);
    return QRectF(q->x(), q->y(), q->width(), q->height());
}

QPoint QQuickWebEngineViewPrivate::mapToGlobal(const QPoint &posInView) const
{
    Q_Q(const QQuickWebEngineView);
    return q->window() ? q->window()->mapToGlobal(posInView) : QPoint();
}

qreal QQuickWebEngineViewPrivate::dpiScale() const
{
    return m_dpiScale;
}

void QQuickWebEngineViewPrivate::loadStarted(const QUrl &provisionalUrl)
{
    Q_Q(QQuickWebEngineView);
    m_history->reset();
    QQuickWebEngineLoadRequest loadRequest(provisionalUrl, QQuickWebEngineView::LoadStartedStatus);
    Q_EMIT q->loadingChanged(&loadRequest);
}

void QQuickWebEngineViewPrivate::loadCommitted()
{
    m_history->reset();
}

void QQuickWebEngineViewPrivate::loadFinished(bool success, int error_code, const QString &error_description)
{
    Q_Q(QQuickWebEngineView);
    m_history->reset();
    if (error_code == WebEngineError::UserAbortedError) {
        QQuickWebEngineLoadRequest loadRequest(q->url(), QQuickWebEngineView::LoadStoppedStatus);
        Q_EMIT q->loadingChanged(&loadRequest);
        return;
    }
    if (success) {
        QQuickWebEngineLoadRequest loadRequest(q->url(), QQuickWebEngineView::LoadSucceededStatus);
        Q_EMIT q->loadingChanged(&loadRequest);
        return;
    }

    Q_ASSERT(error_code);
    QQuickWebEngineLoadRequest loadRequest(
        q->url(),
        QQuickWebEngineView::LoadFailedStatus,
        error_description,
        error_code,
        static_cast<QQuickWebEngineView::ErrorDomain>(WebEngineError::toQtErrorDomain(error_code)));
    Q_EMIT q->loadingChanged(&loadRequest);
    return;
}

void QQuickWebEngineViewPrivate::focusContainer()
{
    Q_Q(QQuickWebEngineView);
    q->forceActiveFocus();
}

void QQuickWebEngineViewPrivate::adoptNewWindow(WebContentsAdapter *newWebContents, WindowOpenDisposition disposition, bool userGesture, const QRect &)
{
    QQuickWebEngineNewViewRequest request;
    // This increases the ref-count of newWebContents and will tell Chromium
    // to start loading it and possibly return it to its parent page window.open().
    request.m_adapter = newWebContents;
    request.m_isUserInitiated = userGesture;

    switch (disposition) {
    case WebContentsAdapterClient::NewForegroundTabDisposition:
    case WebContentsAdapterClient::NewBackgroundTabDisposition:
        request.m_destination = QQuickWebEngineView::NewViewInTab;
        break;
    case WebContentsAdapterClient::NewPopupDisposition:
        request.m_destination = QQuickWebEngineView::NewViewInDialog;
        break;
    case WebContentsAdapterClient::NewWindowDisposition:
        request.m_destination = QQuickWebEngineView::NewViewInWindow;
        break;
    default:
        Q_UNREACHABLE();
    }

    emit e->newViewRequested(&request);
}

void QQuickWebEngineViewPrivate::close()
{
    Q_UNREACHABLE();
}

void QQuickWebEngineViewPrivate::requestFullScreen(bool fullScreen)
{
    Q_EMIT e->fullScreenRequested(fullScreen);
}

bool QQuickWebEngineViewPrivate::isFullScreen() const
{
    return e->isFullScreen();
}

void QQuickWebEngineViewPrivate::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID)
{
    Q_Q(QQuickWebEngineView);
    Q_UNUSED(level);
    Q_EMIT q->javaScriptConsoleMessage(static_cast<QQuickWebEngineView::JavaScriptConsoleMessageLevel>(level), message, lineNumber, sourceID);
}

void QQuickWebEngineViewPrivate::runMediaAccessPermissionRequest(const QUrl &securityOrigin, WebContentsAdapterClient::MediaRequestFlags requestFlags)
{
   if (!requestFlags)
       return;
   QQuickWebEngineViewExperimental::Feature feature;
   if (requestFlags.testFlag(WebContentsAdapterClient::MediaAudioCapture) && requestFlags.testFlag(WebContentsAdapterClient::MediaAudioCapture))
       feature = QQuickWebEngineViewExperimental::MediaAudioVideoDevices;
   else if (requestFlags.testFlag(WebContentsAdapterClient::MediaAudioCapture))
       feature = QQuickWebEngineViewExperimental::MediaAudioDevices;
   else if (requestFlags.testFlag(WebContentsAdapterClient::MediaVideoCapture))
       feature = QQuickWebEngineViewExperimental::MediaVideoDevices;
   Q_EMIT e->featurePermissionRequested(securityOrigin, feature);
}

void QQuickWebEngineViewPrivate::setDevicePixelRatio(qreal devicePixelRatio)
{
    this->devicePixelRatio = devicePixelRatio;
    QScreen *screen = window ? window->screen() : QGuiApplication::primaryScreen();
    m_dpiScale = devicePixelRatio / screen->devicePixelRatio();
}

void QQuickWebEngineViewPrivate::adoptWebContents(WebContentsAdapter *webContents)
{
    if (!webContents) {
        qWarning("Trying to open an empty request, it was either already used or was invalidated."
            "\nYou must complete the request synchronously within the newViewRequested signal handler."
            " If a view hasn't been adopted before returning, the request will be invalidated.");
        return;
    }

    Q_Q(QQuickWebEngineView);
    // This throws away the WebContentsAdapter that has been used until now.
    // All its states, particularly the loading URL, are replaced by the adopted WebContentsAdapter.
    adapter = webContents;
    adapter->initialize(this);

    // Emit signals for values that might be different from the previous WebContentsAdapter.
    emit q->titleChanged();
    emit q->urlChanged();
    emit q->iconChanged();
    // FIXME: The current loading state should be stored in the WebContentAdapter
    // and it should be checked here if the signal emission is really necessary.
    QQuickWebEngineLoadRequest loadRequest(adapter->activeUrl(), QQuickWebEngineView::LoadSucceededStatus);
    emit q->loadingChanged(&loadRequest);
    emit q->loadProgressChanged();
}

QQuickWebEngineView::QQuickWebEngineView(QQuickItem *parent)
    : QQuickItem(*(new QQuickWebEngineViewPrivate), parent)
{
    Q_D(QQuickWebEngineView);
    d->e->q_ptr = this;
    d->adapter->initialize(d);
    this->setFocus(true);
    this->setActiveFocusOnTab(true);
    this->setFlag(QQuickItem::ItemIsFocusScope);
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
    if (url.isEmpty())
        return;

    Q_D(QQuickWebEngineView);
    d->adapter->load(url);
}

QUrl QQuickWebEngineView::icon() const
{
    Q_D(const QQuickWebEngineView);
    return d->icon;
}

void QQuickWebEngineView::loadHtml(const QString &html, const QUrl &baseUrl, const QUrl &unreachableUrl)
{
    Q_D(QQuickWebEngineView);
    d->adapter->setContent(html.toUtf8(), QStringLiteral("text/html;charset=UTF-8"), baseUrl, unreachableUrl);
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

void QQuickWebEngineViewPrivate::didRunJavaScript(quint64 requestId, const QVariant &result)
{
    QJSValue callback = m_callbacks.take(requestId);
    QJSValueList args;
    args.append(callback.engine()->toScriptValue(result));
    callback.call(args);
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

void QQuickWebEngineView::forceActiveFocus()
{
    Q_FOREACH (QQuickItem *child, childItems()) {
        if (qobject_cast<RenderWidgetHostViewQtDelegateQuick *>(child)) {
            child->forceActiveFocus();
            break;
        }
    }
}

void QQuickWebEngineViewExperimental::setIsFullScreen(bool fullscreen)
{
    d_ptr->m_isFullScreen = fullscreen;
    emit isFullScreenChanged();
}

bool QQuickWebEngineViewExperimental::isFullScreen() const
{
    return d_ptr->m_isFullScreen;
}

void QQuickWebEngineViewExperimental::setExtraContextMenuEntriesComponent(QQmlComponent *contextMenuExtras)
{
    if (d_ptr->contextMenuExtraItems == contextMenuExtras)
        return;
    d_ptr->contextMenuExtraItems = contextMenuExtras;
    emit extraContextMenuEntriesComponentChanged();
}

QQmlComponent *QQuickWebEngineViewExperimental::extraContextMenuEntriesComponent() const
{
    return d_ptr->contextMenuExtraItems;
}

void QQuickWebEngineViewExperimental::runJavaScript(const QString &script, const QJSValue &callback)
{
    if (!callback.isUndefined()) {
        quint64 requestId = d_ptr->adapter->runJavaScriptCallbackResult(script, /*xPath=*/QString());
        d_ptr->m_callbacks.insert(requestId, callback);
    } else
        d_ptr->adapter->runJavaScript(script, /*xPath=*/QString());
}

QQuickWebEngineHistory *QQuickWebEngineViewExperimental::navigationHistory() const
{
    return d_ptr->m_history.data();
}

void QQuickWebEngineViewExperimental::grantFeaturePermission(const QUrl &securityOrigin, QQuickWebEngineViewExperimental::Feature feature, bool granted)
{
    if (!granted && feature >= MediaAudioDevices && feature <= MediaAudioVideoDevices) {
         d_ptr->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaNone);
         return;
    }

    switch (feature) {
    case MediaAudioDevices:
        d_ptr->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaAudioCapture);
        break;
    case MediaVideoDevices:
        d_ptr->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaVideoCapture);
        break;
    case MediaAudioVideoDevices:
        d_ptr->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaRequestFlags(WebContentsAdapterClient::MediaAudioCapture
                                                                                                               | WebContentsAdapterClient::MediaVideoCapture));
        break;
    }
}

void QQuickWebEngineViewExperimental::goBackTo(int index)
{
    int count = d_ptr->adapter->currentNavigationEntryIndex();
    if (index < 0 || index >= count)
        return;

    d_ptr->adapter->navigateToIndex(count - 1 - index);
}

void QQuickWebEngineViewExperimental::goForwardTo(int index)
{
    int count = d_ptr->adapter->navigationEntryCount() - d_ptr->adapter->currentNavigationEntryIndex() - 1;
    if (index < 0 || index >= count)
        return;

    d_ptr->adapter->navigateToIndex(d_ptr->adapter->currentNavigationEntryIndex() + 1 + index);
}

void QQuickWebEngineView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    Q_FOREACH(QQuickItem *child, childItems()) {
        if (qobject_cast<RenderWidgetHostViewQtDelegateQuick *>(child))
            child->setSize(newGeometry.size());
    }
}

void QQuickWebEngineView::itemChange(ItemChange change, const ItemChangeData &value)
{
    Q_D(QQuickWebEngineView);
    if (change == ItemSceneChange || change == ItemVisibleHasChanged) {
        if (window() && isVisible())
            d->adapter->wasShown();
        else
            d->adapter->wasHidden();
    }
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

QT_END_NAMESPACE

#include "moc_qquickwebengineview_p.cpp"
#include "moc_qquickwebengineview_p_p.cpp"
