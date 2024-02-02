// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginepage.h"
#include "qwebenginepage_p.h"

#include "qwebenginecertificateerror.h"
#include "qwebenginefilesystemaccessrequest.h"
#include "qwebenginefindtextresult.h"
#include "qwebenginefullscreenrequest.h"
#include "qwebenginehistory.h"
#include "qwebenginehistory_p.h"
#include "qwebenginehttprequest.h"
#include "qwebengineloadinginfo.h"
#include "qwebenginenavigationrequest.h"
#include "qwebenginenewwindowrequest.h"
#include "qwebenginenewwindowrequest_p.h"
#include "qwebengineprofile.h"
#include "qwebengineprofile_p.h"
#include "qwebengineregisterprotocolhandlerrequest.h"
#include "qwebenginescript.h"
#include "qwebenginescriptcollection_p.h"
#include "qwebenginesettings.h"

#include "authentication_dialog_controller.h"
#include "autofill_popup_controller.h"
#include "color_chooser_controller.h"
#include "find_text_helper.h"
#include "file_picker_controller.h"
#include "javascript_dialog_controller.h"
#include "profile_adapter.h"
#include "render_view_context_menu_qt.h"
#include "render_widget_host_view_qt_delegate.h"
#include "render_widget_host_view_qt_delegate_client.h"
#include "render_widget_host_view_qt_delegate_item.h"
#include "touch_selection_menu_controller.h"
#include "web_contents_adapter.h"

#include <QAction>
#include <QGuiApplication>
#include <QAuthenticator>
#include <QClipboard>
#include <QKeyEvent>
#include <QIcon>
#include <QLoggingCategory>
#include <QMimeData>
#include <QRect>
#include <QTimer>
#include <QUrl>
#include <QVariant>

QT_BEGIN_NAMESPACE

using namespace QtWebEngineCore;

static QWebEnginePage::WebWindowType toWindowType(WebContentsAdapterClient::WindowOpenDisposition disposition)
{
    switch (disposition) {
    case WebContentsAdapterClient::NewForegroundTabDisposition:
        return QWebEnginePage::WebBrowserTab;
    case WebContentsAdapterClient::NewBackgroundTabDisposition:
        return QWebEnginePage::WebBrowserBackgroundTab;
    case WebContentsAdapterClient::NewPopupDisposition:
        return QWebEnginePage::WebDialog;
    case WebContentsAdapterClient::NewWindowDisposition:
        return QWebEnginePage::WebBrowserWindow;
    default:
        Q_UNREACHABLE();
    }
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

QWebEnginePagePrivate::QWebEnginePagePrivate(QWebEngineProfile *_profile)
    : adapter(QSharedPointer<WebContentsAdapter>::create())
    , history(new QWebEngineHistory(new QWebEngineHistoryPrivate(this)))
    , profile(_profile ? _profile : QWebEngineProfile::defaultProfile())
    , settings(new QWebEngineSettings(profile->settings()))
    , view(nullptr)
    , isLoading(false)
    , scriptCollection(new QWebEngineScriptCollectionPrivate(profileAdapter()->userResourceController(), adapter))
    , m_isBeingAdopted(false)
    , m_backgroundColor(Qt::white)
    , fullscreenMode(false)
    , webChannel(nullptr)
    , webChannelWorldId(QWebEngineScript::MainWorld)
    , defaultAudioMuted(false)
    , defaultZoomFactor(1.0)
{
    memset(actions, 0, sizeof(actions));

#if QT_DEPRECATED_SINCE(6, 5)
    qRegisterMetaType<QWebEngineQuotaRequest>();
#endif
    qRegisterMetaType<QWebEngineRegisterProtocolHandlerRequest>();
    qRegisterMetaType<QWebEngineFileSystemAccessRequest>();
    qRegisterMetaType<QWebEngineFindTextResult>();

    // See setVisible().
    wasShownTimer.setSingleShot(true);
    QObject::connect(&wasShownTimer, &QTimer::timeout, [this](){
        ensureInitialized();
    });

    profile->d_ptr->addWebContentsAdapterClient(this);
}

QWebEnginePagePrivate::~QWebEnginePagePrivate()
{
    delete history;
    delete settings;
    profile->d_ptr->removeWebContentsAdapterClient(this);
}

RenderWidgetHostViewQtDelegate *QWebEnginePagePrivate::CreateRenderWidgetHostViewQtDelegate(RenderWidgetHostViewQtDelegateClient *client)
{
    if (view)
        return view->CreateRenderWidgetHostViewQtDelegate(client);
    delegateItem = new QtWebEngineCore::RenderWidgetHostViewQtDelegateItem(client, false);
    return delegateItem;
}

RenderWidgetHostViewQtDelegate *QWebEnginePagePrivate::CreateRenderWidgetHostViewQtDelegateForPopup(RenderWidgetHostViewQtDelegateClient *client)
{
    // Set the QWebEngineView as the parent for a popup delegate, so that the new popup window
    // responds properly to clicks in case the QWebEngineView is inside a modal QDialog. Setting the
    // parent essentially notifies the OS that the popup window is part of the modal session, and
    // should allow interaction.
    // The new delegate will not be deleted by the parent view though, because we unset the parent
    // when the parent is destroyed. The delegate will be destroyed by Chromium when the popup is
    // dismissed.
    return view
         ? view->CreateRenderWidgetHostViewQtDelegateForPopup(client)
         : new QtWebEngineCore::RenderWidgetHostViewQtDelegateItem(client, true);
}

void QWebEnginePagePrivate::initializationFinished()
{
    if (m_backgroundColor != Qt::white)
        adapter->setBackgroundColor(m_backgroundColor);
#if QT_CONFIG(webengine_webchannel)
    if (webChannel)
        adapter->setWebChannel(webChannel, webChannelWorldId);
#endif
    if (defaultAudioMuted != adapter->isAudioMuted())
        adapter->setAudioMuted(defaultAudioMuted);
    if (!qFuzzyCompare(adapter->currentZoomFactor(), defaultZoomFactor))
        adapter->setZoomFactor(defaultZoomFactor);
    if (view)
        adapter->setVisible(view->isVisible());

    scriptCollection.d->initializationFinished(adapter);

    m_isBeingAdopted = false;
}

void QWebEnginePagePrivate::titleChanged(const QString &title)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->titleChanged(title);
}

void QWebEnginePagePrivate::urlChanged()
{
    Q_Q(QWebEnginePage);
    QUrl qurl = adapter->activeUrl();
    if (url != qurl) {
        url = qurl;
        Q_EMIT q->urlChanged(qurl);
    }
}

void QWebEnginePagePrivate::iconChanged(const QUrl &url)
{
    Q_Q(QWebEnginePage);
    if (iconUrl == url)
        return;
    iconUrl = url;
    Q_EMIT q->iconUrlChanged(iconUrl);
    Q_EMIT q->iconChanged(iconUrl.isEmpty() ? QIcon() : adapter->icon());
}

void QWebEnginePagePrivate::loadProgressChanged(int progress)
{
    Q_Q(QWebEnginePage);
    QTimer::singleShot(0, q, [q, progress] () { Q_EMIT q->loadProgress(progress); });
}

void QWebEnginePagePrivate::didUpdateTargetURL(const QUrl &hoveredUrl)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->linkHovered(hoveredUrl.toString());
}

void QWebEnginePagePrivate::selectionChanged()
{
    Q_Q(QWebEnginePage);
    QTimer::singleShot(0, q, [this, q]() {
        updateEditActions();
        Q_EMIT q->selectionChanged();
    });
}

void QWebEnginePagePrivate::zoomUpdateIsNeeded()
{
    Q_Q(QWebEnginePage);
    q->setZoomFactor(defaultZoomFactor);
}

void QWebEnginePagePrivate::recentlyAudibleChanged(bool recentlyAudible)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->recentlyAudibleChanged(recentlyAudible);
}

void QWebEnginePagePrivate::renderProcessPidChanged(qint64 pid)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->renderProcessPidChanged(pid);
}

QRectF QWebEnginePagePrivate::viewportRect() const
{
    return view ? view->viewportRect() : QRectF();
}

QColor QWebEnginePagePrivate::backgroundColor() const
{
    return m_backgroundColor;
}

void QWebEnginePagePrivate::loadStarted(QWebEngineLoadingInfo info)
{
    Q_Q(QWebEnginePage);
    isLoading = true;
    QTimer::singleShot(0, q, [q, info = std::move(info)] () {
        Q_EMIT q->loadStarted();
        Q_EMIT q->loadingChanged(info);
    });
}

void QWebEnginePagePrivate::loadFinished(QWebEngineLoadingInfo info)
{
    Q_Q(QWebEnginePage);
    isLoading = false;
    QTimer::singleShot(0, q, [q, info = std::move(info)] () {
        Q_EMIT q->loadFinished(info.status() == QWebEngineLoadingInfo::LoadSucceededStatus);
        Q_EMIT q->loadingChanged(info);
    });
}

void QWebEnginePagePrivate::didPrintPageToPdf(const QString &filePath, bool success)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->pdfPrintingFinished(filePath, success);
    if (view)
        view->didPrintPageToPdf(filePath, success);
}

void QWebEnginePagePrivate::focusContainer()
{
    if (view) {
        view->focusContainer();
    }
}

void QWebEnginePagePrivate::unhandledKeyEvent(QKeyEvent *event)
{
    if (view) {
        view->unhandledKeyEvent(event);
    }
}

QSharedPointer<WebContentsAdapter>
QWebEnginePagePrivate::adoptNewWindow(QSharedPointer<WebContentsAdapter> newWebContents,
                                      WindowOpenDisposition disposition, bool userGesture,
                                      const QRect &initialGeometry, const QUrl &targetUrl)
{
    Q_Q(QWebEnginePage);
    Q_ASSERT(newWebContents);
    QWebEnginePage *newPage = q->createWindow(toWindowType(disposition));
    if (newPage) {
        if (!newWebContents->webContents())
            return newPage->d_func()->adapter; // Reuse existing adapter

        if (!newPage->d_func()->adoptWebContents(newWebContents.get()))
            return nullptr;

        if (!initialGeometry.isEmpty())
            emit newPage->geometryChangeRequested(initialGeometry);

        return newWebContents;
    }

    QWebEngineNewWindowRequest request(toDestinationType(disposition), initialGeometry,
                                       targetUrl, userGesture, newWebContents);

    Q_EMIT q->newWindowRequested(request);

    if (request.d_ptr->isRequestHandled)
        return newWebContents;
    return nullptr;
}

