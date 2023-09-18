// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwebengineaction_p.h"
#include "qquickwebengineaction_p_p.h"
#include "qquickwebengineclientcertificateselection_p.h"
#include "qquickwebenginedialogrequests_p.h"
#include "qquickwebenginefaviconprovider_p_p.h"
#include "qquickwebenginenewwindowrequest_p.h"
#include "qquickwebengineprofile.h"
#include "qquickwebengineprofile_p.h"
#include "qquickwebenginescriptcollection_p.h"
#include "qquickwebenginescriptcollection_p_p.h"
#include "qquickwebenginesettings_p.h"
#include "qquickwebenginetouchhandleprovider_p_p.h"
#include "qquickwebenginetouchhandle_p.h"
#include "qquickwebenginetouchselectionmenurequest_p.h"
#include "qquickwebengineview_p.h"
#include "qquickwebengineview_p_p.h"

#include "authentication_dialog_controller.h"
#include "autofill_popup_controller.h"
#include "profile_adapter.h"
#include "file_picker_controller.h"
#include "find_text_helper.h"
#include "javascript_dialog_controller.h"
#include "render_widget_host_view_qt_delegate_item.h"
#include "render_widget_host_view_qt_delegate_quickwindow.h"
#include "touch_selection_menu_controller.h"
#include "ui_delegates_manager.h"
#include "web_contents_adapter.h"

#include <QtWebEngineCore/qwebenginecertificateerror.h>
#include <QtWebEngineCore/qwebenginefilesystemaccessrequest.h>
#include <QtWebEngineCore/qwebenginefindtextresult.h>
#include <QtWebEngineCore/qwebenginefullscreenrequest.h>
#include <QtWebEngineCore/qwebengineloadinginfo.h>
#include <QtWebEngineCore/qwebenginenavigationrequest.h>
#include <QtWebEngineCore/qwebenginepage.h>
#include <QtWebEngineCore/qwebengineregisterprotocolhandlerrequest.h>
#include <QtWebEngineCore/qwebenginescriptcollection.h>
#include <QtWebEngineCore/private/qwebenginecontextmenurequest_p.h>
#include <QtWebEngineCore/private/qwebenginehistory_p.h>
#include <QtWebEngineCore/private/qwebenginenewwindowrequest_p.h>
#include <QtWebEngineCore/private/qwebenginescriptcollection_p.h>
#include <QtWebEngineCore/private/qwebenginepage_p.h>
#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>

#include <QtCore/qloggingcategory.h>
#include <QtCore/qmimedata.h>
#include <QtCore/qurl.h>
#include <QtCore/qtimer.h>
#include <QtGui/qclipboard.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformintegration.h>
#include <QtQml/qqmlcomponent.h>
#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlproperty.h>

#if QT_CONFIG(accessibility)
#include "qquickwebengine_accessible.h"

#include <QtGui/qaccessible.h>
#endif

#if QT_CONFIG(webengine_printing_and_pdf)
#include <QtCore/qmargins.h>
#include <QtGui/qpagelayout.h>
#include <QtGui/qpageranges.h>
#include <QtGui/qpagesize.h>
#endif

#if QT_CONFIG(webengine_webchannel)
#include <QtWebChannelQuick/qqmlwebchannel.h>
#endif

QT_BEGIN_NAMESPACE
using namespace QtWebEngineCore;

Q_STATIC_ASSERT(int(QQuickWebEngineView::WebActionCount) == int(QWebEnginePage::WebActionCount));
using LoadStatus = QWebEngineLoadingInfo::LoadStatus;
using ErrorDomain = QWebEngineLoadingInfo::ErrorDomain;
#if QT_DEPRECATED_SINCE(6, 2)
QT_WARNING_PUSH QT_WARNING_DISABLE_DEPRECATED
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::AcceptRequest)            == static_cast<int>(QWebEngineNavigationRequest::AcceptRequest));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::IgnoreRequest)            == static_cast<int>(QWebEngineNavigationRequest::IgnoreRequest));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::LinkClickedNavigation)    == static_cast<int>(QWebEngineNavigationRequest::LinkClickedNavigation));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::TypedNavigation)          == static_cast<int>(QWebEngineNavigationRequest::TypedNavigation));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::FormSubmittedNavigation)  == static_cast<int>(QWebEngineNavigationRequest::FormSubmittedNavigation));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::BackForwardNavigation)    == static_cast<int>(QWebEngineNavigationRequest::BackForwardNavigation));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::ReloadNavigation)         == static_cast<int>(QWebEngineNavigationRequest::ReloadNavigation));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::OtherNavigation)          == static_cast<int>(QWebEngineNavigationRequest::OtherNavigation));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::RedirectNavigation)       == static_cast<int>(QWebEngineNavigationRequest::RedirectNavigation));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::NewViewInWindow)          == static_cast<int>(QWebEngineNewWindowRequest::InNewWindow));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::NewViewInTab)             == static_cast<int>(QWebEngineNewWindowRequest::InNewTab));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::NewViewInDialog)          == static_cast<int>(QWebEngineNewWindowRequest::InNewDialog));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::NewViewInBackgroundTab)   == static_cast<int>(QWebEngineNewWindowRequest::InNewBackgroundTab));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::NoErrorDomain)          == static_cast<int>(ErrorDomain::NoErrorDomain));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::InternalErrorDomain)    == static_cast<int>(ErrorDomain::InternalErrorDomain));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::ConnectionErrorDomain)  == static_cast<int>(ErrorDomain::ConnectionErrorDomain));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::CertificateErrorDomain) == static_cast<int>(ErrorDomain::CertificateErrorDomain));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::HttpErrorDomain)        == static_cast<int>(ErrorDomain::HttpErrorDomain));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::FtpErrorDomain)         == static_cast<int>(ErrorDomain::FtpErrorDomain));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::DnsErrorDomain)         == static_cast<int>(ErrorDomain::DnsErrorDomain));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::LoadStartedStatus)   == static_cast<int>(LoadStatus::LoadStartedStatus));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::LoadStoppedStatus)   == static_cast<int>(LoadStatus::LoadStoppedStatus));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::LoadFailedStatus)    == static_cast<int>(LoadStatus::LoadFailedStatus));
Q_STATIC_ASSERT(static_cast<int>(QQuickWebEngineView::LoadSucceededStatus) == static_cast<int>(LoadStatus::LoadSucceededStatus));
QT_WARNING_POP
#endif

class WebEngineQuickWidgetDelegate : public QtWebEngineCore::WidgetDelegate
{
public:
    WebEngineQuickWidgetDelegate(QtWebEngineCore::RenderWidgetHostViewQtDelegateItem *item, QQuickWebEngineView *parent)
        : m_contentItem(item)
        , m_parentView(parent)
    {
    }

    ~WebEngineQuickWidgetDelegate() override
    {
        if (m_contentItem)
            m_contentItem->setWidgetDelegate(nullptr);
    }

    void InitAsPopup(const QRect &screenRect) override
    {
        Q_UNUSED(screenRect);
        Q_UNREACHABLE();
    }

    void Bind(WebContentsAdapterClient *client) override
    {
        QQuickWebEngineViewPrivate::bindViewAndDelegateItem(
                static_cast<QQuickWebEngineViewPrivate *>(client), m_contentItem);
    }

    void Unbind() override
    {
        QQuickWebEngineViewPrivate::bindViewAndDelegateItem(nullptr, m_contentItem);
    }

    void Destroy() override
    {
        delete this;
    }

    bool ActiveFocusOnPress() override
    {
        return m_parentView->property("activeFocusOnPress").toBool() || m_parentView->hasActiveFocus();
    }

private:
    QPointer<RenderWidgetHostViewQtDelegateItem> m_contentItem; // deleted by core
    QPointer<QQuickWebEngineView> m_parentView;
};

#if QT_CONFIG(accessibility)
static QAccessibleInterface *webAccessibleFactory(const QString &, QObject *object)
{
    if (QQuickWebEngineView *v = qobject_cast<QQuickWebEngineView*>(object))
        return new QQuickWebEngineViewAccessible(v);
    return nullptr;
}
#endif // QT_CONFIG(accessibility)

static QLatin1String defaultMimeType("text/html;charset=UTF-8");

QQuickWebEngineViewPrivate::QQuickWebEngineViewPrivate()
    : m_profile(nullptr)
    , adapter(QSharedPointer<WebContentsAdapter>::create())
    , m_history(new QWebEngineHistory(new QWebEngineHistoryPrivate(this, [] (const QUrl &url) {
        return QQuickWebEngineFaviconProvider::faviconProviderUrl(url);
    })))
    , contextMenuExtraItems(nullptr)
    , loadProgress(0)
    , m_fullscreenMode(false)
    , isLoading(false)
    , m_activeFocusOnPress(true)
    , m_webChannel(nullptr)
    , m_webChannelWorld(0)
    , m_defaultAudioMuted(false)
    , m_isBeingAdopted(false)
    , m_backgroundColor(Qt::white)
    , m_zoomFactor(1.0)
    , m_profileInitialized(false)
    , m_contextMenuRequest(nullptr)
    , m_touchHandleDelegate(nullptr)
{
    memset(actions, 0, sizeof(actions));

#if QT_CONFIG(accessibility)
    QAccessible::installFactory(&webAccessibleFactory);
#endif // QT_CONFIG(accessibility)
}

QQuickWebEngineViewPrivate::~QQuickWebEngineViewPrivate()
{
    Q_ASSERT(m_profileInitialized);
    m_profile->d_ptr->removeWebContentsAdapterClient(this);
    if (m_faviconProvider)
        m_faviconProvider->detach(q_ptr);
    bindViewAndDelegateItem(this, nullptr);
}

void QQuickWebEngineViewPrivate::initializeProfile()
{
    if (!m_profileInitialized) {
        Q_ASSERT(!adapter->isInitialized());
        m_profileInitialized = true;

        if (!m_profile) {
            m_profile = QQuickWebEngineProfile::defaultProfile();

            // MEMO first ever call to default profile will create one without context
            // it needs something to get qml engine from (and view is created in qml land)
            m_profile->ensureQmlContext(q_ptr);
        }

        m_profile->d_ptr->addWebContentsAdapterClient(this);
        m_settings.reset(new QQuickWebEngineSettings(m_profile->settings()));
        adapter->setClient(this);
    }
}

bool QQuickWebEngineViewPrivate::profileInitialized() const
{
    return m_profileInitialized;
}

void QQuickWebEngineViewPrivate::releaseProfile()
{
    // The profile for this web contents is about to be
    // garbage collected, delete WebContents first and
    // let the QQuickWebEngineView be collected later by gc.
    bindViewAndDelegateItem(this, nullptr);
    q_ptr->d_ptr.reset();
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
    Q_Q(QQuickWebEngineView);
    auto *item = new RenderWidgetHostViewQtDelegateItem(client, /*isPopup = */ false);
    item->setWidgetDelegate(new WebEngineQuickWidgetDelegate(item, q));
    return item;
}