void QWebEnginePagePrivate::createNewWindow(WindowOpenDisposition disposition, bool userGesture, const QUrl &targetUrl)
{
    Q_Q(QWebEnginePage);
    QWebEnginePage *newPage = q->createWindow(toWindowType(disposition));
    if (newPage) {
        newPage->setUrl(targetUrl);
        return;
    }

    QWebEngineNewWindowRequest request(toDestinationType(disposition), QRect(),
                                       targetUrl, userGesture, nullptr);

    Q_EMIT q->newWindowRequested(request);
}

QString QWebEnginePagePrivate::actionText(int action)
{
    switch (action) {
    case QWebEnginePage::Back:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Back);
    case QWebEnginePage::Forward:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Forward);
    case QWebEnginePage::Stop:
        return QWebEnginePage::tr("Stop");
    case QWebEnginePage::Reload:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Reload);
    case QWebEnginePage::ReloadAndBypassCache:
        return QWebEnginePage::tr("Reload and Bypass Cache");
    case QWebEnginePage::Cut:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Cut);
    case QWebEnginePage::Copy:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Copy);
    case QWebEnginePage::Paste:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Paste);
    case QWebEnginePage::Undo:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Undo);
    case QWebEnginePage::Redo:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Redo);
    case QWebEnginePage::SelectAll:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::SelectAll);
    case QWebEnginePage::PasteAndMatchStyle:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::PasteAndMatchStyle);
    case QWebEnginePage::OpenLinkInThisWindow:
        return QWebEnginePage::tr("Open link in this window");
    case QWebEnginePage::OpenLinkInNewWindow:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::OpenLinkInNewWindow);
    case QWebEnginePage::OpenLinkInNewTab:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::OpenLinkInNewTab);
    case QWebEnginePage::OpenLinkInNewBackgroundTab:
        return QWebEnginePage::tr("Open link in new background tab");
    case QWebEnginePage::CopyLinkToClipboard:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::CopyLinkToClipboard);
    case QWebEnginePage::DownloadLinkToDisk:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::DownloadLinkToDisk);
    case QWebEnginePage::CopyImageToClipboard:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::CopyImageToClipboard);
    case QWebEnginePage::CopyImageUrlToClipboard:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::CopyImageUrlToClipboard);
    case QWebEnginePage::DownloadImageToDisk:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::DownloadImageToDisk);
    case QWebEnginePage::CopyMediaUrlToClipboard:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::CopyMediaUrlToClipboard);
    case QWebEnginePage::ToggleMediaControls:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::ToggleMediaControls);
    case QWebEnginePage::ToggleMediaLoop:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::ToggleMediaLoop);
    case QWebEnginePage::ToggleMediaPlayPause:
        return QWebEnginePage::tr("Toggle Play/Pause");
    case QWebEnginePage::ToggleMediaMute:
        return QWebEnginePage::tr("Toggle Mute");
    case QWebEnginePage::DownloadMediaToDisk:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::DownloadMediaToDisk);
    case QWebEnginePage::InspectElement:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::InspectElement);
    case QWebEnginePage::ExitFullScreen:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::ExitFullScreen);
    case QWebEnginePage::RequestClose:
        return QWebEnginePage::tr("Close Page");
    case QWebEnginePage::Unselect:
        return QWebEnginePage::tr("Unselect");
    case QWebEnginePage::SavePage:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::SavePage);
    case QWebEnginePage::ViewSource:
        return RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::ViewSource);
    case QWebEnginePage::ToggleBold:
        return QWebEnginePage::tr("&Bold");
    case QWebEnginePage::ToggleItalic:
        return QWebEnginePage::tr("&Italic");
    case QWebEnginePage::ToggleUnderline:
        return QWebEnginePage::tr("&Underline");
    case QWebEnginePage::ToggleStrikethrough:
        return QWebEnginePage::tr("&Strikethrough");
    case QWebEnginePage::AlignLeft:
        return QWebEnginePage::tr("Align &Left");
    case QWebEnginePage::AlignCenter:
        return QWebEnginePage::tr("Align &Center");
    case QWebEnginePage::AlignRight:
        return QWebEnginePage::tr("Align &Right");
    case QWebEnginePage::AlignJustified:
        return QWebEnginePage::tr("Align &Justified");
    case QWebEnginePage::Indent:
        return QWebEnginePage::tr("&Indent");
    case QWebEnginePage::Outdent:
        return QWebEnginePage::tr("&Outdent");
    case QWebEnginePage::InsertOrderedList:
        return QWebEnginePage::tr("Insert &Ordered List");
    case QWebEnginePage::InsertUnorderedList:
        return QWebEnginePage::tr("Insert &Unordered List");
    case QWebEnginePage::ChangeTextDirectionLTR:
        return QWebEnginePage::tr("Change text direction left to right");
    case QWebEnginePage::ChangeTextDirectionRTL:
        return QWebEnginePage::tr("Change text direction right to left");
    default:
        break;
    }
    return {};
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

bool QWebEnginePagePrivate::adoptWebContents(WebContentsAdapter *webContents)
{
    Q_ASSERT(webContents);
    if (webContents->profileAdapter() && profileAdapter() != webContents->profileAdapter()) {
        qWarning("Can not adopt content from a different WebEngineProfile.");
        return false;
    }

    m_isBeingAdopted = true;

    webContents->setRequestInterceptor(adapter->requestInterceptor());

    // This throws away the WebContentsAdapter that has been used until now.
    // All its states, particularly the loading URL, are replaced by the adopted WebContentsAdapter.
    WebContentsAdapterOwner *adapterOwner = new WebContentsAdapterOwner(adapter->sharedFromThis());
    adapterOwner->deleteLater();

    adapter = webContents->sharedFromThis();
    adapter->setClient(this);
    return true;
}

bool QWebEnginePagePrivate::isBeingAdopted()
{
    return m_isBeingAdopted;
}

void QWebEnginePagePrivate::close()
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->windowCloseRequested();
}

void QWebEnginePagePrivate::windowCloseRejected()
{
    // Do nothing for now.
}

void QWebEnginePagePrivate::didRunJavaScript(quint64 requestId, const QVariant& result)
{
    if (auto callback = m_variantCallbacks.take(requestId))
        callback(result);
}

void QWebEnginePagePrivate::didFetchDocumentMarkup(quint64 requestId, const QString& result)
{
    if (auto callback = m_stringCallbacks.take(requestId))
        callback(result);
}

void QWebEnginePagePrivate::didFetchDocumentInnerText(quint64 requestId, const QString& result)
{
    if (auto callback = m_stringCallbacks.take(requestId))
        callback(result);
}

void QWebEnginePagePrivate::didPrintPage(quint64 requestId, QSharedPointer<QByteArray> result)
{
#if QT_CONFIG(webengine_printing_and_pdf)
    // If no currentPrinter is set that means that were printing to PDF only.
    if (!currentPrinter) {
        if (!result.data())
            return;
        if (auto callback = m_pdfResultCallbacks.take(requestId))
            callback(*(result.data()));
        return;
    }

    if (view)
        view->didPrintPage(currentPrinter, result);
    else
        currentPrinter = nullptr;
#else
    // we should never enter this branch, but just for safe-keeping...
    Q_UNUSED(result);
    if (auto callback = m_pdfResultCallbacks.take(requestId))
        callback(QByteArray());
#endif
}

bool QWebEnginePagePrivate::passOnFocus(bool reverse)
{
    return view ? view->passOnFocus(reverse) : false;
}

void QWebEnginePagePrivate::authenticationRequired(QSharedPointer<AuthenticationDialogController> controller)
{
    Q_Q(QWebEnginePage);
    QAuthenticator networkAuth;
    networkAuth.setRealm(controller->realm());

    if (controller->isProxy())
        Q_EMIT q->proxyAuthenticationRequired(controller->url(), &networkAuth, controller->host());
    else
        Q_EMIT q->authenticationRequired(controller->url(), &networkAuth);

    // Authentication has been cancelled
    if (networkAuth.isNull()) {
        controller->reject();
        return;
    }

    controller->accept(networkAuth.user(), networkAuth.password());
}

void QWebEnginePagePrivate::releaseProfile()
{
    qWarning("Release of profile requested but WebEnginePage still not deleted. Expect troubles !");
    // this is not the way to go, but might avoid the crash if user code does not make any calls to page.
    q_ptr->d_ptr.reset();
}

void QWebEnginePagePrivate::showColorDialog(QSharedPointer<ColorChooserController> controller)
{
    if (view)
        view->showColorDialog(controller);
}

void QWebEnginePagePrivate::runMediaAccessPermissionRequest(const QUrl &securityOrigin, WebContentsAdapterClient::MediaRequestFlags requestFlags)
{
    Q_Q(QWebEnginePage);
    QWebEnginePage::Feature feature;
    if (requestFlags.testFlag(WebContentsAdapterClient::MediaAudioCapture) &&
        requestFlags.testFlag(WebContentsAdapterClient::MediaVideoCapture))
        feature = QWebEnginePage::MediaAudioVideoCapture;
    else if (requestFlags.testFlag(WebContentsAdapterClient::MediaAudioCapture))
        feature = QWebEnginePage::MediaAudioCapture;
    else if (requestFlags.testFlag(WebContentsAdapterClient::MediaVideoCapture))
        feature = QWebEnginePage::MediaVideoCapture;
    else if (requestFlags.testFlag(WebContentsAdapterClient::MediaDesktopAudioCapture) &&
             requestFlags.testFlag(WebContentsAdapterClient::MediaDesktopVideoCapture))
        feature = QWebEnginePage::DesktopAudioVideoCapture;
    else // if (requestFlags.testFlag(WebContentsAdapterClient::MediaDesktopVideoCapture))
        feature = QWebEnginePage::DesktopVideoCapture;
    Q_EMIT q->featurePermissionRequested(securityOrigin, feature);
}

static QWebEnginePage::Feature toFeature(QtWebEngineCore::ProfileAdapter::PermissionType type)
{
    switch (type) {
    case QtWebEngineCore::ProfileAdapter::NotificationPermission:
        return QWebEnginePage::Notifications;
    case QtWebEngineCore::ProfileAdapter::GeolocationPermission:
        return QWebEnginePage::Geolocation;
    default:
        break;
    }
    Q_UNREACHABLE();
    return QWebEnginePage::Feature(-1);
}

void QWebEnginePagePrivate::runFeaturePermissionRequest(QtWebEngineCore::ProfileAdapter::PermissionType permission, const QUrl &securityOrigin)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->featurePermissionRequested(securityOrigin, toFeature(permission));
}

void QWebEnginePagePrivate::runMouseLockPermissionRequest(const QUrl &securityOrigin)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->featurePermissionRequested(securityOrigin, QWebEnginePage::MouseLock);
}

void QWebEnginePagePrivate::runRegisterProtocolHandlerRequest(QWebEngineRegisterProtocolHandlerRequest request)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->registerProtocolHandlerRequested(request);
}

/*!
    \fn void QWebEnginePage::fileSystemAccessRequested(QWebEngineFileSystemAccessRequest request)
    \since 6.4

    This signal is emitted when the web page requests access to local files or directories.

    The request object \a request can be used to accept or reject the request.
*/

void QWebEnginePagePrivate::runFileSystemAccessRequest(QWebEngineFileSystemAccessRequest request)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->fileSystemAccessRequested(request);
}

QObject *QWebEnginePagePrivate::accessibilityParentObject()
{
    return view ? view->accessibilityParentObject() : nullptr;
}

void QWebEnginePagePrivate::updateAction(QWebEnginePage::WebAction action) const
{
#if !QT_CONFIG(action)
    Q_UNUSED(action);
#else
    QAction *a = actions[action];
    if (!a)
        return;

    bool enabled = true;

    switch (action) {
    case QWebEnginePage::Back:
        enabled = adapter->canGoToOffset(-1);
        break;
    case QWebEnginePage::Forward:
        enabled = adapter->canGoToOffset(1);
        break;
    case QWebEnginePage::Stop:
        enabled = isLoading;
        break;
    case QWebEnginePage::Reload:
    case QWebEnginePage::ReloadAndBypassCache:
        enabled = !isLoading;
        break;
    case QWebEnginePage::ViewSource:
        enabled = adapter->canViewSource();
        break;
    case QWebEnginePage::Cut:
    case QWebEnginePage::Copy:
    case QWebEnginePage::Unselect:
        enabled = adapter->hasFocusedFrame() && !adapter->selectedText().isEmpty();
        break;
    case QWebEnginePage::Paste:
    case QWebEnginePage::Undo:
    case QWebEnginePage::Redo:
    case QWebEnginePage::SelectAll:
    case QWebEnginePage::PasteAndMatchStyle:
        enabled = adapter->hasFocusedFrame();
        break;
    default:
        break;
    }

    a->setEnabled(enabled);
#endif // QT_CONFIG(action)
}

void QWebEnginePagePrivate::updateNavigationActions()
{
    updateAction(QWebEnginePage::Back);
    updateAction(QWebEnginePage::Forward);
    updateAction(QWebEnginePage::Stop);
    updateAction(QWebEnginePage::Reload);
    updateAction(QWebEnginePage::ReloadAndBypassCache);
    updateAction(QWebEnginePage::ViewSource);
}

void QWebEnginePagePrivate::updateEditActions()
{
    updateAction(QWebEnginePage::Cut);
    updateAction(QWebEnginePage::Copy);
    updateAction(QWebEnginePage::Paste);
    updateAction(QWebEnginePage::Undo);
    updateAction(QWebEnginePage::Redo);
    updateAction(QWebEnginePage::SelectAll);
    updateAction(QWebEnginePage::PasteAndMatchStyle);
    updateAction(QWebEnginePage::Unselect);
}

#if QT_CONFIG(action)
void QWebEnginePagePrivate::_q_webActionTriggered(bool checked)
{
    Q_Q(QWebEnginePage);
    QAction *a = qobject_cast<QAction *>(q->sender());
    if (!a)
        return;
    QWebEnginePage::WebAction action = static_cast<QWebEnginePage::WebAction>(a->data().toInt());
    q->triggerAction(action, checked);
}
#endif // QT_CONFIG(action)

void QWebEnginePagePrivate::recreateFromSerializedHistory(QDataStream &input)
{
    QSharedPointer<WebContentsAdapter> newWebContents = WebContentsAdapter::createFromSerializedNavigationHistory(input, this);
    if (newWebContents) {
        adapter = std::move(newWebContents);
        adapter->setClient(this);
        adapter->loadDefault();
    }
}

void QWebEnginePagePrivate::updateScrollPosition(const QPointF &position)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->scrollPositionChanged(position);
}

void QWebEnginePagePrivate::updateContentsSize(const QSizeF &size)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->contentsSizeChanged(size);
}

void QWebEnginePagePrivate::setFullScreenMode(bool fullscreen)
{
    if (fullscreenMode != fullscreen) {
        fullscreenMode = fullscreen;
        adapter->changedFullScreen();
    }
}

ProfileAdapter* QWebEnginePagePrivate::profileAdapter()
{
    return profile->d_ptr->profileAdapter();
}

WebContentsAdapter *QWebEnginePagePrivate::webContentsAdapter()
{
    ensureInitialized();
    return adapter.data();
}

const QObject *QWebEnginePagePrivate::holdingQObject() const
{
    Q_Q(const QWebEnginePage);
    return q;
}

void QWebEnginePagePrivate::findTextFinished(const QWebEngineFindTextResult &result)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->findTextFinished(result);
}

void QWebEnginePagePrivate::showAutofillPopup(QtWebEngineCore::AutofillPopupController *controller,
                                              const QRect &bounds, bool autoselectFirstSuggestion)
{
    if (view)
        view->showAutofillPopup(controller, bounds, autoselectFirstSuggestion);
}

void QWebEnginePagePrivate::hideAutofillPopup()
{
    if (view)
        view->hideAutofillPopup();
}

void QWebEnginePagePrivate::ensureInitialized() const
{
    if (!adapter->isInitialized())
        adapter->loadDefault();
}

QWebEnginePage::QWebEnginePage(QObject* parent)
    : QObject(parent)
    , d_ptr(new QWebEnginePagePrivate())
{
    Q_D(QWebEnginePage);
    d->q_ptr = this;
    d->adapter->setClient(d);
}

/*!
    \fn void QWebEnginePage::findTextFinished(const QWebEngineFindTextResult &result)
    \since 5.14

    This signal is emitted when a search string search on a page is completed. \a result is
    the result of the string search.

    \sa findText()
*/


/*!
    \enum QWebEnginePage::RenderProcessTerminationStatus
    \since 5.6

    This enum describes the status with which the render process terminated:

    \value  NormalTerminationStatus
            The render process terminated normally.
    \value  AbnormalTerminationStatus
            The render process terminated with with a non-zero exit status.
    \value  CrashedTerminationStatus
            The render process crashed, for example because of a segmentation fault.
    \value  KilledTerminationStatus
            The render process was killed, for example by \c SIGKILL or task manager kill.
*/

/*!
    \fn QWebEnginePage::renderProcessTerminated(RenderProcessTerminationStatus terminationStatus, int exitCode)
    \since 5.6

    This signal is emitted when the render process is terminated with a non-zero exit status.
    \a terminationStatus is the termination status of the process and \a exitCode is the status code
    with which the process terminated.
*/

/*!
    \fn QWebEnginePage::fullScreenRequested(QWebEngineFullScreenRequest fullScreenRequest)

    This signal is emitted when the web page issues the request to enter fullscreen mode for
    a web-element, usually a video element.

    The request object \a fullScreenRequest can be used to accept or reject the request.

    If the request is accepted the element requesting fullscreen will fill the viewport,
    but it is up to the application to make the view fullscreen or move the page to a view
    that is fullscreen.

    \sa QWebEngineSettings::FullScreenSupportEnabled
*/

/*!
    \fn QWebEnginePage::quotaRequested(QWebEngineQuotaRequest quotaRequest)
    \since 5.11
    \deprecated [6.5] This signal is no longer emitted.

    Requesting host quota is no longer supported by Chromium.
    The behavior of navigator.webkitPersistentStorage
    is identical to navigator.webkitTemporaryStorage.

    For further details, see https://crbug.com/1233525
*/

/*!
    \fn QWebEnginePage::registerProtocolHandlerRequested(QWebEngineRegisterProtocolHandlerRequest request)
    \since 5.11

    This signal is emitted when the web page tries to register a custom protocol
    using the \l registerProtocolHandler API.

    The request object \a request can be used to accept or reject the request:

    \snippet webenginewidgets/simplebrowser/webview.cpp registerProtocolHandlerRequested
*/

/*!
    \property QWebEnginePage::scrollPosition
    \since 5.7

    \brief The scroll position of the page contents.
*/

/*!
    \property QWebEnginePage::contentsSize
    \since 5.7

    \brief The size of the page contents.
*/

/*!
    \fn void QWebEnginePage::audioMutedChanged(bool muted)
    \since 5.7

    This signal is emitted when the page's \a muted state changes.
    \note Not to be confused with a specific HTML5 audio or video element being muted.
*/

/*!
    \fn void QWebEnginePage::recentlyAudibleChanged(bool recentlyAudible);
    \since 5.7

    This signal is emitted when the page's audible state, \a recentlyAudible, changes, because
    the audio is played or stopped.

    \note The signal is also emitted when calling the setAudioMuted() method.
*/

/*!
  \fn void QWebEnginePage::renderProcessPidChanged(qint64 pid);
  \since 5.15

  This signal is emitted when the underlying render process PID, \a pid, changes.
*/

/*!
    \fn void QWebEnginePage::iconUrlChanged(const QUrl &url)

    This signal is emitted when the URL of the icon ("favicon") associated with the
    page is changed. The new URL is specified by \a url.

    \sa iconUrl(), icon(), iconChanged()
*/

/*!
    \fn void QWebEnginePage::iconChanged(const QIcon &icon)
    \since 5.7

    This signal is emitted when the icon ("favicon") associated with the
    page is changed. The new icon is specified by \a icon.

    \sa icon(), iconUrl(), iconUrlChanged()
*/