RenderWidgetHostViewQtDelegate *QQuickWebEngineViewPrivate::CreateRenderWidgetHostViewQtDelegateForPopup(RenderWidgetHostViewQtDelegateClient *client)
{
    Q_Q(QQuickWebEngineView);
    const bool hasWindowCapability = QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::MultipleWindows);
    RenderWidgetHostViewQtDelegateItem *quickDelegate = new RenderWidgetHostViewQtDelegateItem(client, /*isPopup = */ true);
    if (hasWindowCapability) {
        RenderWidgetHostViewQtDelegateQuickWindow *wrapperWindow =
                new RenderWidgetHostViewQtDelegateQuickWindow(quickDelegate, q->window());
        quickDelegate->setWidgetDelegate(wrapperWindow);
        wrapperWindow->setVirtualParent(q);
        return quickDelegate;
    }
    quickDelegate->setParentItem(q);
    quickDelegate->setWidgetDelegate(new WebEngineQuickWidgetDelegate(quickDelegate, q));
    quickDelegate->show();
    return quickDelegate;
}

void QQuickWebEngineViewPrivate::contextMenuRequested(QWebEngineContextMenuRequest *request)
{
    Q_Q(QQuickWebEngineView);

    m_contextMenuRequest = request;

    QQmlEngine *engine = qmlEngine(q);

    // TODO: this is a workaround for QTBUG-65044
    if (!engine)
        return;

    // mark the object for gc by creating temporary jsvalue
    // FIXME: we most likely do not need to make any copy here
    auto *r = new QWebEngineContextMenuRequest(
            new QWebEngineContextMenuRequestPrivate(*request->d.data()));
    engine->newQObject(r);
    Q_EMIT q->contextMenuRequested(r);

    if (r->isAccepted())
        return;

    // Assign the WebEngineView as the parent of the menu, so mouse events are properly propagated
    // on OSX.
    QObject *menu = ui()->addMenu(q, QString(), r->position());
    if (!menu)
        return;

    QQuickContextMenuBuilder contextMenuBuilder(r, q, menu);

    // Populate our menu
    contextMenuBuilder.initMenu();

    // FIXME: expose the context menu data as an attached property to make this more useful
    if (contextMenuExtraItems)
        contextMenuBuilder.appendExtraItems(engine);

    // Now fire the popup() method on the top level menu
    ui()->showMenu(menu);
}

void QQuickWebEngineViewPrivate::navigationRequested(int navigationType, const QUrl &url, bool &accepted, bool isMainFrame)
{
    Q_Q(QQuickWebEngineView);
    auto request = new QWebEngineNavigationRequest(url, static_cast<QWebEngineNavigationRequest::NavigationType>(navigationType), isMainFrame);
    qmlEngine(q)->newQObject(request);
    Q_EMIT q->navigationRequested(request);

    accepted = request->isAccepted();
    if (accepted && adapter->findTextHelper()->isFindTextInProgress())
        adapter->findTextHelper()->stopFinding();
}

void QQuickWebEngineViewPrivate::javascriptDialog(QSharedPointer<JavaScriptDialogController> dialog)
{
    Q_Q(QQuickWebEngineView);
    QQuickWebEngineJavaScriptDialogRequest *request = new QQuickWebEngineJavaScriptDialogRequest(dialog);
    // mark the object for gc by creating temporary jsvalue
    qmlEngine(q)->newQObject(request);
    Q_EMIT q->javaScriptDialogRequested(request);
    if (!request->isAccepted())
        ui()->showDialog(dialog);
}

void QQuickWebEngineViewPrivate::allowCertificateError(const QWebEngineCertificateError &error)
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->certificateError(error);
}

void QQuickWebEngineViewPrivate::selectClientCert(
        const QSharedPointer<QtWebEngineCore::ClientCertSelectController> &controller)
{
    Q_Q(QQuickWebEngineView);
    QQuickWebEngineClientCertificateSelection *certSelection = new QQuickWebEngineClientCertificateSelection(controller);
    // mark the object for gc by creating temporary jsvalue
    qmlEngine(q)->newQObject(certSelection);
    Q_EMIT q->selectClientCertificate(certSelection);
}

static QQuickWebEngineView::Feature toFeature(QtWebEngineCore::ProfileAdapter::PermissionType type)
{
    switch (type) {
    case QtWebEngineCore::ProfileAdapter::NotificationPermission:
        return QQuickWebEngineView::Notifications;
    case QtWebEngineCore::ProfileAdapter::GeolocationPermission:
        return QQuickWebEngineView::Geolocation;
    default:
        break;
    }
    Q_UNREACHABLE();
    return QQuickWebEngineView::Feature(-1);
}


void QQuickWebEngineViewPrivate::runFeaturePermissionRequest(QtWebEngineCore::ProfileAdapter::PermissionType permission, const QUrl &url)
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->featurePermissionRequested(url, toFeature(permission));
}

void QQuickWebEngineViewPrivate::showColorDialog(QSharedPointer<ColorChooserController> controller)
{
    Q_Q(QQuickWebEngineView);
    QQuickWebEngineColorDialogRequest *request = new QQuickWebEngineColorDialogRequest(controller);
    // mark the object for gc by creating temporary jsvalue
    qmlEngine(q)->newQObject(request);
    Q_EMIT q->colorDialogRequested(request);
    if (!request->isAccepted())
        ui()->showColorDialog(controller);
}

void QQuickWebEngineViewPrivate::runFileChooser(QSharedPointer<FilePickerController> controller)
{
    Q_Q(QQuickWebEngineView);
    QQuickWebEngineFileDialogRequest *request = new QQuickWebEngineFileDialogRequest(controller);
    // mark the object for gc by creating temporary jsvalue
    qmlEngine(q)->newQObject(request);
    Q_EMIT q->fileDialogRequested(request);
    if (!request->isAccepted())
        ui()->showFilePicker(controller);
}

bool QQuickWebEngineViewPrivate::passOnFocus(bool reverse)
{
    Q_Q(QQuickWebEngineView);
    // The child delegate currently has focus, find the next one from there and give it focus.
    QQuickItem *next = q->scopedFocusItem()->nextItemInFocusChain(!reverse);
    if (next) {
        next->forceActiveFocus(reverse ? Qt::BacktabFocusReason : Qt::TabFocusReason);
        return true;
    }
    return false;
}

void QQuickWebEngineViewPrivate::titleChanged(const QString &title)
{
    Q_Q(QQuickWebEngineView);
    Q_UNUSED(title);
    Q_EMIT q->titleChanged();
}

void QQuickWebEngineViewPrivate::urlChanged()
{
    Q_Q(QQuickWebEngineView);
    QUrl url = adapter->activeUrl();
    if (m_url != url) {
        m_url = url;
        Q_EMIT q->urlChanged();
    }
}

void QQuickWebEngineViewPrivate::iconChanged(const QUrl &url)
{
    Q_Q(QQuickWebEngineView);

    if (iconUrl == QQuickWebEngineFaviconProvider::faviconProviderUrl(url))
        return;

    iconUrl = QQuickWebEngineFaviconProvider::faviconProviderUrl(url);
    m_history->reset();
    QTimer::singleShot(0, q, &QQuickWebEngineView::iconChanged);
}

void QQuickWebEngineViewPrivate::loadProgressChanged(int progress)
{
    Q_Q(QQuickWebEngineView);
    loadProgress = progress;
    QTimer::singleShot(0, q, &QQuickWebEngineView::loadProgressChanged);
}

void QQuickWebEngineViewPrivate::didUpdateTargetURL(const QUrl &hoveredUrl)
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->linkHovered(hoveredUrl);
}

void QQuickWebEngineViewPrivate::selectionChanged()
{
    updateEditActions();
}

void QQuickWebEngineViewPrivate::zoomUpdateIsNeeded()
{
    Q_Q(QQuickWebEngineView);
    q->setZoomFactor(m_zoomFactor);
}

void QQuickWebEngineViewPrivate::recentlyAudibleChanged(bool recentlyAudible)
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->recentlyAudibleChanged(recentlyAudible);
}

void QQuickWebEngineViewPrivate::renderProcessPidChanged(qint64 pid)
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->renderProcessPidChanged(pid);
}

QRectF QQuickWebEngineViewPrivate::viewportRect() const
{
    Q_Q(const QQuickWebEngineView);
    return QRectF(q->x(), q->y(), q->width(), q->height());
}

QColor QQuickWebEngineViewPrivate::backgroundColor() const
{
    return m_backgroundColor;
}

void QQuickWebEngineViewPrivate::loadStarted(QWebEngineLoadingInfo info)
{
    Q_Q(QQuickWebEngineView);
    isLoading = true;
    m_history->reset();
    QTimer::singleShot(0, q, [q, info] () {
        emit q->loadingChanged(info);
    });
}

void QQuickWebEngineViewPrivate::loadCommitted()
{
    m_history->reset();
}

void QQuickWebEngineViewPrivate::loadFinished(QWebEngineLoadingInfo info)
{
    Q_Q(QQuickWebEngineView);
    isLoading = false;
    m_history->reset();
    QTimer::singleShot(0, q, [q, info] () {
        emit q->loadingChanged(info);
    });
    return;
}

void QQuickWebEngineViewPrivate::focusContainer()
{
    Q_Q(QQuickWebEngineView);
    QQuickWindow *window = q->window();
    if (window)
        window->requestActivate();
    q->forceActiveFocus();
}

void QQuickWebEngineViewPrivate::unhandledKeyEvent(QKeyEvent *event)
{
    Q_Q(QQuickWebEngineView);
    if (q->parentItem())
        QCoreApplication::sendEvent(q->parentItem(), event);
}

static QWebEngineNewWindowRequest::DestinationType toDestinationType(WebContentsAdapterClient::WindowOpenDisposition disposition)
{
    switch (disposition) {
    case WebContentsAdapterClient::NewForegroundTabDisposition:
        return QWebEngineNewWindowRequest::InNewTab;
    case WebContentsAdapterClient::NewBackgroundTabDisposition:
        return QWebEngineNewWindowRequest::InNewBackgroundTab;
    case WebContentsAdapterClient::NewPopupDisposition:
        return QWebEngineNewWindowRequest::InNewDialog;
    case WebContentsAdapterClient::NewWindowDisposition:
        return QWebEngineNewWindowRequest::InNewWindow;
    default:
        Q_UNREACHABLE();
    }
}

QSharedPointer<WebContentsAdapter>
QQuickWebEngineViewPrivate::adoptNewWindow(QSharedPointer<WebContentsAdapter> newWebContents,
                                           WindowOpenDisposition disposition, bool userGesture,
                                           const QRect &geometry, const QUrl &targetUrl)
{
    Q_Q(QQuickWebEngineView);
    Q_ASSERT(newWebContents);
    QQuickWebEngineNewWindowRequest request(toDestinationType(disposition), geometry,
                                          targetUrl, userGesture, newWebContents);

    Q_EMIT q->newWindowRequested(&request);

    if (request.d_ptr->isRequestHandled)
        return newWebContents;
    return nullptr;
}