/*!
    Constructs an empty web engine page in the web engine profile \a profile with the parent
    \a parent.

    If the profile is not the default profile, the caller must ensure that the profile stays alive
    for as long as the page does.

    \since 5.5
*/
QWebEnginePage::QWebEnginePage(QWebEngineProfile *profile, QObject* parent)
    : QObject(parent)
    , d_ptr(new QWebEnginePagePrivate(profile))
{
    Q_D(QWebEnginePage);
    d->q_ptr = this;
    d->adapter->setClient(d);
}

QWebEnginePage::~QWebEnginePage()
{
    if (d_ptr) {
        // d_ptr might be exceptionally null if profile adapter got deleted first
        setDevToolsPage(nullptr);
        emit _q_aboutToDelete();

        for (auto varFun : std::as_const(d_ptr->m_variantCallbacks))
            varFun(QVariant());
        for (auto strFun : std::as_const(d_ptr->m_stringCallbacks))
            strFun(QString());
        d_ptr->m_variantCallbacks.clear();
        d_ptr->m_stringCallbacks.clear();
    }
}

QWebEngineHistory *QWebEnginePage::history() const
{
    Q_D(const QWebEnginePage);
    return d->history;
}

QWebEngineSettings *QWebEnginePage::settings() const
{
    Q_D(const QWebEnginePage);
    return d->settings;
}

/*!
 * Returns a pointer to the web channel instance used by this page or a null pointer if none was set.
 * This channel automatically uses the internal web engine transport mechanism over Chromium IPC
 * that is exposed in the JavaScript context of this page as \c qt.webChannelTransport.
 *
 * \since 5.5
 * \sa setWebChannel()
 */
QWebChannel *QWebEnginePage::webChannel() const
{
#if QT_CONFIG(webengine_webchannel)
    Q_D(const QWebEnginePage);
    return d->webChannel;
#endif
    qWarning("WebEngine compiled without webchannel support");
    return nullptr;
}

/*!
 * Sets the web channel instance to be used by this page to \a channel and connects it to
 * web engine's transport using Chromium IPC messages. The transport is exposed in the JavaScript
 * world \a worldId as
 * \c qt.webChannelTransport, which should be used when using the \l{Qt WebChannel JavaScript API}.
 *
 * \note The page does not take ownership of the channel object.
 * \note Only one web channel can be installed per page, setting one even in another JavaScript
 *       world uninstalls any already installed web channel.
 *
 * \since 5.7
 * \sa QWebEngineScript::ScriptWorldId
 */
void QWebEnginePage::setWebChannel(QWebChannel *channel, quint32 worldId)
{
#if QT_CONFIG(webengine_webchannel)
    Q_D(QWebEnginePage);
    if (d->webChannel != channel || d->webChannelWorldId != worldId) {
        d->webChannel = channel;
        d->webChannelWorldId = worldId;
        d->adapter->setWebChannel(channel, worldId);
    }
#else
    Q_UNUSED(channel);
    Q_UNUSED(worldId);
    qWarning("WebEngine compiled without webchannel support");
#endif
}

/*!
    \property QWebEnginePage::backgroundColor
    \brief The page's background color behind the document's body.
    \since 5.6

    You can set the background color to Qt::transparent or to a translucent
    color to see through the document, or you can set it to match your
    web content in a hybrid application to prevent the white flashes that may appear
    during loading.

    The default value is white.
*/
QColor QWebEnginePage::backgroundColor() const
{
    Q_D(const QWebEnginePage);
    return d->m_backgroundColor;
}

void QWebEnginePage::setBackgroundColor(const QColor &color)
{
    Q_D(QWebEnginePage);
    if (d->m_backgroundColor == color)
        return;
    d->m_backgroundColor = color;
    d->adapter->setBackgroundColor(color);
}

/*!
 * Save the currently loaded web page to disk.
 *
 * The web page is saved to \a filePath in the specified \a{format}.
 *
 * This is a short cut for the following actions:
 * \list
 *   \li Trigger the Save web action.
 *   \li Accept the next download item and set the specified file path and save format.
 * \endlist
 *
 * This function issues an asynchronous download request for the web page and returns immediately.
 *
 * \sa QWebEngineDownloadRequest::SavePageFormat
 * \since 5.8
 */
void QWebEnginePage::save(const QString &filePath,
                          QWebEngineDownloadRequest::SavePageFormat format) const
{
    Q_D(const QWebEnginePage);
    d->ensureInitialized();
    d->adapter->save(filePath, format);
}

/*!
    \property QWebEnginePage::audioMuted
    \brief Whether the current page audio is muted.
    \since 5.7

    The default value is \c false.
    \sa recentlyAudible
*/
bool QWebEnginePage::isAudioMuted() const {
    Q_D(const QWebEnginePage);
    if (d->adapter->isInitialized())
        return d->adapter->isAudioMuted();
    return d->defaultAudioMuted;
}

void QWebEnginePage::setAudioMuted(bool muted) {
    Q_D(QWebEnginePage);
    bool wasAudioMuted = isAudioMuted();
    d->defaultAudioMuted = muted;
    d->adapter->setAudioMuted(muted);
    if (wasAudioMuted != isAudioMuted())
        Q_EMIT audioMutedChanged(muted);
}

/*!
    \property QWebEnginePage::recentlyAudible
    \brief The current page's \e {audible state}, that is, whether audio was recently played
    or not.
    \since 5.7

    The default value is \c false.
    \sa audioMuted
*/
bool QWebEnginePage::recentlyAudible() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->isInitialized() && d->adapter->recentlyAudible();
}

/*!
  \property QWebEnginePage::renderProcessPid
  \brief The process ID (PID) of the render process assigned to the current
  page's main frame.
  \since 5.15

  If no render process is available yet, \c 0 is returned.
*/
qint64 QWebEnginePage::renderProcessPid() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->renderProcessPid();
}

/*!
    Returns the web engine profile the page belongs to.
    \since 5.5
*/
QWebEngineProfile *QWebEnginePage::profile() const
{
    Q_D(const QWebEnginePage);
    return d->profile;
}

bool QWebEnginePage::hasSelection() const
{
    return !selectedText().isEmpty();
}

QString QWebEnginePage::selectedText() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->selectedText();
}

#if QT_CONFIG(action)
QAction *QWebEnginePage::action(WebAction action) const
{
    Q_D(const QWebEnginePage);
    if (action == QWebEnginePage::NoWebAction)
        return nullptr;
    if (d->actions[action])
        return d->actions[action];

    const QString text = QWebEnginePagePrivate::actionText(action);

    QAction *a = new QAction(const_cast<QWebEnginePage*>(this));
    a->setText(text);
    a->setData(action);

    connect(a, SIGNAL(triggered(bool)), this, SLOT(_q_webActionTriggered(bool)));

    d->actions[action] = a;
    d->updateAction(action);
    return a;
}
#endif // QT_CONFIG(action)

void QWebEnginePage::triggerAction(WebAction action, bool)
{
    Q_D(QWebEnginePage);
    d->ensureInitialized();
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
        if (d->view && d->view->lastContextMenuRequest() && d->view->lastContextMenuRequest()->filteredLinkUrl().isValid())
            setUrl(d->view->lastContextMenuRequest()->filteredLinkUrl());
        break;
    case OpenLinkInNewWindow:
        if (d->view && d->view->lastContextMenuRequest() && d->view->lastContextMenuRequest()->filteredLinkUrl().isValid())
            d->createNewWindow(WebContentsAdapterClient::NewWindowDisposition, true,
                               d->view->lastContextMenuRequest()->filteredLinkUrl());
        break;
    case OpenLinkInNewTab:
        if (d->view && d->view->lastContextMenuRequest() && d->view->lastContextMenuRequest()->filteredLinkUrl().isValid())
            d->createNewWindow(WebContentsAdapterClient::NewForegroundTabDisposition, true,
                               d->view->lastContextMenuRequest()->filteredLinkUrl());
        break;
    case OpenLinkInNewBackgroundTab:
        if (d->view && d->view->lastContextMenuRequest() && d->view->lastContextMenuRequest()->filteredLinkUrl().isValid())
            d->createNewWindow(WebContentsAdapterClient::NewBackgroundTabDisposition, true,
                               d->view->lastContextMenuRequest()->filteredLinkUrl());
        break;
    case CopyLinkToClipboard:
        if (d->view && d->view->lastContextMenuRequest() && !d->view->lastContextMenuRequest()->linkUrl().isEmpty()) {
            QString urlString = d->view->lastContextMenuRequest()->linkUrl().toString(
                    QUrl::FullyEncoded);
            QString linkText = d->view->lastContextMenuRequest()->linkText().toHtmlEscaped();
            QString title = d->view->lastContextMenuRequest()->titleText();
            if (!title.isEmpty())
                title = QStringLiteral(" title=\"%1\"").arg(title.toHtmlEscaped());
            QMimeData *data = new QMimeData();
            data->setText(urlString);
            QString html = QStringLiteral("<a href=\"") + urlString + QStringLiteral("\"") + title + QStringLiteral(">")
                         + linkText + QStringLiteral("</a>");
            data->setHtml(html);
            data->setUrls(QList<QUrl>() << d->view->lastContextMenuRequest()->linkUrl());
            QGuiApplication::clipboard()->setMimeData(data);
        }
        break;
    case DownloadLinkToDisk:
        if (d->view && d->view->lastContextMenuRequest() && d->view->lastContextMenuRequest()->filteredLinkUrl().isValid())
            d->adapter->download(d->view->lastContextMenuRequest()->filteredLinkUrl(),
                                 d->view->lastContextMenuRequest()->suggestedFileName(),
                                 d->view->lastContextMenuRequest()->referrerUrl(),
                                 d->view->lastContextMenuRequest()->referrerPolicy());

        break;
    case CopyImageToClipboard:
        if (d->view && d->view->lastContextMenuRequest() && d->view->lastContextMenuRequest()->hasImageContent()
            && (d->view->lastContextMenuRequest()->mediaType()
                        == QWebEngineContextMenuRequest::MediaTypeImage
                || d->view->lastContextMenuRequest()->mediaType()
                        == QWebEngineContextMenuRequest::MediaTypeCanvas)) {
            d->adapter->copyImageAt(d->view->lastContextMenuRequest()->position());
        }
        break;
    case CopyImageUrlToClipboard:
        if (d->view && d->view->lastContextMenuRequest() && d->view->lastContextMenuRequest()->mediaUrl().isValid()
            && d->view->lastContextMenuRequest()->mediaType()
                    == QWebEngineContextMenuRequest::MediaTypeImage) {
            QString urlString =
                    d->view->lastContextMenuRequest()->mediaUrl().toString(QUrl::FullyEncoded);
            QString alt = d->view->lastContextMenuRequest()->altText();
            if (!alt.isEmpty())
                alt = QStringLiteral(" alt=\"%1\"").arg(alt.toHtmlEscaped());
            QString title = d->view->lastContextMenuRequest()->titleText();
            if (!title.isEmpty())
                title = QStringLiteral(" title=\"%1\"").arg(title.toHtmlEscaped());
            QMimeData *data = new QMimeData();
            data->setText(urlString);
            QString html = QStringLiteral("<img src=\"") + urlString + QStringLiteral("\"") + title + alt + QStringLiteral("></img>");
            data->setHtml(html);
            data->setUrls(QList<QUrl>() << d->view->lastContextMenuRequest()->mediaUrl());
            QGuiApplication::clipboard()->setMimeData(data);
        }
        break;
    case DownloadImageToDisk:
    case DownloadMediaToDisk:
        if (d->view && d->view->lastContextMenuRequest() && d->view->lastContextMenuRequest()->mediaUrl().isValid())
            d->adapter->download(d->view->lastContextMenuRequest()->mediaUrl(),
                                 d->view->lastContextMenuRequest()->suggestedFileName(),
                                 d->view->lastContextMenuRequest()->referrerUrl(),
                                 d->view->lastContextMenuRequest()->referrerPolicy());
        break;
    case CopyMediaUrlToClipboard:
        if (d->view && d->view->lastContextMenuRequest() && d->view->lastContextMenuRequest()->mediaUrl().isValid()
            && (d->view->lastContextMenuRequest()->mediaType()
                        == QWebEngineContextMenuRequest::MediaTypeAudio
                || d->view->lastContextMenuRequest()->mediaType()
                        == QWebEngineContextMenuRequest::MediaTypeVideo)) {
            QString urlString =
                    d->view->lastContextMenuRequest()->mediaUrl().toString(QUrl::FullyEncoded);
            QString title = d->view->lastContextMenuRequest()->titleText();
            if (!title.isEmpty())
                title = QStringLiteral(" title=\"%1\"").arg(title.toHtmlEscaped());
            QMimeData *data = new QMimeData();
            data->setText(urlString);
            if (d->view->lastContextMenuRequest()->mediaType()
                == QWebEngineContextMenuRequest::MediaTypeAudio)
                data->setHtml(QStringLiteral("<audio src=\"") + urlString + QStringLiteral("\"") + title +
                              QStringLiteral("></audio>"));
            else
                data->setHtml(QStringLiteral("<video src=\"") + urlString + QStringLiteral("\"") + title +
                              QStringLiteral("></video>"));
            data->setUrls(QList<QUrl>() << d->view->lastContextMenuRequest()->mediaUrl());
            QGuiApplication::clipboard()->setMimeData(data);
        }
        break;
    case ToggleMediaControls:
        if (d->view && d->view->lastContextMenuRequest() && d->view->lastContextMenuRequest()->mediaUrl().isValid()
            && d->view->lastContextMenuRequest()->mediaFlags()
                    & QWebEngineContextMenuRequest::MediaCanToggleControls) {
            bool enable = !(d->view->lastContextMenuRequest()->mediaFlags()
                            & QWebEngineContextMenuRequest::MediaControls);
            d->adapter->executeMediaPlayerActionAt(d->view->lastContextMenuRequest()->position(),
                                                   WebContentsAdapter::MediaPlayerControls, enable);
        }
        break;
    case ToggleMediaLoop:
        if (d->view && d->view->lastContextMenuRequest() && d->view->lastContextMenuRequest()->mediaUrl().isValid()
            && (d->view->lastContextMenuRequest()->mediaType()
                        == QWebEngineContextMenuRequest::MediaTypeAudio
                || d->view->lastContextMenuRequest()->mediaType()
                        == QWebEngineContextMenuRequest::MediaTypeVideo)) {
            bool enable = !(d->view->lastContextMenuRequest()->mediaFlags()
                            & QWebEngineContextMenuRequest::MediaLoop);
            d->adapter->executeMediaPlayerActionAt(d->view->lastContextMenuRequest()->position(),
                                                   WebContentsAdapter::MediaPlayerLoop, enable);
        }
        break;
    case ToggleMediaPlayPause:
        if (d->view && d->view->lastContextMenuRequest() && d->view->lastContextMenuRequest()->mediaUrl().isValid()
            && (d->view->lastContextMenuRequest()->mediaType()
                        == QWebEngineContextMenuRequest::MediaTypeAudio
                || d->view->lastContextMenuRequest()->mediaType()
                        == QWebEngineContextMenuRequest::MediaTypeVideo)) {
            bool enable = (d->view->lastContextMenuRequest()->mediaFlags()
                           & QWebEngineContextMenuRequest::MediaPaused);
            d->adapter->executeMediaPlayerActionAt(d->view->lastContextMenuRequest()->position(),
                                                   WebContentsAdapter::MediaPlayerPlay, enable);
        }
        break;
    case ToggleMediaMute:
        if (d->view && d->view->lastContextMenuRequest() && d->view->lastContextMenuRequest()->mediaUrl().isValid()
            && d->view->lastContextMenuRequest()->mediaFlags()
                    & QWebEngineContextMenuRequest::MediaHasAudio) {
            // Make sure to negate the value, so that toggling actually works.
            bool enable = !(d->view->lastContextMenuRequest()->mediaFlags()
                            & QWebEngineContextMenuRequest::MediaMuted);
            d->adapter->executeMediaPlayerActionAt(d->view->lastContextMenuRequest()->position(),
                                                   WebContentsAdapter::MediaPlayerMute, enable);
        }
        break;
    case InspectElement:
        if (d->view && d->view->lastContextMenuRequest())
            d->adapter->inspectElementAt(d->view->lastContextMenuRequest()->position());
        break;
    case ExitFullScreen:
        // See under ViewSource, anything that can trigger a delete of the current view is dangerous to call directly here.
        QTimer::singleShot(0, this, [d](){ d->adapter->exitFullScreen(); });
        break;
    case RequestClose:
        d->adapter->requestClose();
        break;
    case SavePage:
        d->adapter->save();
        break;
    case ViewSource:
        // This is a workaround to make the ViewSource action working in a context menu.
        // The WebContentsAdapter::viewSource() method deletes a
        // RenderWidgetHostViewQtDelegateWidget instance which passes the control to the event
        // loop. If the QMenu::aboutToHide() signal is connected to the QObject::deleteLater()
        // slot the QMenu is deleted by the event handler while the ViewSource action is still not
        // completed. This may lead to a crash. To avoid this the WebContentsAdapter::viewSource()
        // method is called indirectly via the QTimer::singleShot() function which schedules the
        // the viewSource() call after the QMenu's destruction.
        QTimer::singleShot(0, this, [d](){ d->adapter->viewSource(); });
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
    case NoWebAction:
        break;
    case WebActionCount:
        Q_UNREACHABLE();
        break;
    }
}

/*!
 * \since 5.8
 * Replace the current misspelled word with \a replacement.
 *
 * The current misspelled word can be found in QWebEngineContextMenuRequest::misspelledWord(),
 * and suggested replacements in QWebEngineContextMenuRequest::spellCheckerSuggestions().
 *
 */

void QWebEnginePage::replaceMisspelledWord(const QString &replacement)
{
    Q_D(QWebEnginePage);
    d->adapter->replaceMisspelling(replacement);
}

void QWebEnginePage::findText(const QString &subString, FindFlags options, const std::function<void(const QWebEngineFindTextResult &)> &resultCallback)
{
    Q_D(QWebEnginePage);
    if (!d->adapter->isInitialized()) {
        if (resultCallback)
            resultCallback(QWebEngineFindTextResult());
        return;
    }

    d->adapter->findTextHelper()->startFinding(subString, options & FindCaseSensitively, options & FindBackward, resultCallback);
}

/*!
 * \reimp
 */
bool QWebEnginePage::event(QEvent *e)
{
    return QObject::event(e);
}

void QWebEnginePagePrivate::contextMenuRequested(QWebEngineContextMenuRequest *data)
{
    if (view)
        view->contextMenuRequested(data);
}

/*!
    \fn void QWebEnginePage::navigationRequested(QWebEngineNavigationRequest &request)
    \since 6.2

    This signal is emitted on navigation together with the call the acceptNavigationRequest().
    It can be used to accept or ignore the \a request. The default is to accept.

    \sa acceptNavigationRequest()
*/

void QWebEnginePagePrivate::navigationRequested(int navigationType, const QUrl &url, bool &accepted, bool isMainFrame)
{
    Q_Q(QWebEnginePage);

    accepted = q->acceptNavigationRequest(url, static_cast<QWebEnginePage::NavigationType>(navigationType), isMainFrame);
    if (accepted) {
        QWebEngineNavigationRequest navigationRequest(url, static_cast<QWebEngineNavigationRequest::NavigationType>(navigationType), isMainFrame);
        Q_EMIT q->navigationRequested(navigationRequest);
        accepted = navigationRequest.isAccepted();
    }

    if (accepted && adapter->findTextHelper()->isFindTextInProgress())
        adapter->findTextHelper()->stopFinding();
}

void QWebEnginePagePrivate::requestFullScreenMode(const QUrl &origin, bool fullscreen)
{
    Q_Q(QWebEnginePage);
    QWebEngineFullScreenRequest request(origin, fullscreen, [q = QPointer(q)] (bool toggleOn) { if (q) q->d_ptr->setFullScreenMode(toggleOn); });
    Q_EMIT q->fullScreenRequested(std::move(request));
}

bool QWebEnginePagePrivate::isFullScreenMode() const
{
    return fullscreenMode;
}