bool QQuickWebEngineViewPrivate::isBeingAdopted()
{
    return false;
}

void QQuickWebEngineViewPrivate::close()
{
    Q_Q(QQuickWebEngineView);
    emit q->windowCloseRequested();
}

void QQuickWebEngineViewPrivate::windowCloseRejected()
{
    Q_Q(QQuickWebEngineView);

    if (Q_UNLIKELY(q->metaObject()->indexOfMethod("windowCloseRejected()") != -1))
        QMetaObject::invokeMethod(q, "windowCloseRejected");
}

void QQuickWebEngineViewPrivate::requestFullScreenMode(const QUrl &origin, bool fullscreen)
{
    Q_Q(QQuickWebEngineView);
    QWebEngineFullScreenRequest request(origin, fullscreen, [q = QPointer(q)] (bool toggleOn) { if (q) q->d_ptr->setFullScreenMode(toggleOn); });
    Q_EMIT q->fullScreenRequested(request);
}

bool QQuickWebEngineViewPrivate::isFullScreenMode() const
{
    return m_fullscreenMode;
}

void QQuickWebEngineViewPrivate::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID)
{
    Q_Q(QQuickWebEngineView);
    if (q->receivers(SIGNAL(javaScriptConsoleMessage(QQuickWebEngineView::JavaScriptConsoleMessageLevel,QString,int,QString))) > 0) {
        Q_EMIT q->javaScriptConsoleMessage(static_cast<QQuickWebEngineView::JavaScriptConsoleMessageLevel>(level), message, lineNumber, sourceID);
        return;
    }

    static QLoggingCategory loggingCategory("js", QtWarningMsg);
    const QByteArray file = sourceID.toUtf8();
    QMessageLogger logger(file.constData(), lineNumber, nullptr, loggingCategory.categoryName());

    switch (level) {
    case JavaScriptConsoleMessageLevel::Info:
        if (loggingCategory.isInfoEnabled())
            logger.info().noquote() << message;
        break;
    case JavaScriptConsoleMessageLevel::Warning:
        if (loggingCategory.isWarningEnabled())
            logger.warning().noquote() << message;
        break;
    case JavaScriptConsoleMessageLevel::Error:
        if (loggingCategory.isCriticalEnabled())
            logger.critical().noquote() << message;
        break;
    }
}

void QQuickWebEngineViewPrivate::authenticationRequired(QSharedPointer<AuthenticationDialogController> controller)
{
    Q_Q(QQuickWebEngineView);
    QQuickWebEngineAuthenticationDialogRequest *request = new QQuickWebEngineAuthenticationDialogRequest(controller);
    // mark the object for gc by creating temporary jsvalue
    qmlEngine(q)->newQObject(request);
    Q_EMIT q->authenticationDialogRequested(request);
    if (!request->isAccepted())
        ui()->showDialog(controller);
}

void QQuickWebEngineViewPrivate::runMediaAccessPermissionRequest(const QUrl &securityOrigin, WebContentsAdapterClient::MediaRequestFlags requestFlags)
{
    Q_Q(QQuickWebEngineView);
    if (!requestFlags)
        return;
    QQuickWebEngineView::Feature feature;
    if (requestFlags.testFlag(WebContentsAdapterClient::MediaAudioCapture) && requestFlags.testFlag(WebContentsAdapterClient::MediaVideoCapture))
        feature = QQuickWebEngineView::MediaAudioVideoCapture;
    else if (requestFlags.testFlag(WebContentsAdapterClient::MediaAudioCapture))
        feature = QQuickWebEngineView::MediaAudioCapture;
    else if (requestFlags.testFlag(WebContentsAdapterClient::MediaVideoCapture))
        feature = QQuickWebEngineView::MediaVideoCapture;
    else if (requestFlags.testFlag(WebContentsAdapterClient::MediaDesktopAudioCapture) &&
             requestFlags.testFlag(WebContentsAdapterClient::MediaDesktopVideoCapture))
        feature = QQuickWebEngineView::DesktopAudioVideoCapture;
    else // if (requestFlags.testFlag(WebContentsAdapterClient::MediaDesktopVideoCapture))
        feature = QQuickWebEngineView::DesktopVideoCapture;
    Q_EMIT q->featurePermissionRequested(securityOrigin, feature);
}

void QQuickWebEngineViewPrivate::runMouseLockPermissionRequest(const QUrl &securityOrigin)
{
    // TODO: Add mouse lock support
    adapter->grantMouseLockPermission(securityOrigin, false);
}

void QQuickWebEngineViewPrivate::runRegisterProtocolHandlerRequest(QWebEngineRegisterProtocolHandlerRequest request)
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->registerProtocolHandlerRequested(request);
}

void QQuickWebEngineViewPrivate::runFileSystemAccessRequest(
        QWebEngineFileSystemAccessRequest request)
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->fileSystemAccessRequested(request);
}

QObject *QQuickWebEngineViewPrivate::accessibilityParentObject()
{
    Q_Q(QQuickWebEngineView);
    return q;
}

ProfileAdapter *QQuickWebEngineViewPrivate::profileAdapter()
{
    return m_profile->d_ptr->profileAdapter();
}

WebContentsAdapter *QQuickWebEngineViewPrivate::webContentsAdapter()
{
    return adapter.data();
}

void QQuickWebEngineViewPrivate::printRequested()
{
    Q_Q(QQuickWebEngineView);
    QTimer::singleShot(0, q, [q]() {
        Q_EMIT q->printRequested();
    });
}

void QQuickWebEngineViewPrivate::findTextFinished(const QWebEngineFindTextResult &result)
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->findTextFinished(result);
}

void QQuickWebEngineViewPrivate::showAutofillPopup(
        QtWebEngineCore::AutofillPopupController *controller, const QRect &bounds,
        bool autoselectFirstSuggestion)
{
    ui()->showAutofillPopup(controller, bounds.bottomLeft(), bounds.width() + 2,
                            autoselectFirstSuggestion);
}

void QQuickWebEngineViewPrivate::hideAutofillPopup()
{
    ui()->hideAutofillPopup();
}

QWebEngineSettings *QQuickWebEngineViewPrivate::webEngineSettings() const
{
    return m_settings->d_ptr.data();
}

const QObject *QQuickWebEngineViewPrivate::holdingQObject() const
{
    Q_Q(const QQuickWebEngineView);
    return q;
}

void QQuickWebEngineViewPrivate::lifecycleStateChanged(LifecycleState state)
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->lifecycleStateChanged(static_cast<QQuickWebEngineView::LifecycleState>(state));
}

void QQuickWebEngineViewPrivate::recommendedStateChanged(LifecycleState state)
{
    Q_Q(QQuickWebEngineView);
    QTimer::singleShot(0, q, [q, state]() {
        Q_EMIT q->recommendedStateChanged(static_cast<QQuickWebEngineView::LifecycleState>(state));
    });
}

void QQuickWebEngineViewPrivate::visibleChanged(bool visible)
{
    Q_UNUSED(visible);
}

class WebContentsAdapterOwner : public QObject
{
public:
    typedef QSharedPointer<QtWebEngineCore::WebContentsAdapter> AdapterPtr;
    WebContentsAdapterOwner(const AdapterPtr &ptr)
        : adapter(ptr)
    {}

private:
    AdapterPtr adapter;
};

bool QQuickWebEngineViewPrivate::adoptWebContents(WebContentsAdapter *webContents)
{
    Q_ASSERT(webContents);
    if (webContents->profileAdapter() && profileAdapter() != webContents->profileAdapter()) {
        qWarning("Can not adopt content from a different WebEngineProfile.");
        return false;
    }

    m_isBeingAdopted = true;

    // This throws away the WebContentsAdapter that has been used until now.
    // All its states, particularly the loading URL, are replaced by the adopted WebContentsAdapter.
    WebContentsAdapterOwner *adapterOwner = new WebContentsAdapterOwner(adapter->sharedFromThis());
    adapterOwner->deleteLater();

    adapter = webContents->sharedFromThis();
    adapter->setClient(this);
    return true;
}

QQuickWebEngineView::QQuickWebEngineView(QQuickItem *parent)
    : QQuickItem(parent)
    , d_ptr(new QQuickWebEngineViewPrivate)
{
    Q_D(QQuickWebEngineView);
    d->q_ptr = this;
    this->setActiveFocusOnTab(true);
    this->setFlags(QQuickItem::ItemIsFocusScope | QQuickItem::ItemAcceptsDrops);

    connect(action(WebAction::Back), &QQuickWebEngineAction::enabledChanged,
            this, &QQuickWebEngineView::canGoBackChanged);
    connect(action(WebAction::Forward), &QQuickWebEngineAction::enabledChanged,
            this, &QQuickWebEngineView::canGoForwardChanged);
}

QQuickWebEngineView::~QQuickWebEngineView()
{
}

void QQuickWebEngineViewPrivate::ensureContentsAdapter()
{
    initializeProfile();
    if (!adapter->isInitialized()) {
        if (!m_html.isEmpty())
            adapter->setContent(m_html.toUtf8(), defaultMimeType, m_url);
        else if (m_url.isValid())
            adapter->load(m_url);
        else
            adapter->loadDefault();
    }

    if (!m_faviconProvider) {
        QQmlEngine *engine = qmlEngine(q_ptr);
        // TODO: this is a workaround for QTBUG-65044
        if (!engine)
            return;
        m_faviconProvider = static_cast<QQuickWebEngineFaviconProvider *>(
                engine->imageProvider(QQuickWebEngineFaviconProvider::identifier()));
        m_faviconProvider->attach(q_ptr);
        Q_ASSERT(m_faviconProvider);
    }
}

void QQuickWebEngineViewPrivate::initializationFinished()
{
    Q_Q(QQuickWebEngineView);

    Q_ASSERT(m_profileInitialized);
    if (m_backgroundColor != Qt::white) {
        adapter->setBackgroundColor(m_backgroundColor);
        emit q->backgroundColorChanged();
    }

    // apply if it was set before first ever navigation already
    q->setZoomFactor(m_zoomFactor);

#if QT_CONFIG(webengine_webchannel)
    if (m_webChannel)
        adapter->setWebChannel(m_webChannel, m_webChannelWorld);
#endif

    if (m_defaultAudioMuted != adapter->isAudioMuted())
        adapter->setAudioMuted(m_defaultAudioMuted);

    if (devToolsView && devToolsView->d_ptr->adapter)
        adapter->openDevToolsFrontend(devToolsView->d_ptr->adapter);

    if (m_scriptCollection)
        m_scriptCollection->d->d->initializationFinished(adapter);

    if (q->window())
        adapter->setVisible(q->isVisible());

    if (!m_isBeingAdopted)
        return;

    // Ideally these would only be emitted if something actually changed.
    emit q->titleChanged();
    emit q->urlChanged();
    emit q->iconChanged();
    emit q->loadingChanged(QWebEngineLoadingInfo(m_url, LoadStatus::LoadSucceededStatus));
    emit q->loadProgressChanged();

    m_isBeingAdopted = false;
}