void QWebEnginePagePrivate::javascriptDialog(QSharedPointer<JavaScriptDialogController> controller)
{
    Q_Q(QWebEnginePage);
    bool accepted = false;
    QString promptResult;
    switch (controller->type()) {
    case AlertDialog:
        q->javaScriptAlert(controller->securityOrigin(), controller->message());
        accepted = true;
        break;
    case ConfirmDialog:
        accepted = q->javaScriptConfirm(controller->securityOrigin(), controller->message());
        break;
    case PromptDialog:
        accepted = q->javaScriptPrompt(controller->securityOrigin(), controller->message(), controller->defaultPrompt(), &promptResult);
        if (accepted)
            controller->textProvided(promptResult);
        break;
    case UnloadDialog:
        accepted = q->javaScriptConfirm(controller->securityOrigin(), QCoreApplication::translate("QWebEnginePage", "Are you sure you want to leave this page? Changes that you made may not be saved."));
        break;
    case InternalAuthorizationDialog:
        accepted = view ? view->showAuthorizationDialog(controller->title(), controller->message())
                        : false;
        break;
    }
    if (accepted)
        controller->accept();
    else
        controller->reject();
}

void QWebEnginePagePrivate::allowCertificateError(const QWebEngineCertificateError &error)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->certificateError(error);
}

void QWebEnginePagePrivate::selectClientCert(const QSharedPointer<ClientCertSelectController> &controller)
{
    Q_Q(QWebEnginePage);
    QWebEngineClientCertificateSelection certSelection(controller);

    Q_EMIT q->selectClientCertificate(certSelection);
}

/*!
    \fn void QWebEnginePage::selectClientCertificate(QWebEngineClientCertificateSelection clientCertificateSelection)
    \since 5.12

    This signal is emitted when a web site requests an SSL client certificate, and one or more were
    found in system's client certificate store.

    Handling the signal is asynchronous, and loading will be waiting until a certificate is selected,
    or the last copy of \a clientCertificateSelection is destroyed.

    If the signal is not handled, \a clientCertificateSelection is automatically destroyed, and loading
    will continue without a client certificate.

    \sa QWebEngineClientCertificateSelection
*/

void QWebEnginePagePrivate::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
{
    Q_Q(QWebEnginePage);
    q->javaScriptConsoleMessage(static_cast<QWebEnginePage::JavaScriptConsoleMessageLevel>(level), message, lineNumber, sourceID);
}

void QWebEnginePagePrivate::renderProcessTerminated(RenderProcessTerminationStatus terminationStatus,
                                                int exitCode)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->renderProcessTerminated(static_cast<QWebEnginePage::RenderProcessTerminationStatus>(
                                      terminationStatus), exitCode);
}

void QWebEnginePagePrivate::requestGeometryChange(const QRect &geometry, const QRect &frameGeometry)
{
    Q_UNUSED(geometry);
    Q_Q(QWebEnginePage);
    Q_EMIT q->geometryChangeRequested(frameGeometry);
}

QObject *QWebEnginePagePrivate::dragSource() const
{
#if QT_CONFIG(draganddrop)
    return view->accessibilityParentObject();
#else
    return nullptr;
#endif // QT_CONFIG(draganddrop)
}

bool QWebEnginePagePrivate::isEnabled() const
{
    if (view)
        return view->isEnabled();
    return true;
}

void QWebEnginePagePrivate::setToolTip(const QString &toolTipText)
{
    if (view)
        view->setToolTip(toolTipText);
}

/*!
    \fn void QWebEnginePage::printRequested()
    \since 5.12

    This signal is emitted when the JavaScript \c{window.print()} method is called or the user pressed the print
    button of PDF viewer plugin.
    Typically, the signal handler can simply call printToPdf().

    \sa printToPdf()
*/

void QWebEnginePagePrivate::printRequested()
{
    Q_Q(QWebEnginePage);
    QTimer::singleShot(0, q, [q]() {
        Q_EMIT q->printRequested();
    });
    if (view)
        view->printRequested();
}

QtWebEngineCore::TouchHandleDrawableDelegate *
QWebEnginePagePrivate::createTouchHandleDelegate(const QMap<int, QImage> &images)
{
    return view->createTouchHandleDelegate(images);
}

void QWebEnginePagePrivate::showTouchSelectionMenu(
        QtWebEngineCore::TouchSelectionMenuController *controller, const QRect &selectionBounds,
        const QSize &handleSize)
{
    Q_UNUSED(handleSize);

    if (controller->buttonCount() == 1) {
        controller->runContextMenu();
        return;
    }

    view->showTouchSelectionMenu(controller, selectionBounds);
}

void QWebEnginePagePrivate::hideTouchSelectionMenu()
{
    view->hideTouchSelectionMenu();
}

void QWebEnginePagePrivate::lifecycleStateChanged(LifecycleState state)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->lifecycleStateChanged(static_cast<QWebEnginePage::LifecycleState>(state));
}

void QWebEnginePagePrivate::recommendedStateChanged(LifecycleState state)
{
    Q_Q(QWebEnginePage);
    QTimer::singleShot(0, q, [q, state]() {
        Q_EMIT q->recommendedStateChanged(static_cast<QWebEnginePage::LifecycleState>(state));
    });
}

void QWebEnginePagePrivate::visibleChanged(bool visible)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->visibleChanged(visible);
}

/*!
    \since 5.13

    Registers the request interceptor \a interceptor to intercept URL requests.

    The page does not take ownership of the pointer. This interceptor is called
    after any interceptors on the profile, and unlike profile interceptors, only
    URL requests from this page are intercepted. If the original request was
    already blocked or redirected by the profile interceptor, it will not be
    intercepted by this.

    To unset the request interceptor, set a \c nullptr.

    \sa QWebEngineUrlRequestInfo, QWebEngineProfile::setUrlRequestInterceptor()
*/

void QWebEnginePage::setUrlRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor)
{
    Q_D(QWebEnginePage);
    d->adapter->setRequestInterceptor(interceptor);
}

void QWebEnginePage::setFeaturePermission(const QUrl &securityOrigin, QWebEnginePage::Feature feature, QWebEnginePage::PermissionPolicy policy)
{
    Q_D(QWebEnginePage);
    if (policy == PermissionUnknown) {
        switch (feature) {
        case MediaAudioVideoCapture:
        case MediaAudioCapture:
        case MediaVideoCapture:
        case DesktopAudioVideoCapture:
        case DesktopVideoCapture:
        case MouseLock:
            break;
        case Geolocation:
            d->adapter->grantFeaturePermission(securityOrigin, ProfileAdapter::GeolocationPermission, ProfileAdapter::AskPermission);
            break;
        case Notifications:
            d->adapter->grantFeaturePermission(securityOrigin, ProfileAdapter::NotificationPermission, ProfileAdapter::AskPermission);
            break;
        }
        return;
    }

    const WebContentsAdapterClient::MediaRequestFlags audioVideoCaptureFlags(
        WebContentsAdapterClient::MediaVideoCapture |
        WebContentsAdapterClient::MediaAudioCapture);
    const WebContentsAdapterClient::MediaRequestFlags desktopAudioVideoCaptureFlags(
        WebContentsAdapterClient::MediaDesktopVideoCapture |
        WebContentsAdapterClient::MediaDesktopAudioCapture);

    if (policy == PermissionGrantedByUser) {
        switch (feature) {
        case MediaAudioVideoCapture:
            d->adapter->grantMediaAccessPermission(securityOrigin, audioVideoCaptureFlags);
            break;
        case MediaAudioCapture:
            d->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaAudioCapture);
            break;
        case MediaVideoCapture:
            d->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaVideoCapture);
            break;
        case DesktopAudioVideoCapture:
            d->adapter->grantMediaAccessPermission(securityOrigin, desktopAudioVideoCaptureFlags);
            break;
        case DesktopVideoCapture:
            d->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaDesktopVideoCapture);
            break;
        case MouseLock:
            d->adapter->grantMouseLockPermission(securityOrigin, true);
            break;
        case Geolocation:
            d->adapter->grantFeaturePermission(securityOrigin, ProfileAdapter::GeolocationPermission, ProfileAdapter::AllowedPermission);
            break;
        case Notifications:
            d->adapter->grantFeaturePermission(securityOrigin, ProfileAdapter::NotificationPermission, ProfileAdapter::AllowedPermission);
            break;
        }
    } else { // if (policy == PermissionDeniedByUser)
        switch (feature) {
        case MediaAudioVideoCapture:
        case MediaAudioCapture:
        case MediaVideoCapture:
        case DesktopAudioVideoCapture:
        case DesktopVideoCapture:
            d->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaNone);
            break;
        case Geolocation:
            d->adapter->grantFeaturePermission(securityOrigin, ProfileAdapter::GeolocationPermission, ProfileAdapter::DeniedPermission);
            break;
        case MouseLock:
            d->adapter->grantMouseLockPermission(securityOrigin, false);
            break;
        case Notifications:
            d->adapter->grantFeaturePermission(securityOrigin, ProfileAdapter::NotificationPermission, ProfileAdapter::DeniedPermission);
            break;
        }
    }
}

static inline QWebEnginePage::FileSelectionMode toPublic(FilePickerController::FileChooserMode mode)
{
    // Should the underlying values change, we'll need a switch here.
    return static_cast<QWebEnginePage::FileSelectionMode>(mode);
}

void QWebEnginePagePrivate::runFileChooser(QSharedPointer<FilePickerController> controller)
{
    Q_Q(QWebEnginePage);

    QStringList selectedFileNames = q->chooseFiles(toPublic(controller->mode()), (QStringList() << controller->defaultFileName()), controller->acceptedMimeTypes());

    if (!selectedFileNames.empty())
        controller->accepted(selectedFileNames);
    else
        controller->rejected();
}

QWebEngineSettings *QWebEnginePagePrivate::webEngineSettings() const
{
    return settings;
}

/*!
    \since 5.10
    Downloads the resource from the location given by \a url to a local file.

    If \a filename is given, it is used as the suggested file name.
    If it is relative, the file is saved in the standard download location with
    the given name.
    If it is a null or empty QString, the default file name is used.

    This will emit QWebEngineProfile::downloadRequested() after the download
    has started.
*/

void QWebEnginePage::download(const QUrl& url, const QString& filename)
{
    Q_D(QWebEnginePage);
    d->ensureInitialized();
    d->adapter->download(url, filename);
}

/*!
    \fn void QWebEnginePage::loadingChanged(const QWebEngineLoadingInfo &loadingInfo)
    \since 6.2
    This signal is emitted when loading the page specified by \a loadingInfo begins,
    ends, or fails.
*/

/*!
  \property QWebEnginePage::loading
  \since 6.2

  \brief Whether the page is currently loading.

  \sa QWebEngineLoadingInfo, loadStarted, loadFinished
*/
bool QWebEnginePage::isLoading() const
{
    return d_ptr->isLoading;
}

void QWebEnginePage::load(const QUrl& url)
{
    Q_D(QWebEnginePage);
    d->adapter->load(url);
}

/*!
    \since 5.9
    Issues the specified \a request and loads the response.

    \sa load(), setUrl(), url(), urlChanged(), QUrl::fromUserInput()
*/
void QWebEnginePage::load(const QWebEngineHttpRequest& request)
{
    Q_D(QWebEnginePage);
    d->adapter->load(request);
}

void QWebEnginePage::toHtml(const std::function<void(const QString &)> &resultCallback) const
{
    Q_D(const QWebEnginePage);
    d->ensureInitialized();
    quint64 requestId = d->adapter->fetchDocumentMarkup();
    d->m_stringCallbacks.insert(requestId, resultCallback);
}

void QWebEnginePage::toPlainText(const std::function<void(const QString &)> &resultCallback) const
{
    Q_D(const QWebEnginePage);
    d->ensureInitialized();
    quint64 requestId = d->adapter->fetchDocumentInnerText();
    d->m_stringCallbacks.insert(requestId, resultCallback);
}

void QWebEnginePage::setHtml(const QString &html, const QUrl &baseUrl)
{
    setContent(html.toUtf8(), QStringLiteral("text/html;charset=UTF-8"), baseUrl);
}

void QWebEnginePage::setContent(const QByteArray &data, const QString &mimeType, const QUrl &baseUrl)
{
    Q_D(QWebEnginePage);
    d->adapter->setContent(data, mimeType, baseUrl);
}

QString QWebEnginePage::title() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->pageTitle();
}

void QWebEnginePage::setUrl(const QUrl &url)
{
    Q_D(QWebEnginePage);
    if (d->url != url) {
        d->url = url;
        emit urlChanged(url);
    }
    load(url);
}

QUrl QWebEnginePage::url() const
{
    Q_D(const QWebEnginePage);
    return d->url;
}

QUrl QWebEnginePage::requestedUrl() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->requestedUrl();
}

/*!
    \property QWebEnginePage::iconUrl
    \brief The URL of the icon associated with the page currently viewed.

    By default, this property contains an empty URL.

    \sa iconUrlChanged(), icon(), iconChanged()
*/
QUrl QWebEnginePage::iconUrl() const
{
    Q_D(const QWebEnginePage);
    return d->iconUrl;
}

/*!
    \property QWebEnginePage::icon
    \brief The icon associated with the page currently viewed.
    \since 5.7

    By default, this property contains a null icon. If touch icons are disabled
    (see \c QWebEngineSettings::TouchIconsEnabled), the favicon is provided in two sizes
    (16x16 and 32x32 pixels) encapsulated in \c{QIcon}. Otherwise, single icon is provided
    with the largest available size.

    \sa iconChanged(), iconUrl(), iconUrlChanged(), QWebEngineSettings::TouchIconsEnabled
*/
QIcon QWebEnginePage::icon() const
{
    Q_D(const QWebEnginePage);

    if (d->iconUrl.isEmpty() || !d->adapter->isInitialized())
        return QIcon();

    return d->adapter->icon();
}

qreal QWebEnginePage::zoomFactor() const
{
    Q_D(const QWebEnginePage);
    if (d->adapter->isInitialized())
        return d->adapter->currentZoomFactor();
    return d->defaultZoomFactor;
}

void QWebEnginePage::setZoomFactor(qreal factor)
{
    Q_D(QWebEnginePage);
    d->defaultZoomFactor = factor;

    if (d->adapter->isInitialized()) {
        d->adapter->setZoomFactor(factor);
        // MEMO: should reset if factor was not applied due to being invalid
        d->defaultZoomFactor = zoomFactor();
    }
}

void QWebEnginePage::runJavaScript(const QString& scriptSource, const std::function<void(const QVariant &)> &resultCallback)
{
    Q_D(QWebEnginePage);
    d->ensureInitialized();
    if (d->adapter->lifecycleState() == WebContentsAdapter::LifecycleState::Discarded) {
        qWarning("runJavaScript: disabled in Discarded state");
        if (resultCallback)
            resultCallback(QVariant());
        return;
    }
    quint64 requestId = d->adapter->runJavaScriptCallbackResult(scriptSource, QWebEngineScript::MainWorld);
    if (requestId)
        d->m_variantCallbacks.insert(requestId, resultCallback);
    else if (resultCallback)
        resultCallback(QVariant());
}

void QWebEnginePage::runJavaScript(const QString& scriptSource, quint32 worldId, const std::function<void(const QVariant &)> &resultCallback)
{
    Q_D(QWebEnginePage);
    d->ensureInitialized();
    if (d->adapter->lifecycleState() == WebContentsAdapter::LifecycleState::Discarded) {
        qWarning("runJavaScript: disabled in Discarded state");
        if (resultCallback)
            resultCallback(QVariant());
        return;
    }
    if (resultCallback) {
        quint64 requestId = d->adapter->runJavaScriptCallbackResult(scriptSource, worldId);
        if (requestId)
            d->m_variantCallbacks.insert(requestId, resultCallback);
        else
            resultCallback(QVariant());
    } else {
        d->adapter->runJavaScript(scriptSource, worldId);
    }
}

/*!
    Returns the collection of scripts that are injected into the page.

    In addition, a page might also execute scripts
    added through QWebEngineProfile::scripts().

    \sa QWebEngineScriptCollection, QWebEngineScript, {Script Injection}
*/

QWebEngineScriptCollection &QWebEnginePage::scripts()
{
    Q_D(QWebEnginePage);
    return d->scriptCollection;
}

QWebEnginePage *QWebEnginePage::createWindow(WebWindowType type)
{
    Q_D(QWebEnginePage);
    return d->view ? d->view->createPageForWindow(type) : nullptr;
}

/*!
    \since 5.11
    Returns the page this page is inspecting, if any.

    Returns \c nullptr if this page is not a developer tools page.

    \sa setInspectedPage(), devToolsPage()
*/

QWebEnginePage *QWebEnginePage::inspectedPage() const
{
    Q_D(const QWebEnginePage);
    return d->inspectedPage;
}

/*!
    \since 5.11
    Navigates this page to an internal URL that is the developer
    tools of \a page.

    This is the same as calling setDevToolsPage() on \a page
    with \c this as argument.

    \sa inspectedPage(), setDevToolsPage()
*/

void QWebEnginePage::setInspectedPage(QWebEnginePage *page)
{
    Q_D(QWebEnginePage);
    if (d->inspectedPage == page)
        return;
    QWebEnginePage *oldPage = d->inspectedPage;
    d->inspectedPage = nullptr;
    if (oldPage)
        oldPage->setDevToolsPage(nullptr);
    d->inspectedPage = page;
    if (page)
        page->setDevToolsPage(this);
}

/*!
    \since 5.11
    Returns the page that is hosting the developer tools
    of this page, if any.

    Returns \c nullptr if no developer tools page is set.

    \sa setDevToolsPage(), inspectedPage()
*/

QWebEnginePage *QWebEnginePage::devToolsPage() const
{
    Q_D(const QWebEnginePage);
    return d->devToolsPage;
}

/*!
    \since 5.11
    Binds \a devToolsPage to be the developer tools of this page.
    Triggers \a devToolsPage to navigate to an internal URL
    with the developer tools.

    This is the same as calling setInspectedPage() on \a devToolsPage
    with \c this as argument.

    \sa devToolsPage(), setInspectedPage()
*/

void QWebEnginePage::setDevToolsPage(QWebEnginePage *devToolsPage)
{
    Q_D(QWebEnginePage);
    if (d->devToolsPage == devToolsPage)
        return;
    d->ensureInitialized();
    QWebEnginePage *oldDevTools = d->devToolsPage;
    d->devToolsPage = nullptr;
    if (oldDevTools)
        oldDevTools->setInspectedPage(nullptr);
    d->devToolsPage = devToolsPage;
    if (devToolsPage)
        devToolsPage->setInspectedPage(this);
    if (d->adapter) {
        if (devToolsPage)
            d->adapter->openDevToolsFrontend(devToolsPage->d_ptr->adapter);
        else
            d->adapter->closeDevToolsFrontend();
    }
}

/*!
    \since 6.6
    Returns the id of the developer tools host associated with this page.

    If remote debugging is enabled (see \l{Qt WebEngine Developer Tools}), the id can be used to
   build the URL to connect to the developer tool websocket:
   \c{ws://localhost:<debugggin-port>/devtools/page/<id>)}. The websocket can be used to to interact
   with the page using the \l{https://chromedevtools.github.io/devtools-protocol/}{DevTools
   Protocol}.
*/

QString QWebEnginePage::devToolsId() const
{
    Q_D(const QWebEnginePage);
    d->ensureInitialized();
    return d->adapter->devToolsId();
}

ASSERT_ENUMS_MATCH(FilePickerController::Open, QWebEnginePage::FileSelectOpen)
ASSERT_ENUMS_MATCH(FilePickerController::OpenMultiple, QWebEnginePage::FileSelectOpenMultiple)
ASSERT_ENUMS_MATCH(FilePickerController::UploadFolder, QWebEnginePage::FileSelectUploadFolder)
ASSERT_ENUMS_MATCH(FilePickerController::Save, QWebEnginePage::FileSelectSave)

// TODO: remove virtuals
QStringList QWebEnginePage::chooseFiles(FileSelectionMode mode, const QStringList &oldFiles, const QStringList &acceptedMimeTypes)
{
    Q_D(const QWebEnginePage);
    return d->view ? d->view->chooseFiles(mode, oldFiles, acceptedMimeTypes) : QStringList();
}