void QQuickWebEngineViewPrivate::setFullScreenMode(bool fullscreen)
{
    Q_Q(QQuickWebEngineView);
    if (m_fullscreenMode != fullscreen) {
        m_fullscreenMode = fullscreen;
        adapter->changedFullScreen();
        Q_EMIT q->isFullScreenChanged();
    }
}

// static
void QQuickWebEngineViewPrivate::bindViewAndDelegateItem(QQuickWebEngineViewPrivate *viewPrivate,
                                                         RenderWidgetHostViewQtDelegateItem *delegateItem)
{
    auto oldDelegateItem = viewPrivate ? viewPrivate->delegateItem : nullptr;
    auto oldAdapterClient = delegateItem ? delegateItem->m_adapterClient : nullptr;

    auto *oldViewPrivate = static_cast<QQuickWebEngineViewPrivate *>(oldAdapterClient);

    // Change pointers first.

    if (delegateItem && oldViewPrivate != viewPrivate) {
        if (oldViewPrivate)
            oldViewPrivate->delegateItem = nullptr;
        delegateItem->m_adapterClient = viewPrivate;
    }

    if (viewPrivate && oldDelegateItem != delegateItem) {
        if (oldDelegateItem)
            oldDelegateItem->m_adapterClient = nullptr;
        viewPrivate->delegateItem = delegateItem;
    }

    // Then notify.

    if (oldViewPrivate && oldViewPrivate != viewPrivate)
        oldViewPrivate->delegateItemChanged(delegateItem, nullptr);

    if (viewPrivate && oldDelegateItem != delegateItem)
        viewPrivate->delegateItemChanged(oldDelegateItem, delegateItem);
}

void QQuickWebEngineViewPrivate::delegateItemChanged(QtWebEngineCore::RenderWidgetHostViewQtDelegateItem *oldDelegateItem,
                                                     QtWebEngineCore::RenderWidgetHostViewQtDelegateItem *newDelegateItem)
{
    Q_Q(QQuickWebEngineView);

    if (oldDelegateItem) {
        oldDelegateItem->setParentItem(nullptr);
#if QT_CONFIG(accessibility)
        if (!QtWebEngineCore::closingDown()) {
            if (auto iface = QAccessible::queryAccessibleInterface(oldDelegateItem))
                QAccessible::deleteAccessibleInterface(QAccessible::uniqueId(iface));
        }
#endif
    }

    if (newDelegateItem) {
        Q_ASSERT(!QtWebEngineCore::closingDown());
#if QT_CONFIG(accessibility)
        QAccessible::registerAccessibleInterface(new QtWebEngineCore::RenderWidgetHostViewQtDelegateQuickAccessible(newDelegateItem, q));
#endif
        newDelegateItem->setParentItem(q);
        newDelegateItem->setSize(q->boundingRect().size());
        // Focus on creation if the view accepts it
        if (q->activeFocusOnPress())
            newDelegateItem->setFocus(true);
    }
}

void QQuickWebEngineViewPrivate::updateAction(QQuickWebEngineView::WebAction action) const
{
    QQuickWebEngineAction *a = actions[action];
    if (!a)
        return;

    bool enabled = true;

    switch (action) {
    case QQuickWebEngineView::Back:
        enabled = adapter->canGoBack();
        break;
    case QQuickWebEngineView::Forward:
        enabled = adapter->canGoForward();
        break;
    case QQuickWebEngineView::Stop:
        enabled = isLoading;
        break;
    case QQuickWebEngineView::Reload:
    case QQuickWebEngineView::ReloadAndBypassCache:
        enabled = !isLoading;
        break;
    case QQuickWebEngineView::ViewSource:
        enabled = adapter->canViewSource();
        break;
    case QQuickWebEngineView::Cut:
    case QQuickWebEngineView::Copy:
    case QQuickWebEngineView::Unselect:
        enabled = adapter->hasFocusedFrame() && !adapter->selectedText().isEmpty();
        break;
    case QQuickWebEngineView::Paste:
    case QQuickWebEngineView::Undo:
    case QQuickWebEngineView::Redo:
    case QQuickWebEngineView::SelectAll:
    case QQuickWebEngineView::PasteAndMatchStyle:
        enabled = adapter->hasFocusedFrame();
        break;
    default:
        break;
    }

    a->d_ptr->setEnabled(enabled);
}

void QQuickWebEngineViewPrivate::updateNavigationActions()
{
    updateAction(QQuickWebEngineView::Back);
    updateAction(QQuickWebEngineView::Forward);
    updateAction(QQuickWebEngineView::Stop);
    updateAction(QQuickWebEngineView::Reload);
    updateAction(QQuickWebEngineView::ReloadAndBypassCache);
    updateAction(QQuickWebEngineView::ViewSource);
}

void QQuickWebEngineViewPrivate::updateEditActions()
{
    updateAction(QQuickWebEngineView::Cut);
    updateAction(QQuickWebEngineView::Copy);
    updateAction(QQuickWebEngineView::Paste);
    updateAction(QQuickWebEngineView::Undo);
    updateAction(QQuickWebEngineView::Redo);
    updateAction(QQuickWebEngineView::SelectAll);
    updateAction(QQuickWebEngineView::PasteAndMatchStyle);
    updateAction(QQuickWebEngineView::Unselect);
}

QQuickWebEngineScriptCollection *QQuickWebEngineViewPrivate::getUserScripts()
{
    Q_Q(QQuickWebEngineView);
    if (!m_scriptCollection)
        m_scriptCollection.reset(
            new QQuickWebEngineScriptCollection(
                new QQuickWebEngineScriptCollectionPrivate(
                    new QWebEngineScriptCollectionPrivate(
                        profileAdapter()->userResourceController(), adapter))));

    if (!m_scriptCollection->qmlEngine())
        m_scriptCollection->setQmlEngine(qmlEngine(q));

    return m_scriptCollection.data();
}

QUrl QQuickWebEngineView::url() const
{
    Q_D(const QQuickWebEngineView);
        return d->m_url;
}

void QQuickWebEngineView::setUrl(const QUrl& url)
{
    Q_D(QQuickWebEngineView);
    if (url.isEmpty())
        return;

    if (d->m_url != url) {
        d->m_url = url;
        d->m_html.clear();
        emit urlChanged();
    }

    if (d->adapter->isInitialized()) {
        d->adapter->load(url);
    }
}

QUrl QQuickWebEngineView::icon() const
{
    Q_D(const QQuickWebEngineView);
    return d->iconUrl;
}

void QQuickWebEngineView::loadHtml(const QString &html, const QUrl &baseUrl)
{
    Q_D(QQuickWebEngineView);
    d->m_url = baseUrl;
    d->m_html = html;
    if (d->adapter->isInitialized()) {
        d->adapter->setContent(html.toUtf8(), defaultMimeType, baseUrl);
        return;
    }
}

void QQuickWebEngineView::goBack()
{
    Q_D(QQuickWebEngineView);
    d->adapter->navigateBack();
}

void QQuickWebEngineView::goForward()
{
    Q_D(QQuickWebEngineView);
    d->adapter->navigateForward();
}

void QQuickWebEngineView::reload()
{
    Q_D(QQuickWebEngineView);
    d->adapter->reload();
}

void QQuickWebEngineView::reloadAndBypassCache()
{
    Q_D(QQuickWebEngineView);
    d->adapter->reloadAndBypassCache();
}

void QQuickWebEngineView::stop()
{
    Q_D(QQuickWebEngineView);
    d->adapter->stop();
}

void QQuickWebEngineView::setZoomFactor(qreal arg)
{
    Q_D(QQuickWebEngineView);
    if (d->adapter->isInitialized() && !qFuzzyCompare(arg, zoomFactor())) {
        d->adapter->setZoomFactor(arg);
        // MEMO: should reset if factor was not applied due to being invalid
        d->m_zoomFactor = zoomFactor();
        emit zoomFactorChanged(d->m_zoomFactor);
    } else {
        d->m_zoomFactor = arg;
    }
}

QQuickWebEngineProfile *QQuickWebEngineView::profile()
{
    Q_D(QQuickWebEngineView);
    d->initializeProfile();
    return d->m_profile;
}

void QQuickWebEngineView::setProfile(QQuickWebEngineProfile *profile)
{
    Q_D(QQuickWebEngineView);

    if (d->m_profile == profile)
        return;

    if (!d->profileInitialized()) {
        d->m_profile = profile;
        return;
    }

    if (d->m_profile)
        d->m_profile->d_ptr->removeWebContentsAdapterClient(d);

    d->m_profile = profile;
    d->m_profile->d_ptr->addWebContentsAdapterClient(d);
    d->m_settings->setParentSettings(profile->settings());

    d->updateAdapter();
    Q_EMIT profileChanged();
}

QQuickWebEngineSettings *QQuickWebEngineView::settings()
{
    Q_D(QQuickWebEngineView);
    d->initializeProfile();
    return d->m_settings.data();
}

QQuickWebEngineScriptCollection *QQuickWebEngineView::userScripts()
{
    Q_D(QQuickWebEngineView);
    return d->getUserScripts();
}

void QQuickWebEngineViewPrivate::updateAdapter()
{
    // When the profile changes we need to create a new WebContentAdapter and reload the active URL.
    bool wasInitialized = adapter->isInitialized();
    adapter = QSharedPointer<WebContentsAdapter>::create();
    adapter->setClient(this);
    if (wasInitialized) {
        if (!m_html.isEmpty())
            adapter->setContent(m_html.toUtf8(), defaultMimeType, m_url);
        else if (m_url.isValid())
            adapter->load(m_url);
        else
            adapter->loadDefault();
    }
}

bool QQuickWebEngineView::activeFocusOnPress() const
{
    Q_D(const QQuickWebEngineView);
    return d->m_activeFocusOnPress;
}

void QQuickWebEngineViewPrivate::didRunJavaScript(quint64 requestId, const QVariant &result)
{
    Q_Q(QQuickWebEngineView);
    QJSValue callback = m_callbacks.take(requestId);
    QJSValueList args;
    args.append(qmlEngine(q)->toScriptValue(result));
    callback.call(args);
}

void QQuickWebEngineViewPrivate::didPrintPage(quint64 requestId, QSharedPointer<QByteArray> result)
{
    Q_Q(QQuickWebEngineView);
    QJSValue callback = m_callbacks.take(requestId);
    QJSValueList args;
    args.append(qmlEngine(q)->toScriptValue(*(result.data())));
    callback.call(args);
}

void QQuickWebEngineViewPrivate::didPrintPageToPdf(const QString &filePath, bool success)
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->pdfPrintingFinished(filePath, success);
}

void QQuickWebEngineViewPrivate::updateScrollPosition(const QPointF &position)
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->scrollPositionChanged(position);
}

void QQuickWebEngineViewPrivate::updateContentsSize(const QSizeF &size)
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->contentsSizeChanged(size);
}