void QWebEnginePage::javaScriptAlert(const QUrl &securityOrigin, const QString &msg)
{
    Q_UNUSED(securityOrigin);
    Q_D(const QWebEnginePage);
    if (d->view)
        d->view->javaScriptAlert(url(), msg);
}

bool QWebEnginePage::javaScriptConfirm(const QUrl &securityOrigin, const QString &msg)
{
    Q_UNUSED(securityOrigin);
    Q_D(const QWebEnginePage);
    return d->view ? d->view->javaScriptConfirm(url(), msg) : false;
}

bool QWebEnginePage::javaScriptPrompt(const QUrl &securityOrigin, const QString &msg, const QString &defaultValue, QString *result)
{
    Q_UNUSED(securityOrigin);
    Q_D(const QWebEnginePage);
    return d->view ? d->view->javaScriptPrompt(url(), msg, defaultValue, result) : false;
}

void QWebEnginePage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
{
    static QLoggingCategory loggingCategory("js", QtWarningMsg);
    static QByteArray file = sourceID.toUtf8();
    QMessageLogger logger(file.constData(), lineNumber, nullptr, loggingCategory.categoryName());

    switch (level) {
    case JavaScriptConsoleMessageLevel::InfoMessageLevel:
        if (loggingCategory.isInfoEnabled())
            logger.info().noquote() << message;
        break;
    case JavaScriptConsoleMessageLevel::WarningMessageLevel:
        if (loggingCategory.isWarningEnabled())
            logger.warning().noquote() << message;
        break;
    case JavaScriptConsoleMessageLevel::ErrorMessageLevel:
        if (loggingCategory.isCriticalEnabled())
            logger.critical().noquote() << message;
        break;
    }
}

bool QWebEnginePage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    Q_UNUSED(url);
    Q_UNUSED(type);
    Q_UNUSED(isMainFrame);
    return true;
}

QPointF QWebEnginePage::scrollPosition() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->lastScrollOffset();
}

QSizeF QWebEnginePage::contentsSize() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->lastContentsSize();
}

/*!
    \fn void QWebEnginePage::newWindowRequested(QWebEngineNewWindowRequest &request)
    \since 6.2

    This signal is emitted when \a request is issued to load a page in a separate
    web engine window. This can either be because the current page requested it explicitly
    through a JavaScript call to \c window.open, or because the user clicked on a link
    while holding Shift, Ctrl, or a built-in combination that triggers the page to open
    in a new window.

    The signal is handled by calling openIn() with the new page on the request.
    If this signal is not handled, the requested load will fail.

    \note This signal is not emitted if \l createWindow() handled the request first.

    \sa createWindow(), QWebEngineNewWindowRequest::openIn()
*/

/*!
    \fn void QWebEnginePage::pdfPrintingFinished(const QString &filePath, bool success)

    This signal is emitted when printing the web page into a PDF file has
    finished.
    \a filePath will contain the path the file was requested to be created
    at, and \a success will be \c true if the file was successfully created and
    \c false otherwise.

    \sa printToPdf()
*/

/*!
    Renders the current content of the page into a PDF document and saves it
    in the location specified in \a filePath.
    The page size and orientation of the produced PDF document are taken from
    the values specified in \a layout, while the range of pages printed is
    taken from \a ranges with the default being printing all pages.

    This method issues an asynchronous request for printing the web page into
    a PDF and returns immediately.
    To be informed about the result of the request, connect to the signal
    pdfPrintingFinished().

    \note The \l QWebEnginePage::Stop web action can be used to interrupt
    this asynchronous operation.

    If a file already exists at the provided file path, it will be overwritten.
    \sa pdfPrintingFinished()
*/
void QWebEnginePage::printToPdf(const QString &filePath, const QPageLayout &layout, const QPageRanges &ranges)
{
#if QT_CONFIG(webengine_printing_and_pdf)
    Q_D(QWebEnginePage);
    d->ensureInitialized();
    d->adapter->printToPDF(layout, ranges, filePath);
#else
    Q_UNUSED(filePath);
    Q_UNUSED(layout);
    Q_UNUSED(ranges);
#endif
}

/*!
    Renders the current content of the page into a PDF document and returns a byte array containing the PDF data
    as parameter to \a resultCallback.
    The page size and orientation of the produced PDF document are taken from the values specified in \a layout,
    while the range of pages printed is taken from \a ranges with the default being printing all pages.

    The \a resultCallback must take a const reference to a QByteArray as parameter. If printing was successful, this byte array
    will contain the PDF data, otherwise, the byte array will be empty.

    \note The \l QWebEnginePage::Stop web action can be used to interrupt this operation.

    \warning We guarantee that the callback (\a resultCallback) is always called, but it might be done
    during page destruction. When QWebEnginePage is deleted, the callback is triggered with an invalid
    value and it is not safe to use the corresponding QWebEnginePage or QWebEngineView instance inside it.
*/
void QWebEnginePage::printToPdf(const std::function<void(const QByteArray&)> &resultCallback, const QPageLayout &layout, const QPageRanges &ranges)
{
    Q_D(QWebEnginePage);
#if QT_CONFIG(webengine_printing_and_pdf)
    d->ensureInitialized();
    quint64 requestId = d->adapter->printToPDFCallbackResult(layout, ranges);
    d->m_pdfResultCallbacks.insert(requestId, resultCallback);
#else
    Q_UNUSED(layout);
    Q_UNUSED(ranges);
    if (resultCallback)
        resultCallback(QByteArray());
#endif
}

/*!
    \internal
*/
void QWebEnginePage::acceptAsNewWindow(QWebEngineNewWindowRequest &request)
{
    Q_D(QWebEnginePage);
    auto adapter = request.d_ptr->adapter;
    QUrl url = request.requestedUrl();
    if ((!adapter && !url.isValid()) || request.d_ptr->isRequestHandled) {
        qWarning("Trying to open an empty request, it was either already used or was invalidated."
            "\nYou must complete the request synchronously within the newWindowRequested signal handler."
            " If a view hasn't been adopted before returning, the request will be invalidated.");
        return;
    }

    if (!adapter)
        setUrl(url);
    else if (!d->adoptWebContents(adapter.data()))
        return;

    QRect geometry = request.requestedGeometry();
    if (!geometry.isEmpty())
        emit geometryChangeRequested(geometry);

    request.d_ptr->setHandled();
}

/*!
  \enum QWebEnginePage::LifecycleState
  \since 5.14

  This enum describes the lifecycle state of the page:

  \value  Active
  Normal state.
  \value  Frozen
  Low CPU usage state where most HTML task sources are suspended.
  \value  Discarded
  Very low resource usage state where the entire browsing context is discarded.

  \sa lifecycleState, {Page Lifecycle API}, {WebEngine Lifecycle Example}
*/

/*!
  \property QWebEnginePage::lifecycleState
  \since 5.14

  \brief The current lifecycle state of the page.

  The following restrictions are enforced by the setter:

  \list
  \li A \l{visible} page must remain in the \c{Active} state.
  \li If the page is being inspected by a \l{devToolsPage} then both pages must
      remain in the \c{Active} states.
  \li A page in the \c{Discarded} state can only transition to the \c{Active}
      state. This will cause a reload of the page.
  \endlist

  These are the only hard limits on the lifecycle state, but see also
  \l{recommendedState} for the recommended soft limits.

  \sa recommendedState, {Page Lifecycle API}, {WebEngine Lifecycle Example}
*/

QWebEnginePage::LifecycleState QWebEnginePage::lifecycleState() const
{
    Q_D(const QWebEnginePage);
    return static_cast<LifecycleState>(d->adapter->lifecycleState());
}

void QWebEnginePage::setLifecycleState(LifecycleState state)
{
    Q_D(QWebEnginePage);
    d->adapter->setLifecycleState(static_cast<WebContentsAdapterClient::LifecycleState>(state));
}

/*!
  \property QWebEnginePage::recommendedState
  \since 5.14

  \brief The recommended limit for the lifecycle state of the page.

  Setting the lifecycle state to a lower resource usage state than the
  recommended state may cause side-effects such as stopping background audio
  playback or loss of HTML form input. Setting the lifecycle state to a higher
  resource state is however completely safe.

  \sa lifecycleState, {Page Lifecycle API}, {WebEngine Lifecycle Example}
*/

QWebEnginePage::LifecycleState QWebEnginePage::recommendedState() const
{
    Q_D(const QWebEnginePage);
    return static_cast<LifecycleState>(d->adapter->recommendedState());
}

/*!
  \property QWebEnginePage::visible
  \since 5.14

  \brief Whether the page is considered visible in the Page Visibility API.

  Setting this property changes the \c{Document.hidden} and the
  \c{Document.visibilityState} properties in JavaScript which web sites can use
  to voluntarily reduce their resource usage if they are not visible to the
  user.

  If the page is connected to a \e {view} then this property will be managed
  automatically by the view according to it's own visibility.

  \sa lifecycleState
*/

bool QWebEnginePage::isVisible() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->isVisible();
}

void QWebEnginePage::setVisible(bool visible)
{
    Q_D(QWebEnginePage);

    if (!d->adapter->isInitialized()) {
        // On the one hand, it is too early to initialize here. The application
        // may call show() before load(), or it may call show() from
        // createWindow(), and then we would create an unnecessary blank
        // WebContents here. On the other hand, if the application calls show()
        // then it expects something to be shown, so we have to initialize.
        // Therefore we have to delay the initialization via the event loop.
        if (visible)
            d->wasShownTimer.start();
        else
            d->wasShownTimer.stop();
        return;
    }

    d->adapter->setVisible(visible);
}

QDataStream &operator<<(QDataStream &stream, const QWebEngineHistory &history)
{
    auto adapter = history.d_func()->adapter();
    if (!adapter->isInitialized())
        adapter->loadDefault();
    adapter->serializeNavigationHistory(stream);
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QWebEngineHistory &history)
{
    static_cast<QWebEnginePagePrivate *>(history.d_func()->client)->recreateFromSerializedHistory(stream);
    return stream;
}

QT_END_NAMESPACE

#include "moc_qwebenginepage.cpp"