void QQuickWebEngineViewPrivate::renderProcessTerminated(
        RenderProcessTerminationStatus terminationStatus, int exitCode)
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->renderProcessTerminated(static_cast<QQuickWebEngineView::RenderProcessTerminationStatus>(
                                      renderProcessExitStatus(terminationStatus)), exitCode);
}

void QQuickWebEngineViewPrivate::requestGeometryChange(const QRect &geometry, const QRect &frameGeometry)
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->geometryChangeRequested(geometry, frameGeometry);
}

QObject *QQuickWebEngineViewPrivate::dragSource() const
{
    // QTBUG-57516
    // Fixme: This is just a band-aid workaround.
#if QT_CONFIG(draganddrop)
    return QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::MultipleWindows) ?
                q_ptr : nullptr;
#else
    return nullptr;
#endif
}

bool QQuickWebEngineViewPrivate::isEnabled() const
{
    const Q_Q(QQuickWebEngineView);
    return q->isEnabled();
}

void QQuickWebEngineViewPrivate::setToolTip(const QString &toolTipText)
{
    Q_Q(QQuickWebEngineView);
    QQuickWebEngineTooltipRequest *request = new QQuickWebEngineTooltipRequest(toolTipText, q);
    // mark the object for gc by creating temporary jsvalue
    qmlEngine(q)->newQObject(request);
    Q_EMIT q->tooltipRequested(request);
    if (!request->isAccepted())
        ui()->showToolTip(toolTipText);
}

QtWebEngineCore::TouchHandleDrawableDelegate *
QQuickWebEngineViewPrivate::createTouchHandleDelegate(const QMap<int, QImage> &images)
{
    Q_Q(QQuickWebEngineView);
    // lifecycle managed by Chromium's TouchHandleDrawable
    QQuickWebEngineTouchHandle *handle = new QQuickWebEngineTouchHandle();
    if (m_touchHandleDelegate) {
        QQmlContext *qmlContext = QQmlEngine::contextForObject(q);
        QQmlContext *context = new QQmlContext(qmlContext, handle);
        context->setContextObject(handle);
        QObject *delegate = m_touchHandleDelegate->create(context);
        Q_ASSERT(delegate);
        QQuickItem *item = qobject_cast<QQuickItem *>(delegate);
        item->setParentItem(q);
        handle->setItem(item, false);
    } else {
        QQuickItem *item = ui()->createTouchHandle();
        Q_ASSERT(item);
        QQmlEngine *engine = qmlEngine(item);
        Q_ASSERT(engine);
        QQuickWebEngineTouchHandleProvider *touchHandleProvider =
                static_cast<QQuickWebEngineTouchHandleProvider *>(
                        engine->imageProvider(QQuickWebEngineTouchHandleProvider::identifier()));
        Q_ASSERT(touchHandleProvider);
        touchHandleProvider->init(images);
        handle->setItem(item, true);
    }
    return handle;
}

void QQuickWebEngineViewPrivate::showTouchSelectionMenu(QtWebEngineCore::TouchSelectionMenuController *menuController, const QRect &selectionBounds, const QSize &handleSize)
{
    Q_UNUSED(handleSize);
    Q_Q(QQuickWebEngineView);

    const int kSpacingBetweenButtons = 2;
    const int kMenuButtonMinWidth = 63;
    const int kMenuButtonMinHeight = 38;

    QQuickWebEngineTouchSelectionMenuRequest *request = new QQuickWebEngineTouchSelectionMenuRequest(
                selectionBounds, menuController);
    qmlEngine(q)->newQObject(request);
    Q_EMIT q->touchSelectionMenuRequested(request);

    if (request->isAccepted()) {
        return;
    }

    int buttonCount = menuController->buttonCount();
    if (buttonCount == 1) {
        menuController->runContextMenu();
        return;
    }

    int width = (kSpacingBetweenButtons * (buttonCount + 1)) + (kMenuButtonMinWidth * buttonCount);
    int height = kMenuButtonMinHeight + kSpacingBetweenButtons;
    int x = (selectionBounds.x() + selectionBounds.x() + selectionBounds.width() - width) / 2;
    int y = selectionBounds.y() - height - 2;

    QRect bounds(x, y, width, height);
    ui()->showTouchSelectionMenu(menuController, bounds, kSpacingBetweenButtons);
}

void QQuickWebEngineViewPrivate::hideTouchSelectionMenu()
{
    ui()->hideTouchSelectionMenu();
}

bool QQuickWebEngineView::isLoading() const
{
    Q_D(const QQuickWebEngineView);
    return d->isLoading;
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

void QQuickWebEngineView::runJavaScript(const QString &script, const QJSValue &callback)
{
    runJavaScript(script, QWebEngineScript::MainWorld, callback);
}

void QQuickWebEngineView::runJavaScript(const QString &script, quint32 worldId, const QJSValue &callback)
{
    Q_D(QQuickWebEngineView);
    d->ensureContentsAdapter();
    if (!callback.isUndefined()) {
        quint64 requestId = d_ptr->adapter->runJavaScriptCallbackResult(script, worldId);
        if (requestId) {
            d->m_callbacks.insert(requestId, callback);
        } else {
            callback.call();
        }
    } else
        d->adapter->runJavaScript(script, worldId);
}

qreal QQuickWebEngineView::zoomFactor() const
{
    Q_D(const QQuickWebEngineView);
    if (!d->adapter->isInitialized())
        return d->m_zoomFactor;
    return d->adapter->currentZoomFactor();
}

QColor QQuickWebEngineView::backgroundColor() const
{
    Q_D(const QQuickWebEngineView);
    return d->m_backgroundColor;
}

void QQuickWebEngineView::setBackgroundColor(const QColor &color)
{
    Q_D(QQuickWebEngineView);
    if (color == d->m_backgroundColor)
        return;
    d->m_backgroundColor = color;
    if (d->adapter->isInitialized()) {
        d->adapter->setBackgroundColor(color);
        emit backgroundColorChanged();
    }
}

/*!
    \property QQuickWebEngineView::audioMuted
    \brief The state of whether the current page audio is muted.
    \since 5.7

    The default value is false.
*/
bool QQuickWebEngineView::isAudioMuted() const
{
    const Q_D(QQuickWebEngineView);
    if (d->adapter->isInitialized())
        return d->adapter->isAudioMuted();
    return d->m_defaultAudioMuted;
}

void QQuickWebEngineView::setAudioMuted(bool muted)
{
    Q_D(QQuickWebEngineView);
    bool wasAudioMuted = isAudioMuted();
    d->m_defaultAudioMuted = muted;
    d->adapter->setAudioMuted(muted);
    if (wasAudioMuted != isAudioMuted())
        Q_EMIT audioMutedChanged(muted);
}

bool QQuickWebEngineView::recentlyAudible() const
{
    const Q_D(QQuickWebEngineView);
    return d->adapter->recentlyAudible();
}

qint64 QQuickWebEngineView::renderProcessPid() const
{
    const Q_D(QQuickWebEngineView);
    return d->adapter->renderProcessPid();
}

void QQuickWebEngineView::printToPdf(const QString& filePath, PrintedPageSizeId pageSizeId, PrintedPageOrientation orientation)
{
#if QT_CONFIG(webengine_printing_and_pdf)
    Q_D(QQuickWebEngineView);
    QPageSize layoutSize(static_cast<QPageSize::PageSizeId>(pageSizeId));
    QPageLayout::Orientation layoutOrientation = static_cast<QPageLayout::Orientation>(orientation);
    QPageLayout pageLayout(layoutSize, layoutOrientation, QMarginsF(0.0, 0.0, 0.0, 0.0));
    QPageRanges ranges;
    d->ensureContentsAdapter();
    d->adapter->printToPDF(pageLayout, ranges, filePath);
#else
    Q_UNUSED(filePath);
    Q_UNUSED(pageSizeId);
    Q_UNUSED(orientation);
#endif
}

void QQuickWebEngineView::printToPdf(const QJSValue &callback, PrintedPageSizeId pageSizeId, PrintedPageOrientation orientation)
{
#if QT_CONFIG(webengine_printing_and_pdf)
    Q_D(QQuickWebEngineView);
    QPageSize layoutSize(static_cast<QPageSize::PageSizeId>(pageSizeId));
    QPageLayout::Orientation layoutOrientation = static_cast<QPageLayout::Orientation>(orientation);
    QPageLayout pageLayout(layoutSize, layoutOrientation, QMarginsF(0.0, 0.0, 0.0, 0.0));
    QPageRanges ranges;

    if (callback.isUndefined())
        return;

    d->ensureContentsAdapter();
    quint64 requestId = d->adapter->printToPDFCallbackResult(pageLayout, ranges);
    d->m_callbacks.insert(requestId, callback);
#else
    Q_UNUSED(pageSizeId);
    Q_UNUSED(orientation);

    // Call back with null result.
    QJSValueList args;
    args.append(QJSValue(QByteArray().data()));
    QJSValue callbackCopy = callback;
    callbackCopy.call(args);
#endif
}

void QQuickWebEngineView::replaceMisspelledWord(const QString &replacement)
{
    Q_D(QQuickWebEngineView);
    d->adapter->replaceMisspelling(replacement);
}

bool QQuickWebEngineView::isFullScreen() const
{
    Q_D(const QQuickWebEngineView);
    return d->m_fullscreenMode;
}

void QQuickWebEngineView::findText(const QString &subString, FindFlags options, const QJSValue &callback)
{
    Q_D(QQuickWebEngineView);
    if (!d->adapter->isInitialized())
        return;

    d->adapter->findTextHelper()->startFinding(subString, options & FindCaseSensitively, options & FindBackward, callback);
}

QWebEngineHistory *QQuickWebEngineView::history() const
{
    Q_D(const QQuickWebEngineView);
    return d->m_history.data();
}

QQmlWebChannel *QQuickWebEngineView::webChannel()
{
#if QT_CONFIG(webengine_webchannel)
    Q_D(QQuickWebEngineView);
    if (!d->m_webChannel) {
        d->m_webChannel = new QQmlWebChannel(this);
    }
    return d->m_webChannel;
#endif
    qWarning("WebEngine compiled without webchannel support");
    return nullptr;
}

void QQuickWebEngineView::setWebChannel(QQmlWebChannel *webChannel)
{
#if QT_CONFIG(webengine_webchannel)
    Q_D(QQuickWebEngineView);
    if (d->m_webChannel == webChannel)
        return;
    d->m_webChannel = webChannel;
    if (d->profileInitialized())
        d->adapter->setWebChannel(webChannel, d->m_webChannelWorld);
    Q_EMIT webChannelChanged();
#else
    Q_UNUSED(webChannel);
    qWarning("WebEngine compiled without webchannel support");
#endif
}

uint QQuickWebEngineView::webChannelWorld() const
{
    Q_D(const QQuickWebEngineView);
    return d->m_webChannelWorld;
}

void QQuickWebEngineView::setWebChannelWorld(uint webChannelWorld)
{
#if QT_CONFIG(webengine_webchannel)
    Q_D(QQuickWebEngineView);
    if (d->m_webChannelWorld == webChannelWorld)
        return;
    d->m_webChannelWorld = webChannelWorld;
    if (d->profileInitialized())
        d->adapter->setWebChannel(d->m_webChannel, d->m_webChannelWorld);
    Q_EMIT webChannelWorldChanged(webChannelWorld);
#else
    Q_UNUSED(webChannelWorld);
    qWarning("WebEngine compiled without webchannel support");
#endif
}

QQuickWebEngineView *QQuickWebEngineView::inspectedView() const
{
    Q_D(const QQuickWebEngineView);
    return d->inspectedView;
}

void QQuickWebEngineView::setInspectedView(QQuickWebEngineView *view)
{
    Q_D(QQuickWebEngineView);
    if (d->inspectedView == view)
        return;
    QQuickWebEngineView *oldView = d->inspectedView;
    d->inspectedView = nullptr;
    if (oldView)
        oldView->setDevToolsView(nullptr);
    d->inspectedView = view;
    if (view)
        view->setDevToolsView(this);
    Q_EMIT inspectedViewChanged();
}

QQuickWebEngineView *QQuickWebEngineView::devToolsView() const
{
    Q_D(const QQuickWebEngineView);
    return d->devToolsView;
}

QString QQuickWebEngineView::devToolsId()
{
    Q_D(QQuickWebEngineView);
    d->ensureContentsAdapter();
    return d->adapter->devToolsId();
}

void QQuickWebEngineView::setDevToolsView(QQuickWebEngineView *devToolsView)
{
    Q_D(QQuickWebEngineView);
    if (d->devToolsView == devToolsView)
        return;
    QQuickWebEngineView *oldView = d->devToolsView;
    d->devToolsView = nullptr;
    if (oldView)
        oldView->setInspectedView(nullptr);
    d->devToolsView = devToolsView;
    if (devToolsView)
        devToolsView->setInspectedView(this);
    if (d->profileInitialized() && d->adapter->isInitialized()) {
        if (devToolsView)
            d->adapter->openDevToolsFrontend(devToolsView->d_ptr->adapter);
        else
            d->adapter->closeDevToolsFrontend();
    }
    Q_EMIT devToolsViewChanged();
}

void QQuickWebEngineView::grantFeaturePermission(const QUrl &securityOrigin, QQuickWebEngineView::Feature feature, bool granted)
{
    if (!granted && ((feature >= MediaAudioCapture && feature <= MediaAudioVideoCapture) ||
                     (feature >= DesktopVideoCapture && feature <= DesktopAudioVideoCapture))) {
         d_ptr->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaNone);
         return;
    }

    switch (feature) {
    case MediaAudioCapture:
        d_ptr->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaAudioCapture);
        break;
    case MediaVideoCapture:
        d_ptr->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaVideoCapture);
        break;
    case MediaAudioVideoCapture:
        d_ptr->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaRequestFlags(WebContentsAdapterClient::MediaAudioCapture                                                                                                               | WebContentsAdapterClient::MediaVideoCapture));
        break;
    case DesktopVideoCapture:
        d_ptr->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaDesktopVideoCapture);
        break;
    case DesktopAudioVideoCapture:
        d_ptr->adapter->grantMediaAccessPermission(
            securityOrigin,
            WebContentsAdapterClient::MediaRequestFlags(
                WebContentsAdapterClient::MediaDesktopAudioCapture |
                WebContentsAdapterClient::MediaDesktopVideoCapture));
        break;
    case Geolocation:
        d_ptr->adapter->grantFeaturePermission(securityOrigin, ProfileAdapter::GeolocationPermission,
                                               granted ? ProfileAdapter::AllowedPermission : ProfileAdapter::DeniedPermission);
        break;
    case Notifications:
        d_ptr->adapter->grantFeaturePermission(securityOrigin, ProfileAdapter::NotificationPermission,
                                               granted ? ProfileAdapter::AllowedPermission : ProfileAdapter::DeniedPermission);
        break;
    default:
        Q_UNREACHABLE();
    }
}

void QQuickWebEngineView::setActiveFocusOnPress(bool arg)
{
    Q_D(QQuickWebEngineView);
    if (d->m_activeFocusOnPress == arg)
        return;

    d->m_activeFocusOnPress = arg;
    emit activeFocusOnPressChanged(arg);
}

void QQuickWebEngineView::goBackOrForward(int offset)
{
    Q_D(QQuickWebEngineView);
    const int current = d->adapter->currentNavigationEntryIndex();
    const int count = d->adapter->navigationEntryCount();
    const int index = current + offset;

    if (index < 0 || index >= count)
        return;

    d->adapter->navigateToIndex(index);
}

void QQuickWebEngineView::fullScreenCancelled()
{
    Q_D(QQuickWebEngineView);
    d->adapter->exitFullScreen();
}

void QQuickWebEngineView::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    Q_D(QQuickWebEngineView);
    if (d->delegateItem)
        d->delegateItem->setSize(newGeometry.size());
}

void QQuickWebEngineView::itemChange(ItemChange change, const ItemChangeData &value)
{
    Q_D(QQuickWebEngineView);
    if (d && d->profileInitialized() && d->adapter->isInitialized()
            && (change == ItemSceneChange || change == ItemVisibleHasChanged)) {
        if (window())
            d->adapter->setVisible(isVisible());
    }
    QQuickItem::itemChange(change, value);
}

void QQuickWebEngineView::acceptAsNewWindow(QWebEngineNewWindowRequest *request)
{
    Q_D(QQuickWebEngineView);
    if (!request || (!request->d_ptr->adapter && !request->requestedUrl().isValid())
        || request->d_ptr->isRequestHandled) {
        qWarning("Trying to open an empty request, it was either already used or was invalidated."
            "\nYou must complete the request synchronously within the newWindowRequested signal handler."
            " If a view hasn't been adopted before returning, the request will be invalidated.");
        return;
    }

    auto adapter = request->d_ptr->adapter;
    if (!adapter)
        setUrl(request->requestedUrl());
    else if (!d->adoptWebContents(adapter.data()))
        return;

    request->d_ptr->setHandled();
}

#if QT_CONFIG(draganddrop)
static QPointF mapToScreen(const QQuickItem *item, const QPointF &clientPos)
{
    return item->window()->position() + item->mapToScene(clientPos);
}

void QQuickWebEngineView::dragEnterEvent(QDragEnterEvent *e)
{
    Q_D(QQuickWebEngineView);
    e->accept();
    d->adapter->enterDrag(e, mapToScreen(this, e->position()));
}

void QQuickWebEngineView::dragLeaveEvent(QDragLeaveEvent *e)
{
    Q_D(QQuickWebEngineView);
    e->accept();
    d->adapter->leaveDrag();
}

void QQuickWebEngineView::dragMoveEvent(QDragMoveEvent *e)
{
    Q_D(QQuickWebEngineView);
    Qt::DropAction dropAction = d->adapter->updateDragPosition(e, mapToScreen(this, e->position()));
    if (Qt::IgnoreAction == dropAction) {
        e->ignore();
    } else {
        e->setDropAction(dropAction);
        e->accept();
    }
}

void QQuickWebEngineView::dropEvent(QDropEvent *e)
{
    Q_D(QQuickWebEngineView);
    e->accept();
    d->adapter->endDragging(e, mapToScreen(this, e->position()));
}
#endif // QT_CONFIG(draganddrop)

void QQuickWebEngineView::triggerWebAction(WebAction action)
{
    Q_D(QQuickWebEngineView);
    switch (action) {
    case Back:
        d->adapter->navigateBack();
        break;
    case Forward:
        d->adapter->navigateForward();
        break;
    case Stop:
        d->adapter->stop();
        break;
    case Reload:
        d->adapter->reload();
        break;
    case ReloadAndBypassCache:
        d->adapter->reloadAndBypassCache();
        break;
    case Cut:
        d->adapter->cut();
        break;
    case Copy:
        d->adapter->copy();
        break;
    case Paste:
        d->adapter->paste();
        break;
    case Undo:
        d->adapter->undo();
        break;
    case Redo:
        d->adapter->redo();
        break;
    case SelectAll:
        d->adapter->selectAll();
        break;
    case PasteAndMatchStyle:
        d->adapter->pasteAndMatchStyle();
        break;
    case Unselect:
        d->adapter->unselect();
        break;
    case OpenLinkInThisWindow:
        if (d->m_contextMenuRequest->filteredLinkUrl().isValid())
            setUrl(d->m_contextMenuRequest->filteredLinkUrl());
        break;
    case OpenLinkInNewWindow:
        if (d->m_contextMenuRequest->filteredLinkUrl().isValid()) {
            QQuickWebEngineNewWindowRequest request(QWebEngineNewWindowRequest::InNewWindow, QRect(),
                                                  d->m_contextMenuRequest->filteredLinkUrl(), true, nullptr);
            Q_EMIT newWindowRequested(&request);
        }
        break;
    case OpenLinkInNewTab:
        if (d->m_contextMenuRequest->filteredLinkUrl().isValid()) {
            QQuickWebEngineNewWindowRequest request(QWebEngineNewWindowRequest::InNewBackgroundTab, QRect(),
                                                  d->m_contextMenuRequest->filteredLinkUrl(), true, nullptr);
            Q_EMIT newWindowRequested(&request);
        }
        break;
    case CopyLinkToClipboard:
        if (!d->m_contextMenuRequest->linkUrl().isEmpty()) {
            QString urlString =
                    d->m_contextMenuRequest->linkUrl().toString(QUrl::FullyEncoded);
            QString linkText = d->m_contextMenuRequest->linkText().toHtmlEscaped();
            QString title = d->m_contextMenuRequest->titleText();
            if (!title.isEmpty())
                title = QStringLiteral(" title=\"%1\"").arg(title.toHtmlEscaped());
            QMimeData *data = new QMimeData();
            data->setText(urlString);
            QString html = QStringLiteral("<a href=\"") + urlString + QStringLiteral("\"") + title + QStringLiteral(">")
                         + linkText + QStringLiteral("</a>");
            data->setHtml(html);
            data->setUrls(QList<QUrl>() << d->m_contextMenuRequest->linkUrl());
            qApp->clipboard()->setMimeData(data);
        }
        break;
    case DownloadLinkToDisk:
        if (d->m_contextMenuRequest->filteredLinkUrl().isValid())
            d->adapter->download(d->m_contextMenuRequest->filteredLinkUrl(),
                                 d->m_contextMenuRequest->suggestedFileName(),
                                 d->m_contextMenuRequest->referrerUrl(),
                                 d->m_contextMenuRequest->referrerPolicy());
        break;
    case CopyImageToClipboard:
        if (d->m_contextMenuRequest->hasImageContent()
            && (d->m_contextMenuRequest->mediaType() == QWebEngineContextMenuRequest::MediaTypeImage
                || d->m_contextMenuRequest->mediaType()
                        == QWebEngineContextMenuRequest::MediaTypeCanvas)) {
            d->adapter->copyImageAt(d->m_contextMenuRequest->position());
        }
        break;
    case CopyImageUrlToClipboard:
        if (d->m_contextMenuRequest->mediaUrl().isValid()
            && d->m_contextMenuRequest->mediaType()
                    == QWebEngineContextMenuRequest::MediaTypeImage) {
            QString urlString = d->m_contextMenuRequest->mediaUrl().toString(QUrl::FullyEncoded);
            QString alt = d->m_contextMenuRequest->altText();
            if (!alt.isEmpty())
                alt = QStringLiteral(" alt=\"%1\"").arg(alt.toHtmlEscaped());
            QString title = d->m_contextMenuRequest->titleText();
            if (!title.isEmpty())
                title = QStringLiteral(" title=\"%1\"").arg(title.toHtmlEscaped());
            QMimeData *data = new QMimeData();
            data->setText(urlString);
            QString html = QStringLiteral("<img src=\"") + urlString + QStringLiteral("\"") + title + alt + QStringLiteral("></img>");
            data->setHtml(html);
            data->setUrls(QList<QUrl>() << d->m_contextMenuRequest->mediaUrl());
            qApp->clipboard()->setMimeData(data);
        }
        break;
    case DownloadImageToDisk:
    case DownloadMediaToDisk:
        if (d->m_contextMenuRequest->mediaUrl().isValid())
            d->adapter->download(d->m_contextMenuRequest->mediaUrl(),
                                 d->m_contextMenuRequest->suggestedFileName(),
                                 d->m_contextMenuRequest->referrerUrl(),
                                 d->m_contextMenuRequest->referrerPolicy());
        break;
    case CopyMediaUrlToClipboard:
        if (d->m_contextMenuRequest->mediaUrl().isValid()
            && (d->m_contextMenuRequest->mediaType() == QWebEngineContextMenuRequest::MediaTypeAudio
                || d->m_contextMenuRequest->mediaType()
                        == QWebEngineContextMenuRequest::MediaTypeVideo)) {
            QString urlString = d->m_contextMenuRequest->mediaUrl().toString(QUrl::FullyEncoded);
            QString title = d->m_contextMenuRequest->titleText();
            if (!title.isEmpty())
                title = QStringLiteral(" title=\"%1\"").arg(title.toHtmlEscaped());
            QMimeData *data = new QMimeData();
            data->setText(urlString);
            if (d->m_contextMenuRequest->mediaType()
                == QWebEngineContextMenuRequest::MediaTypeAudio)
                data->setHtml(QStringLiteral("<audio src=\"") + urlString + QStringLiteral("\"") + title +
                              QStringLiteral("></audio>"));
            else
                data->setHtml(QStringLiteral("<video src=\"") + urlString + QStringLiteral("\"") + title +
                              QStringLiteral("></video>"));
            data->setUrls(QList<QUrl>() << d->m_contextMenuRequest->mediaUrl());
            qApp->clipboard()->setMimeData(data);
        }
        break;
    case ToggleMediaControls:
        if (d->m_contextMenuRequest->mediaUrl().isValid()
            && d->m_contextMenuRequest->mediaFlags()
                    & QWebEngineContextMenuRequest::MediaCanToggleControls) {
            bool enable = !(d->m_contextMenuRequest->mediaFlags()
                            & QWebEngineContextMenuRequest::MediaControls);
            d->adapter->executeMediaPlayerActionAt(d->m_contextMenuRequest->position(),
                                                   WebContentsAdapter::MediaPlayerControls, enable);
        }
        break;
    case ToggleMediaLoop:
        if (d->m_contextMenuRequest->mediaUrl().isValid()
            && (d->m_contextMenuRequest->mediaType() == QWebEngineContextMenuRequest::MediaTypeAudio
                || d->m_contextMenuRequest->mediaType()
                        == QWebEngineContextMenuRequest::MediaTypeVideo)) {
            bool enable = !(d->m_contextMenuRequest->mediaFlags()
                            & QWebEngineContextMenuRequest::MediaLoop);
            d->adapter->executeMediaPlayerActionAt(d->m_contextMenuRequest->position(),
                                                   WebContentsAdapter::MediaPlayerLoop, enable);
        }
        break;
    case ToggleMediaPlayPause:
        if (d->m_contextMenuRequest->mediaUrl().isValid()
            && (d->m_contextMenuRequest->mediaType() == QWebEngineContextMenuRequest::MediaTypeAudio
                || d->m_contextMenuRequest->mediaType()
                        == QWebEngineContextMenuRequest::MediaTypeVideo)) {
            bool enable = (d->m_contextMenuRequest->mediaFlags()
                           & QWebEngineContextMenuRequest::MediaPaused);
            d->adapter->executeMediaPlayerActionAt(d->m_contextMenuRequest->position(),
                                                   WebContentsAdapter::MediaPlayerPlay, enable);
        }
        break;
    case ToggleMediaMute:
        if (d->m_contextMenuRequest->mediaUrl().isValid()
            && d->m_contextMenuRequest->mediaFlags()
                    & QWebEngineContextMenuRequest::MediaHasAudio) {
            bool enable = !(d->m_contextMenuRequest->mediaFlags()
                            & QWebEngineContextMenuRequest::MediaMuted);
            d->adapter->executeMediaPlayerActionAt(d->m_contextMenuRequest->position(),
                                                   WebContentsAdapter::MediaPlayerMute, enable);
        }
        break;
    case InspectElement:
        d->adapter->inspectElementAt(d->m_contextMenuRequest->position());
        break;
    case ExitFullScreen:
        d->adapter->exitFullScreen();
        break;
    case RequestClose:
        d->adapter->requestClose();
        break;
    case SavePage:
        d->adapter->save();
        break;
    case ViewSource:
        d->adapter->viewSource();
        break;
    case ToggleBold:
        runJavaScript(QStringLiteral("document.execCommand('bold');"), QWebEngineScript::ApplicationWorld);
        break;
    case ToggleItalic:
        runJavaScript(QStringLiteral("document.execCommand('italic');"), QWebEngineScript::ApplicationWorld);
        break;
    case ToggleUnderline:
        runJavaScript(QStringLiteral("document.execCommand('underline');"), QWebEngineScript::ApplicationWorld);
        break;
    case ToggleStrikethrough:
        runJavaScript(QStringLiteral("document.execCommand('strikethrough');"), QWebEngineScript::ApplicationWorld);
        break;
    case AlignLeft:
        runJavaScript(QStringLiteral("document.execCommand('justifyLeft');"), QWebEngineScript::ApplicationWorld);
        break;
    case AlignCenter:
        runJavaScript(QStringLiteral("document.execCommand('justifyCenter');"), QWebEngineScript::ApplicationWorld);
        break;
    case AlignRight:
        runJavaScript(QStringLiteral("document.execCommand('justifyRight');"), QWebEngineScript::ApplicationWorld);
        break;
    case AlignJustified:
        runJavaScript(QStringLiteral("document.execCommand('justifyFull');"), QWebEngineScript::ApplicationWorld);
        break;
    case Indent:
        runJavaScript(QStringLiteral("document.execCommand('indent');"), QWebEngineScript::ApplicationWorld);
        break;
    case Outdent:
        runJavaScript(QStringLiteral("document.execCommand('outdent');"), QWebEngineScript::ApplicationWorld);
        break;
    case InsertOrderedList:
        runJavaScript(QStringLiteral("document.execCommand('insertOrderedList');"), QWebEngineScript::ApplicationWorld);
        break;
    case InsertUnorderedList:
        runJavaScript(QStringLiteral("document.execCommand('insertUnorderedList');"), QWebEngineScript::ApplicationWorld);
        break;
    case ChangeTextDirectionLTR:
        d->adapter->changeTextDirection(true /*left to right*/);
        break;
    case ChangeTextDirectionRTL:
        d->adapter->changeTextDirection(false /*left to right*/);
        break;
    default:
        Q_UNREACHABLE();
    }
}

QQuickWebEngineAction *QQuickWebEngineView::action(WebAction action)
{
    Q_D(QQuickWebEngineView);
    if (action == QQuickWebEngineView::NoWebAction)
        return nullptr;
    if (d->actions[action]) {
        d->updateAction(action);
        return d->actions[action];
    }

    const QString text = QWebEnginePagePrivate::actionText(action);
    QString iconName;

    switch (action) {
    case Back:
        iconName = QStringLiteral("go-previous");
        break;
    case Forward:
        iconName = QStringLiteral("go-next");
        break;
    case Stop:
        iconName = QStringLiteral("process-stop");
        break;
    case Reload:
        iconName = QStringLiteral("view-refresh");
        break;
    case ReloadAndBypassCache:
        iconName = QStringLiteral("view-refresh");
        break;
    case Cut:
        iconName = QStringLiteral("edit-cut");
        break;
    case Copy:
        iconName = QStringLiteral("edit-copy");
        break;
    case Paste:
        iconName = QStringLiteral("edit-paste");
        break;
    case Undo:
        iconName = QStringLiteral("edit-undo");
        break;
    case Redo:
        iconName = QStringLiteral("edit-redo");
        break;
    case SelectAll:
        iconName = QStringLiteral("edit-select-all");
        break;
    case PasteAndMatchStyle:
        iconName = QStringLiteral("edit-paste");
        break;
    case OpenLinkInThisWindow:
    case OpenLinkInNewWindow:
    case OpenLinkInNewTab:
    case CopyLinkToClipboard:
    case DownloadLinkToDisk:
    case CopyImageToClipboard:
    case CopyImageUrlToClipboard:
    case DownloadImageToDisk:
    case CopyMediaUrlToClipboard:
    case ToggleMediaControls:
    case ToggleMediaLoop:
        break;
    case ToggleMediaPlayPause:
        iconName = QStringLiteral("media-playback-start");
        break;
    case ToggleMediaMute:
        iconName = QStringLiteral("audio-volume-muted");
        break;
    case DownloadMediaToDisk:
    case InspectElement:
        break;
    case ExitFullScreen:
        iconName = QStringLiteral("view-fullscreen");
        break;
    case RequestClose:
        iconName = QStringLiteral("window-close");
        break;
    case Unselect:
        iconName = QStringLiteral("edit-select-none");
        break;
    case SavePage:
        iconName = QStringLiteral("document-save");
        break;
    case OpenLinkInNewBackgroundTab:
        break;
    case ViewSource:
        break;
    case ToggleBold:
        iconName = QStringLiteral("format-text-bold");
        break;
    case ToggleItalic:
        iconName = QStringLiteral("format-text-italic");
        break;
    case ToggleUnderline:
        iconName = QStringLiteral("format-text-underline");
        break;
    case ToggleStrikethrough:
        iconName = QStringLiteral("format-text-strikethrough");
        break;
    case AlignLeft:
        break;
    case AlignCenter:
        break;
    case AlignRight:
        break;
    case AlignJustified:
        break;
    case Indent:
        iconName = QStringLiteral("format-indent-more");
        break;
    case Outdent:
        iconName = QStringLiteral("format-indent-less");
        break;
    case InsertOrderedList:
    case InsertUnorderedList:
    case ChangeTextDirectionLTR:
    case ChangeTextDirectionRTL:
        break;
    case NoWebAction:
    case WebActionCount:
        Q_UNREACHABLE();
        break;
    }

    QQuickWebEngineAction *retVal = new QQuickWebEngineAction(action, text, iconName, false, this);

    d->actions[action] = retVal;
    d->updateAction(action);
    return retVal;
}

QSizeF QQuickWebEngineView::contentsSize() const
{
    Q_D(const QQuickWebEngineView);
    return d->adapter->lastContentsSize();
}

QPointF QQuickWebEngineView::scrollPosition() const
{
    Q_D(const QQuickWebEngineView);
    return d->adapter->lastScrollOffset();
}

void QQuickWebEngineView::componentComplete()
{
    QQuickItem::componentComplete();
    Q_D(QQuickWebEngineView);
    d->initializeProfile();
#if QT_CONFIG(accessibility)
    // Enable accessibility via a dynamic QQmlProperty, instead of using private API call
    // QQuickAccessibleAttached::qmlAttachedProperties(this). The qmlContext is required, otherwise
    // it is not possible to reference attached properties.
    QQmlContext *qmlContext = QQmlEngine::contextForObject(this);
    QQmlProperty role(this, QStringLiteral("Accessible.role"), qmlContext);
    role.write(QAccessible::Grouping);
#endif // QT_CONFIG(accessibility)

    QTimer::singleShot(0, this, &QQuickWebEngineView::lazyInitialize);
}

void QQuickWebEngineView::lazyInitialize()
{
    Q_D(QQuickWebEngineView);
    d->ensureContentsAdapter();
}

QQuickWebEngineView::LifecycleState QQuickWebEngineView::lifecycleState() const
{
    Q_D(const QQuickWebEngineView);
    return static_cast<LifecycleState>(d->adapter->lifecycleState());
}

void QQuickWebEngineView::setLifecycleState(LifecycleState state)
{
    Q_D(QQuickWebEngineView);
    d->adapter->setLifecycleState(static_cast<WebContentsAdapterClient::LifecycleState>(state));
}

QQuickWebEngineView::LifecycleState QQuickWebEngineView::recommendedState() const
{
    Q_D(const QQuickWebEngineView);
    return static_cast<LifecycleState>(d->adapter->recommendedState());
}

QQuickContextMenuBuilder::QQuickContextMenuBuilder(QWebEngineContextMenuRequest *request,
                                                   QQuickWebEngineView *view, QObject *menu)
    : QtWebEngineCore::RenderViewContextMenuQt(request), m_view(view), m_menu(menu)
{
}

void QQuickContextMenuBuilder::appendExtraItems(QQmlEngine *engine)
{
    Q_UNUSED(engine);
    m_view->d_ptr->ui()->addMenuSeparator(m_menu);
    if (QObject *menuExtras = m_view->d_ptr->contextMenuExtraItems->create(qmlContext(m_view))) {
        menuExtras->setParent(m_menu);
        QQmlListReference entries(m_menu, defaultPropertyName(m_menu));
        if (entries.isValid())
            entries.append(menuExtras);
    }
}

bool QQuickContextMenuBuilder::hasInspector()
{
    return m_view->d_ptr->adapter->hasInspector();
}

bool QQuickContextMenuBuilder::isFullScreenMode()
{
    return m_view->d_ptr->isFullScreenMode();
}

void QQuickContextMenuBuilder::addMenuItem(ContextMenuItem menuItem)
{
    QQuickWebEngineAction *action = nullptr;

    switch (menuItem) {
    case ContextMenuItem::Back:
        action = m_view->action(QQuickWebEngineView::Back);
        break;
    case ContextMenuItem::Forward:
        action = m_view->action(QQuickWebEngineView::Forward);
        break;
    case ContextMenuItem::Reload:
        action = m_view->action(QQuickWebEngineView::Reload);
        break;
    case ContextMenuItem::Cut:
        action = m_view->action(QQuickWebEngineView::Cut);
        break;
    case ContextMenuItem::Copy:
        action = m_view->action(QQuickWebEngineView::Copy);
        break;
    case ContextMenuItem::Paste:
        action = m_view->action(QQuickWebEngineView::Paste);
        break;
    case ContextMenuItem::Undo:
        action = m_view->action(QQuickWebEngineView::Undo);
        break;
    case ContextMenuItem::Redo:
        action = m_view->action(QQuickWebEngineView::Redo);
        break;
    case ContextMenuItem::SelectAll:
        action = m_view->action(QQuickWebEngineView::SelectAll);
        break;
    case ContextMenuItem::PasteAndMatchStyle:
        action = m_view->action(QQuickWebEngineView::PasteAndMatchStyle);
        break;
    case ContextMenuItem::OpenLinkInNewWindow:
        action = m_view->action(QQuickWebEngineView::OpenLinkInNewWindow);
        break;
    case ContextMenuItem::OpenLinkInNewTab:
        action = m_view->action(QQuickWebEngineView::OpenLinkInNewTab);
        break;
    case ContextMenuItem::CopyLinkToClipboard:
        action = m_view->action(QQuickWebEngineView::CopyLinkToClipboard);
        break;
    case ContextMenuItem::DownloadLinkToDisk:
        action = m_view->action(QQuickWebEngineView::DownloadLinkToDisk);
        break;
    case ContextMenuItem::CopyImageToClipboard:
        action = m_view->action(QQuickWebEngineView::CopyImageToClipboard);
        break;
    case ContextMenuItem::CopyImageUrlToClipboard:
        action = m_view->action(QQuickWebEngineView::CopyImageUrlToClipboard);
        break;
    case ContextMenuItem::DownloadImageToDisk:
        action = m_view->action(QQuickWebEngineView::DownloadImageToDisk);
        break;
    case ContextMenuItem::CopyMediaUrlToClipboard:
        action = m_view->action(QQuickWebEngineView::CopyMediaUrlToClipboard);
        break;
    case ContextMenuItem::ToggleMediaControls:
        action = m_view->action(QQuickWebEngineView::ToggleMediaControls);
        break;
    case ContextMenuItem::ToggleMediaLoop:
        action = m_view->action(QQuickWebEngineView::ToggleMediaLoop);
        break;
    case ContextMenuItem::DownloadMediaToDisk:
        action = m_view->action(QQuickWebEngineView::DownloadMediaToDisk);
        break;
    case ContextMenuItem::InspectElement:
        action = m_view->action(QQuickWebEngineView::InspectElement);
        break;
    case ContextMenuItem::ExitFullScreen:
        action = m_view->action(QQuickWebEngineView::ExitFullScreen);
        break;
    case ContextMenuItem::SavePage:
        action = m_view->action(QQuickWebEngineView::SavePage);
        break;
    case ContextMenuItem::ViewSource:
        action = m_view->action(QQuickWebEngineView::ViewSource);
        break;
    case ContextMenuItem::SpellingSuggestions:
    {
        QPointer<QQuickWebEngineView> thisRef(m_view);
        for (int i = 0; i < m_contextData->spellCheckerSuggestions().size() && i < 4; i++) {
            action = new QQuickWebEngineAction(m_menu);
            QString replacement = m_contextData->spellCheckerSuggestions().at(i);
            QObject::connect(action, &QQuickWebEngineAction::triggered, [thisRef, replacement] { thisRef->replaceMisspelledWord(replacement); });
            m_view->d_ptr->ui()->addMenuItem(action, m_menu);
        }
        return;
    }
    case ContextMenuItem::Separator:
        m_view->d_ptr->ui()->addMenuSeparator(m_menu);
        return;
    }
    // Set enabled property directly with avoiding binding loops caused by its notifier signal.
    action->d_ptr->m_enabled = isMenuItemEnabled(menuItem);
    m_view->d_ptr->ui()->addMenuItem(action, m_menu);
}

bool QQuickContextMenuBuilder::isMenuItemEnabled(ContextMenuItem menuItem)
{
    switch (menuItem) {
    case ContextMenuItem::Back:
        return m_view->canGoBack();
    case ContextMenuItem::Forward:
        return m_view->canGoForward();
    case ContextMenuItem::Reload:
        return true;
    case ContextMenuItem::Cut:
        return m_contextData->editFlags() & QWebEngineContextMenuRequest::CanCut;
    case ContextMenuItem::Copy:
        return m_contextData->editFlags() & QWebEngineContextMenuRequest::CanCopy;
    case ContextMenuItem::Paste:
        return m_contextData->editFlags() & QWebEngineContextMenuRequest::CanPaste;
    case ContextMenuItem::Undo:
        return m_contextData->editFlags() & QWebEngineContextMenuRequest::CanUndo;
    case ContextMenuItem::Redo:
        return m_contextData->editFlags() & QWebEngineContextMenuRequest::CanRedo;
    case ContextMenuItem::SelectAll:
        return m_contextData->editFlags() & QWebEngineContextMenuRequest::CanSelectAll;
    case ContextMenuItem::PasteAndMatchStyle:
        return m_contextData->editFlags() & QWebEngineContextMenuRequest::CanPaste;
    case ContextMenuItem::OpenLinkInNewWindow:
    case ContextMenuItem::OpenLinkInNewTab:
    case ContextMenuItem::CopyLinkToClipboard:
    case ContextMenuItem::DownloadLinkToDisk:
    case ContextMenuItem::CopyImageToClipboard:
    case ContextMenuItem::CopyImageUrlToClipboard:
    case ContextMenuItem::DownloadImageToDisk:
    case ContextMenuItem::CopyMediaUrlToClipboard:
    case ContextMenuItem::ToggleMediaControls:
    case ContextMenuItem::ToggleMediaLoop:
    case ContextMenuItem::DownloadMediaToDisk:
    case ContextMenuItem::InspectElement:
    case ContextMenuItem::ExitFullScreen:
    case ContextMenuItem::SavePage:
        return true;
    case ContextMenuItem::ViewSource:
        return m_view->d_ptr->adapter->canViewSource();
    case ContextMenuItem::SpellingSuggestions:
    case ContextMenuItem::Separator:
        return true;
    }
    Q_UNREACHABLE();
}

void QQuickWebEngineView::setTouchHandleDelegate(QQmlComponent *delegate)
{
    if (d_ptr->m_touchHandleDelegate != delegate) {
        d_ptr->m_touchHandleDelegate = delegate;
        d_ptr->webContentsAdapter()->resetTouchSelectionController();
        emit touchHandleDelegateChanged();
    }
}

QQmlComponent *QQuickWebEngineView::touchHandleDelegate() const
{
    return d_ptr->m_touchHandleDelegate;
}

void QQuickWebEngineView::save(const QString &filePath,
                               QWebEngineDownloadRequest::SavePageFormat format) const
{
    Q_D(const QQuickWebEngineView);
    d->adapter->save(filePath, format);
}

QT_END_NAMESPACE

#include "moc_qquickwebengineview_p.cpp"
#include "moc_qquickwebengineforeigntypes_p.cpp"
