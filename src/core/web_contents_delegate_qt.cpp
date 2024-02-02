// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "web_contents_delegate_qt.h"

#include "certificate_error_controller.h"
#include "color_chooser_controller.h"
#include "color_chooser_qt.h"
#include "custom_handlers/protocol_handler_registry_factory.h"
#include "custom_handlers/register_protocol_handler_request_controller_impl.h"
#include "file_picker_controller.h"
#include "find_text_helper.h"
#include "javascript_dialog_manager_qt.h"
#include "media_capture_devices_dispatcher.h"
#include "profile_adapter.h"
#include "profile_qt.h"
#include "qwebengineloadinginfo.h"
#include "qwebengineregisterprotocolhandlerrequest.h"
#include "render_widget_host_view_qt.h"
#include "type_conversion.h"
#include "visited_links_manager_qt.h"
#include "web_contents_adapter_client.h"
#include "web_contents_adapter.h"
#include "web_contents_view_qt.h"
#include "web_engine_context.h"
#include "web_engine_error.h"
#include "web_engine_settings.h"
#include "certificate_error_controller.h"

#include "components/custom_handlers/protocol_handler_registry.h"
#include "components/web_cache/browser/web_cache_manager.h"
#include "content/browser/renderer_host/render_frame_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/file_select_listener.h"
#include "content/public/browser/invalidate_type.h"
#include "content/public/browser/media_stream_request.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/url_constants.h"
#include "net/base/data_url.h"
#include "net/base/url_util.h"
#include "third_party/blink/public/mojom/mediastream/media_stream.mojom.h"

#include <QDesktopServices>
#include <QTimer>
#include <QWindow>

namespace QtWebEngineCore {

static WebContentsAdapterClient::JavaScriptConsoleMessageLevel mapToJavascriptConsoleMessageLevel(blink::mojom::ConsoleMessageLevel log_level)
{
    switch (log_level) {
    case blink::mojom::ConsoleMessageLevel::kVerbose:
    case blink::mojom::ConsoleMessageLevel::kInfo:
        return WebContentsAdapterClient::Info;
    case blink::mojom::ConsoleMessageLevel::kWarning:
        return WebContentsAdapterClient::Warning;
    case blink::mojom::ConsoleMessageLevel::kError:
        return WebContentsAdapterClient::Error;
    }
}

WebContentsDelegateQt::WebContentsDelegateQt(content::WebContents *webContents, WebContentsAdapterClient *adapterClient)
    : m_viewClient(adapterClient)
    , m_findTextHelper(new FindTextHelper(webContents, adapterClient))
    , m_loadingState(determineLoadingState(webContents))
    , m_frameFocusedObserver(adapterClient)
{
    webContents->SetDelegate(this);
    Observe(webContents);
}

WebContentsDelegateQt::~WebContentsDelegateQt()
{
    // The destruction of this object should take place before
    // WebContents destruction since WebContentsAdapterClient
    // might be already deleted.
}

content::WebContents *WebContentsDelegateQt::OpenURLFromTab(content::WebContents *source, const content::OpenURLParams &params)
{
    content::WebContents *target = source;
    content::SiteInstance *target_site_instance = params.source_site_instance.get();
    content::Referrer referrer = params.referrer;
    if (params.disposition != WindowOpenDisposition::CURRENT_TAB) {
        QSharedPointer<WebContentsAdapter> targetAdapter = createWindow(nullptr, params.disposition, gfx::Rect(), toQt(params.url), params.user_gesture);
        if (targetAdapter) {
            if (targetAdapter->profile() != source->GetBrowserContext()) {
                target_site_instance = nullptr;
                referrer = content::Referrer();
            }
            if (!targetAdapter->isInitialized())
                targetAdapter->initialize(target_site_instance);
            target = targetAdapter->webContents();
        } else {
            return target;
        }
    }
    Q_ASSERT(target);

    content::NavigationController::LoadURLParams load_url_params(params.url);
    load_url_params.initiator_frame_token = params.initiator_frame_token;
    load_url_params.initiator_process_id = params.initiator_process_id;
    load_url_params.initiator_origin = params.initiator_origin;
    load_url_params.source_site_instance = target_site_instance;
    load_url_params.referrer = referrer;
    load_url_params.frame_tree_node_id = params.frame_tree_node_id;
    load_url_params.redirect_chain = params.redirect_chain;
    load_url_params.transition_type = params.transition;
    load_url_params.extra_headers = params.extra_headers;
    load_url_params.should_replace_current_entry = params.should_replace_current_entry;
    load_url_params.is_renderer_initiated = params.is_renderer_initiated;
    load_url_params.started_from_context_menu = params.started_from_context_menu;
    load_url_params.has_user_gesture = params.user_gesture;
    load_url_params.blob_url_loader_factory = params.blob_url_loader_factory;
    load_url_params.override_user_agent = content::NavigationController::UA_OVERRIDE_TRUE;
    load_url_params.href_translate = params.href_translate;
    load_url_params.reload_type = params.reload_type;
    load_url_params.impression = params.impression;
    if (params.post_data) {
        load_url_params.load_type = content::NavigationController::LOAD_TYPE_HTTP_POST;
        load_url_params.post_data = params.post_data;
    }

    target->GetController().LoadURLWithParams(load_url_params);
    return target;
}

static bool shouldUseActualURL(content::NavigationEntry *entry)
{
    Q_ASSERT(entry);

    // Show actual URL for data URLs only
    if (!entry->GetURL().SchemeIs(url::kDataScheme))
        return false;

    // Do not show data URL of interstitial and error pages
    if (entry->GetPageType() != content::PAGE_TYPE_NORMAL)
        return false;

    // Show the virtual URL based on custom base, if present
    if (!entry->GetBaseURLForDataURL().is_empty())
        return false;

    // Show invalid data URL
    std::string mime_type, charset, data;
    if (!net::DataURL::Parse(entry->GetURL(), &mime_type, &charset, &data))
        return false;

    // Do not show empty data URL
    return !data.empty();
}

void WebContentsDelegateQt::NavigationStateChanged(content::WebContents* source, content::InvalidateTypes changed_flags)
{
    if (changed_flags & content::INVALIDATE_TYPE_URL && !m_pendingUrlUpdate) {
        m_pendingUrlUpdate = true;
        base::WeakPtr<WebContentsDelegateQt> delegate = AsWeakPtr();
        QTimer::singleShot(0, [delegate, this](){ if (delegate) m_viewClient->urlChanged();});
    }

    if (changed_flags & content::INVALIDATE_TYPE_TITLE) {
        QString newTitle = toQt(source->GetTitle());
        if (m_title != newTitle) {
            m_title = newTitle;
            QTimer::singleShot(0, [delegate = AsWeakPtr(), title = newTitle] () {
                if (delegate)
                    delegate->adapterClient()->titleChanged(title);
            });
        }
    }

    // Make sure to only emit the signal when loading isn't in progress, because it causes multiple
    // false signals to be emitted.
    if ((changed_flags & content::INVALIDATE_TYPE_AUDIO) && !(changed_flags & content::INVALIDATE_TYPE_LOAD)) {
        m_viewClient->recentlyAudibleChanged(source->IsCurrentlyAudible());
    }
}

QUrl WebContentsDelegateQt::url(content::WebContents *source) const
{
    content::NavigationEntry *entry = source->GetController().GetVisibleEntry();
    QUrl newUrl;
    if (entry) {
        GURL url = entry->GetURL();
        // Strip user name, password and reference section from view-source URLs
        if (source->GetVisibleURL().SchemeIs(content::kViewSourceScheme) &&
            (url.has_password() || url.has_username() || url.has_ref())) {
            GURL strippedUrl = net::SimplifyUrlForRequest(url);
            newUrl = QUrl(QString("%1:%2").arg(content::kViewSourceScheme, QString::fromStdString(strippedUrl.spec())));
        }
        // If there is a visible entry there are special cases where we dont wan't to use the actual URL
        if (newUrl.isEmpty())
            newUrl = shouldUseActualURL(entry) ? toQt(url) : toQt(entry->GetVirtualURL());
    }
    m_pendingUrlUpdate = false;
    return newUrl;
}
void WebContentsDelegateQt::AddNewContents(content::WebContents *source, std::unique_ptr<content::WebContents> new_contents, const GURL &target_url,
                                           WindowOpenDisposition disposition, const blink::mojom::WindowFeatures &window_features, bool user_gesture, bool *was_blocked)
{
    Q_UNUSED(source)
    QSharedPointer<WebContentsAdapter> newAdapter = createWindow(std::move(new_contents), disposition, window_features.bounds, toQt(target_url), user_gesture);
    // Chromium can forget to pass user-agent override settings to new windows (see QTBUG-61774 and QTBUG-76249),
    // so set it here. Note the actual value doesn't really matter here. Only the second value does, but we try
    // to give the correct user-agent anyway.
    if (newAdapter)
        newAdapter->webContents()->SetUserAgentOverride(
                    blink::UserAgentOverride::UserAgentOnly(newAdapter->profileAdapter()->httpUserAgent().toStdString()),
                    true);
    if (newAdapter && !newAdapter->isInitialized())
        newAdapter->loadDefault();
    if (was_blocked)
        *was_blocked = !newAdapter;
}

void WebContentsDelegateQt::CloseContents(content::WebContents *source)
{
    GetJavaScriptDialogManager(source)->CancelDialogs(source, /* whatever?: */false);
    // Must be the last call because close() might trigger the destruction of this object.
    m_viewClient->close();
}

void WebContentsDelegateQt::LoadProgressChanged(double progress)
{
    if (!m_loadingInfo.isLoading()) // suppress signals that aren't between loadStarted and loadFinished
        return;

    int p = qMin(qRound(progress * 100), 100);
    if (p > m_loadingInfo.progress) { // ensure strict monotonic increase
        m_loadingInfo.progress = p;
        m_viewClient->loadProgressChanged(p);
    }
}

bool WebContentsDelegateQt::HandleKeyboardEvent(content::WebContents *, const content::NativeWebKeyboardEvent &event)
{
    Q_ASSERT(!event.skip_in_browser);
    if (event.os_event)
        m_viewClient->unhandledKeyEvent(reinterpret_cast<QKeyEvent *>(event.os_event));
    // FIXME: ?
    return true;
}

void WebContentsDelegateQt::RenderFrameCreated(content::RenderFrameHost *render_frame_host)
{
    content::FrameTreeNode *node = static_cast<content::RenderFrameHostImpl *>(render_frame_host)->frame_tree_node();
    m_frameFocusedObserver.addNode(node);

    // If it's a child frame (render_widget_host_view_child_frame) install an InputEventObserver on
    // it. Note that it is only needed for WheelEventAck.
    RenderWidgetHostViewQt::registerInputEventObserver(web_contents(), render_frame_host);
}

void WebContentsDelegateQt::PrimaryMainFrameRenderProcessGone(base::TerminationStatus status)
{
    // RenderProcessHost::FastShutdownIfPossible results in TERMINATION_STATUS_STILL_RUNNING
    if (status != base::TERMINATION_STATUS_STILL_RUNNING) {
        m_viewClient->renderProcessTerminated(
                m_viewClient->renderProcessExitStatus(status),
                web_contents()->GetCrashedErrorCode());
    }

    // Based one TabLoadTracker::RenderProcessGone

    if (status == base::TERMINATION_STATUS_NORMAL_TERMINATION
        || status == base::TERMINATION_STATUS_STILL_RUNNING) {
        return;
    }
    LOG(INFO) << "ProcessGone: " << int(status) << " (" << web_contents()->GetCrashedErrorCode() << ")";

    setLoadingState(LoadingState::Unloaded);
}

void WebContentsDelegateQt::RenderFrameHostChanged(content::RenderFrameHost *old_host, content::RenderFrameHost *new_host)
{
    if (old_host) {
        content::FrameTreeNode *old_node = static_cast<content::RenderFrameHostImpl *>(old_host)->frame_tree_node();
        m_frameFocusedObserver.removeNode(old_node);
    }

    if (new_host) {
        content::FrameTreeNode *new_node = static_cast<content::RenderFrameHostImpl *>(new_host)->frame_tree_node();
        m_frameFocusedObserver.addNode(new_node);

        // Is this a main frame?
        if (new_host->GetFrameOwnerElementType() == blink::FrameOwnerElementType::kNone) {
            content::RenderProcessHost *renderProcessHost = new_host->GetProcess();
            const base::Process &process = renderProcessHost->GetProcess();
            if (process.IsValid()) {
                m_viewClient->renderProcessPidChanged(process.Pid());
                m_viewClient->zoomUpdateIsNeeded();
            }
        }
    }
}

void WebContentsDelegateQt::RenderViewHostChanged(content::RenderViewHost *, content::RenderViewHost *newHost)
{
    if (newHost && newHost->GetWidget() && newHost->GetWidget()->GetView()) {
        auto rwhv = static_cast<RenderWidgetHostViewQt *>(newHost->GetWidget()->GetView());
        Q_ASSERT(rwhv->delegate());
        rwhv->delegate()->adapterClientChanged(m_viewClient);
        m_viewClient->zoomUpdateIsNeeded();
    }
}

void WebContentsDelegateQt::RenderViewReady()
{
    // The render view might have returned after a crash without us getting a RenderViewHostChanged call
    content::RenderWidgetHostView *newHostView = web_contents()->GetRenderWidgetHostView();
    if (newHostView) {
        auto *rwhv = static_cast<RenderWidgetHostViewQt *>(newHostView);
        Q_ASSERT(rwhv->delegate());
        rwhv->delegate()->updateAdapterClientIfNeeded(m_viewClient);
    }
}

void WebContentsDelegateQt::emitLoadStarted(bool isErrorPage)
{
    for (auto &&wc : m_certificateErrorControllers)
        if (auto controller = wc.lock())
            controller->deactivate();
    m_certificateErrorControllers.clear();

    // only report first ever load start or separate one for error page only
    if (!isErrorPage && m_loadingInfo.isLoading()) // already running
        return;

    m_isDocumentEmpty = true; // reset to default which may only be overridden on actual resource load complete
    if (!isErrorPage) {
        m_loadingInfo.progress = 0;
        m_viewClient->loadStarted(QWebEngineLoadingInfo(m_loadingInfo.url, QWebEngineLoadingInfo::LoadStartedStatus));
        m_viewClient->updateNavigationActions();
        m_viewClient->loadProgressChanged(0);
    }
}

void WebContentsDelegateQt::DidStartNavigation(content::NavigationHandle *navigation_handle)
{
    if (!webEngineSettings()->testAttribute(QWebEngineSettings::ErrorPageEnabled))
        navigation_handle->SetSilentlyIgnoreErrors();

    if (!navigation_handle->IsInMainFrame() || navigation_handle->IsSameDocument())
        return;

    m_loadingInfo.url = toQt(navigation_handle->GetURL());
    // IsErrorPage is only set after navigation commit, so check it otherwise: error page shouldn't have navigation entry
    bool isErrorPage = m_loadingInfo.triggersErrorPage && !navigation_handle->GetNavigationEntry();
    emitLoadStarted(isErrorPage);
}

void WebContentsDelegateQt::emitLoadFinished(bool isErrorPage)
{
    if (!m_loadingInfo.isLoading()) // not currently running
        return;

    Q_ASSERT(!isErrorPage || webEngineSettings()->testAttribute(QWebEngineSettings::ErrorPageEnabled));
    Q_ASSERT((m_loadingInfo.triggersErrorPage && webEngineSettings()->testAttribute(QWebEngineSettings::ErrorPageEnabled)) || !m_loadingInfo.triggersErrorPage);

    if (isErrorPage) {
        m_loadingInfo.isErrorPage = isErrorPage;
        return;
    }

    if (m_loadingInfo.progress < 100) {
        m_loadingInfo.progress = 100;
        m_viewClient->loadProgressChanged(100);
    }

    auto loadStatus = m_loadingInfo.success
        ? QWebEngineLoadingInfo::LoadSucceededStatus
        : (m_loadingInfo.errorCode == WebEngineError::UserAbortedError
                ? QWebEngineLoadingInfo::LoadStoppedStatus : QWebEngineLoadingInfo::LoadFailedStatus);
    QWebEngineLoadingInfo info(m_loadingInfo.url, loadStatus, m_loadingInfo.isErrorPage,
                               m_loadingInfo.errorDescription, m_loadingInfo.errorCode,
                               QWebEngineLoadingInfo::ErrorDomain(m_loadingInfo.errorDomain),
                               m_loadingInfo.responseHeaders);
    m_viewClient->loadFinished(std::move(info));
    m_viewClient->updateNavigationActions();
}

void WebContentsDelegateQt::emitLoadCommitted()
{
    m_findTextHelper->handleLoadCommitted();
    m_viewClient->loadCommitted();
    m_viewClient->updateNavigationActions();
}

void WebContentsDelegateQt::DidFinishNavigation(content::NavigationHandle *navigation_handle)
{
    if (!navigation_handle->IsInMainFrame())
        return;

    if (navigation_handle->HasCommitted() && !navigation_handle->IsErrorPage()) {
        ProfileAdapter *profileAdapter = m_viewClient->profileAdapter();
        // VisistedLinksMaster asserts !IsOffTheRecord().
        if (navigation_handle->ShouldUpdateHistory() && profileAdapter->trackVisitedLinks()) {
            for (const GURL &url : navigation_handle->GetRedirectChain())
                profileAdapter->visitedLinksManager()->addUrl(url);
        }

        emitLoadCommitted();
    }

    const net::HttpResponseHeaders * const responseHeaders = navigation_handle->GetResponseHeaders();
    if (responseHeaders != nullptr) {
        m_loadingInfo.responseHeaders.clear();
        std::size_t iter = 0;
        std::string headerName;
        std::string headerValue;
        while (responseHeaders->EnumerateHeaderLines(&iter, &headerName, &headerValue)) {
            m_loadingInfo.responseHeaders.insert(
                        QByteArray::fromStdString(headerName),
                        QByteArray::fromStdString(headerValue)
            );
        }
    }

    // Success is reported by DidFinishLoad, but DidFailLoad is now dead code and needs to be handled below
    if (navigation_handle->GetNetErrorCode() == net::OK)
        return;

    // WebContentsObserver::DidFailLoad is not called any longer so we have to report the failure here.
    int error_code = navigation_handle->GetNetErrorCode();
    if (error_code == net::ERR_HTTP_RESPONSE_CODE_FAILURE)
        if (auto entry = web_contents()->GetController().GetActiveEntry())
            error_code = entry->GetHttpStatusCode();
    didFailLoad(toQt(navigation_handle->GetURL()), error_code, WebEngineError::toQtErrorDescription(error_code));

    // The load will succede as an error-page load later, and we reported the original error above
    if (navigation_handle->IsErrorPage()) {
        // Now report we are starting to load an error-page.

        // If it is already committed we will not see another DidFinishNavigation call or a DidFinishLoad call.
        if (navigation_handle->HasCommitted())
            emitLoadCommitted();
    }
}

void WebContentsDelegateQt::PrimaryPageChanged(content::Page &)
{
    // Based on TabLoadTracker::PrimaryPageChanged

    if (!web_contents()->ShouldShowLoadingUI())
        return;

    // A transition to loading requires both DidStartLoading (navigation
    // committed) and DidReceiveResponse (data has been transmitted over the
    // network) events to occur. This is because NavigationThrottles can block
    // actual network requests, but not the rest of the state machinery.
    setLoadingState(LoadingState::Loading);
}

void WebContentsDelegateQt::DidStopLoading()
{
    // Based on TabLoadTracker::DidStopLoading

    // NOTE: PageAlmostIdle feature not implemented

    setLoadingState(LoadingState::Loaded);

    emitLoadFinished();
    m_loadingInfo.clear();
}

void WebContentsDelegateQt::didFailLoad(const QUrl &url, int errorCode, const QString &errorDescription)
{
    m_viewClient->iconChanged(QUrl());
    bool errorPageEnabled = webEngineSettings()->testAttribute(QWebEngineSettings::ErrorPageEnabled);
    // Delay notifying failure until the error-page is done loading.
    // Error-pages are not loaded on failures due to abort.
    bool aborted = (errorCode == -3 /* ERR_ABORTED*/ );
    m_loadingInfo.success = false;
    m_loadingInfo.url = url;
    m_loadingInfo.errorCode = errorCode;
    m_loadingInfo.errorDomain = WebEngineError::toQtErrorDomain(errorCode);
    m_loadingInfo.errorDescription = errorDescription;
    m_loadingInfo.triggersErrorPage = errorPageEnabled && !aborted;
}

void WebContentsDelegateQt::DidFailLoad(content::RenderFrameHost* render_frame_host, const GURL& validated_url, int error_code)
{
    setLoadingState(LoadingState::Loaded);

    if (render_frame_host != web_contents()->GetPrimaryMainFrame())
        return;

    if (validated_url.spec() == content::kUnreachableWebDataURL) {
        // error-pages should only ever fail due to abort:
        Q_ASSERT(error_code == -3 /* ERR_ABORTED */);
        m_viewClient->iconChanged(QUrl());
        emitLoadFinished(/* isErrorPage = */true);
        return;
    }
    didFailLoad(toQt(validated_url), error_code, WebEngineError::toQtErrorDescription(error_code));
}

void WebContentsDelegateQt::DidFinishLoad(content::RenderFrameHost* render_frame_host, const GURL& validated_url)
{
    Q_ASSERT(validated_url.is_valid());
    if (validated_url.spec() == content::kUnreachableWebDataURL) {
        // Trigger LoadFinished signal for main frame's error page only.
        if (!render_frame_host->GetParent()) {
            m_viewClient->iconChanged(QUrl());
            emitLoadFinished(/* isErrorPage = */true);
        }

        return;
    }

    if (render_frame_host->GetParent()) {
        m_viewClient->updateNavigationActions();
        return;
    }

    content::NavigationEntry *entry = web_contents()->GetController().GetActiveEntry();
    int http_statuscode = entry ? entry->GetHttpStatusCode() : 0;
    bool errorPageEnabled = webEngineSettings()->testAttribute(QWebEngineSettings::ErrorPageEnabled);
    bool triggersErrorPage = errorPageEnabled && (http_statuscode >= 400) && m_isDocumentEmpty;

    m_loadingInfo.success = http_statuscode < 400;
    m_loadingInfo.url = toQt(validated_url);
    m_loadingInfo.errorCode = http_statuscode;
    m_loadingInfo.errorDomain = WebEngineError::toQtErrorDomain(http_statuscode);
    m_loadingInfo.errorDescription = WebEngineError::toQtErrorDescription(http_statuscode);
    m_loadingInfo.triggersErrorPage = triggersErrorPage;
}

void WebContentsDelegateQt::WebContentsCreated(content::WebContents * /*source_contents*/,
                                               int /*opener_render_process_id*/, int /*opener_render_frame_id*/,
                                               const std::string &/*frame_name*/,
                                               const GURL &/*target_url*/, content::WebContents *newContents)
{
    if (auto *view = static_cast<content::WebContentsImpl *>(newContents)->GetView())
        static_cast<WebContentsViewQt *>(view)->setFactoryClient(m_viewClient);
}

std::unique_ptr<content::ColorChooser> WebContentsDelegateQt::OpenColorChooser(content::WebContents *source, SkColor color, const std::vector<blink::mojom::ColorSuggestionPtr> &suggestion)
{
    Q_UNUSED(suggestion);
    auto colorChooser = std::make_unique<ColorChooserQt>(source, toQt(color));
    m_viewClient->showColorDialog(colorChooser->controller());

    return colorChooser;
}
content::JavaScriptDialogManager *WebContentsDelegateQt::GetJavaScriptDialogManager(content::WebContents *)
{
    return JavaScriptDialogManagerQt::GetInstance();
}

void WebContentsDelegateQt::EnterFullscreenModeForTab(content::RenderFrameHost *requesting_frame, const blink::mojom::FullscreenOptions &options)
{
    Q_UNUSED(options);
    if (!m_viewClient->isFullScreenMode())
        m_viewClient->requestFullScreenMode(toQt(requesting_frame->GetLastCommittedURL().DeprecatedGetOriginAsURL()), true);
}

void WebContentsDelegateQt::ExitFullscreenModeForTab(content::WebContents *web_contents)
{
    if (m_viewClient->isFullScreenMode())
        m_viewClient->requestFullScreenMode(toQt(web_contents->GetLastCommittedURL().DeprecatedGetOriginAsURL()), false);
}

bool WebContentsDelegateQt::IsFullscreenForTabOrPending(const content::WebContents* web_contents)
{
    Q_UNUSED(web_contents);
    return m_viewClient->isFullScreenMode();
}

ASSERT_ENUMS_MATCH(FilePickerController::Open, blink::mojom::FileChooserParams::Mode::kOpen)
ASSERT_ENUMS_MATCH(FilePickerController::OpenMultiple, blink::mojom::FileChooserParams::Mode::kOpenMultiple)
ASSERT_ENUMS_MATCH(FilePickerController::UploadFolder, blink::mojom::FileChooserParams::Mode::kUploadFolder)
ASSERT_ENUMS_MATCH(FilePickerController::Save, blink::mojom::FileChooserParams::Mode::kSave)

extern FilePickerController *createFilePickerController(FilePickerController::FileChooserMode mode, scoped_refptr<content::FileSelectListener> listener, const QString &defaultFileName, const QStringList &acceptedMimeTypes, QObject *parent = nullptr);

void WebContentsDelegateQt::RunFileChooser(content::RenderFrameHost * /*frameHost*/,
                                           scoped_refptr<content::FileSelectListener> listener,
                                           const blink::mojom::FileChooserParams& params)
{
    QStringList acceptedMimeTypes;
    acceptedMimeTypes.reserve(params.accept_types.size());
    for (std::vector<std::u16string>::const_iterator it = params.accept_types.begin(); it < params.accept_types.end(); ++it)
        acceptedMimeTypes.append(toQt(*it));

    m_filePickerController.reset(createFilePickerController(static_cast<FilePickerController::FileChooserMode>(params.mode),
                                                            listener, toQt(params.default_file_name.value()), acceptedMimeTypes));

    // Defer the call to not block base::MessageLoop::RunTask with modal dialogs.
    QTimer::singleShot(0, [this] () {
        m_viewClient->runFileChooser(m_filePickerController);
    });
}

bool WebContentsDelegateQt::DidAddMessageToConsole(content::WebContents *source, blink::mojom::ConsoleMessageLevel log_level,
                                                   const std::u16string &message, int32_t line_no, const std::u16string &source_id)
{
    Q_UNUSED(source);
    m_viewClient->javaScriptConsoleMessage(mapToJavascriptConsoleMessageLevel(log_level), toQt(message), static_cast<int>(line_no), toQt(source_id));
    return false;
}

void WebContentsDelegateQt::FindReply(content::WebContents *source, int request_id, int number_of_matches, const gfx::Rect& selection_rect, int active_match_ordinal, bool final_update)
{
    m_findTextHelper->handleFindReply(source, request_id, number_of_matches, selection_rect, active_match_ordinal, final_update);
}

void WebContentsDelegateQt::RequestMediaAccessPermission(content::WebContents *web_contents, const content::MediaStreamRequest &request,  content::MediaResponseCallback callback)
{
    MediaCaptureDevicesDispatcher::GetInstance()->processMediaAccessRequest(web_contents, request, std::move(callback));
}

void WebContentsDelegateQt::SetContentsBounds(content::WebContents *source, const gfx::Rect &bounds)
{
    if (!source->HasOpener()) // is popup
        return;

    QRect frameGeometry(toQt(bounds));
    QRect geometry;
    if (RenderWidgetHostViewQt *rwhv = static_cast<RenderWidgetHostViewQt*>(web_contents()->GetRenderWidgetHostView())) {
        if (rwhv->delegate() && rwhv->delegate()->Window())
            geometry = frameGeometry.marginsRemoved(rwhv->delegate()->Window()->frameMargins());
    }
    m_viewClient->requestGeometryChange(geometry, frameGeometry);
}

void WebContentsDelegateQt::UpdateTargetURL(content::WebContents* source, const GURL& url)
{
    Q_UNUSED(source);
    m_viewClient->didUpdateTargetURL(toQt(url));
}

void WebContentsDelegateQt::ActivateContents(content::WebContents* contents)
{
    QWebEngineSettings *settings = m_viewClient->webEngineSettings();
    if (settings->testAttribute(QWebEngineSettings::AllowWindowActivationFromJavaScript))
        contents->Focus();
}

void WebContentsDelegateQt::RequestToLockMouse(content::WebContents *web_contents, bool user_gesture, bool last_unlocked_by_target)
{
    Q_UNUSED(user_gesture);

    if (last_unlocked_by_target)
        web_contents->GotResponseToLockMouseRequest(blink::mojom::PointerLockResult::kSuccess);
    else
        m_viewClient->runMouseLockPermissionRequest(toQt(web_contents->GetLastCommittedURL().DeprecatedGetOriginAsURL()));
}

void WebContentsDelegateQt::overrideWebPreferences(content::WebContents *webContents, blink::web_pref::WebPreferences *webPreferences)
{
    WebEngineSettings::get(m_viewClient->webEngineSettings())->overrideWebPreferences(webContents, webPreferences);
}

QSharedPointer<WebContentsAdapter>
WebContentsDelegateQt::createWindow(std::unique_ptr<content::WebContents> new_contents,
                                    WindowOpenDisposition disposition, const gfx::Rect &initial_pos, const QUrl &url,
                                    bool user_gesture)
{
    QSharedPointer<WebContentsAdapter> newAdapter = QSharedPointer<WebContentsAdapter>::create(std::move(new_contents));

    return m_viewClient->adoptNewWindow(
            std::move(newAdapter),
            static_cast<WebContentsAdapterClient::WindowOpenDisposition>(disposition), user_gesture,
            toQt(initial_pos), url);
}

void WebContentsDelegateQt::allowCertificateError(
        const QSharedPointer<CertificateErrorController> &controller)
{
    QWebEngineCertificateError error(controller);
    m_viewClient->allowCertificateError(error);
    if (!error.isOverridable() || (!controller->deferred() && !controller->answered()))
        error.rejectCertificate();
    else
        m_certificateErrorControllers.append(controller);
}

void WebContentsDelegateQt::selectClientCert(const QSharedPointer<ClientCertSelectController> &selectController)
{
    m_viewClient->selectClientCert(selectController);
}

void WebContentsDelegateQt::requestFeaturePermission(ProfileAdapter::PermissionType feature, const QUrl &requestingOrigin)
{
    m_viewClient->runFeaturePermissionRequest(feature, requestingOrigin);
}

extern WebContentsAdapterClient::NavigationType pageTransitionToNavigationType(ui::PageTransition transition);

void WebContentsDelegateQt::launchExternalURL(const QUrl &url, ui::PageTransition page_transition, bool is_main_frame, bool has_user_gesture)
{
    QWebEngineSettings *settings = m_viewClient->webEngineSettings();
    bool navigationAllowedByPolicy = false;
    bool navigationRequestAccepted = true;

    switch (settings->unknownUrlSchemePolicy()) {
    case QWebEngineSettings::DisallowUnknownUrlSchemes:
        break;
    case QWebEngineSettings::AllowUnknownUrlSchemesFromUserInteraction:
        navigationAllowedByPolicy = has_user_gesture;
        break;
    case QWebEngineSettings::AllowAllUnknownUrlSchemes:
        navigationAllowedByPolicy = true;
        break;
    default:
        Q_UNREACHABLE();
    }

    if (navigationAllowedByPolicy) {
        m_viewClient->navigationRequested(pageTransitionToNavigationType(page_transition), url, navigationRequestAccepted, is_main_frame);
#if QT_CONFIG(desktopservices)
        if (navigationRequestAccepted)
            QDesktopServices::openUrl(url);
#endif
    }

    if (!navigationAllowedByPolicy || !navigationRequestAccepted) {
        QString errorDescription;
        if (!navigationAllowedByPolicy)
            errorDescription = QStringLiteral("Launching external protocol forbidden by WebEngineSettings::UnknownUrlSchemePolicy");
        else
            errorDescription = QStringLiteral("Launching external protocol suppressed by 'navigationRequested' API");
        didFailLoad(url, net::Error::ERR_ABORTED, errorDescription);
    }
}

void WebContentsDelegateQt::BeforeUnloadFired(content::WebContents *tab, bool proceed, bool *proceed_to_fire_unload)
{
    Q_UNUSED(tab);
    Q_ASSERT(proceed_to_fire_unload);
    *proceed_to_fire_unload = proceed;
    if (!proceed)
        m_viewClient->windowCloseRejected();
}

bool WebContentsDelegateQt::CheckMediaAccessPermission(content::RenderFrameHost *, const GURL& security_origin, blink::mojom::MediaStreamType type)
{
    switch (type) {
    case blink::mojom::MediaStreamType::DEVICE_AUDIO_CAPTURE:
        return m_viewClient->profileAdapter()->checkPermission(toQt(security_origin), ProfileAdapter::AudioCapturePermission);
    case blink::mojom::MediaStreamType::DEVICE_VIDEO_CAPTURE:
        return m_viewClient->profileAdapter()->checkPermission(toQt(security_origin), ProfileAdapter::VideoCapturePermission);
    default:
        LOG(INFO) << "WebContentsDelegateQt::CheckMediaAccessPermission: "
                  << "Unsupported media stream type checked " << type;
        return false;
    }
}

void WebContentsDelegateQt::RegisterProtocolHandler(content::RenderFrameHost *frameHost, const std::string &protocol, const GURL &url, bool)
{
    content::BrowserContext *context = frameHost->GetBrowserContext();

    custom_handlers::ProtocolHandler handler =
        custom_handlers::ProtocolHandler::CreateProtocolHandler(protocol, url);

    custom_handlers::ProtocolHandlerRegistry *registry =
        ProtocolHandlerRegistryFactory::GetForBrowserContext(context);
    if (registry->SilentlyHandleRegisterHandlerRequest(handler))
        return;

    QWebEngineRegisterProtocolHandlerRequest request(
        QSharedPointer<RegisterProtocolHandlerRequestControllerImpl>::create(content::WebContents::FromRenderFrameHost(frameHost), handler));
    m_viewClient->runRegisterProtocolHandlerRequest(std::move(request));
}

void WebContentsDelegateQt::UnregisterProtocolHandler(content::RenderFrameHost *frameHost, const std::string &protocol, const GURL &url, bool)
{
    content::BrowserContext* context = frameHost->GetBrowserContext();

    custom_handlers::ProtocolHandler handler =
        custom_handlers::ProtocolHandler::CreateProtocolHandler(protocol, url);

    custom_handlers::ProtocolHandlerRegistry* registry =
        ProtocolHandlerRegistryFactory::GetForBrowserContext(context);
    registry->RemoveHandler(handler);
}

bool WebContentsDelegateQt::TakeFocus(content::WebContents *source, bool reverse)
{
    Q_UNUSED(source);
    return m_viewClient->passOnFocus(reverse);
}

void WebContentsDelegateQt::ContentsZoomChange(bool zoom_in)
{
    WebContentsAdapter *adapter = webContentsAdapter();
    if (zoom_in)
        adapter->setZoomFactor(adapter->currentZoomFactor() + 0.1f);
    else
        adapter->setZoomFactor(adapter->currentZoomFactor() - 0.1f);
}

void WebContentsDelegateQt::ResourceLoadComplete(content::RenderFrameHost* render_frame_host,
                                                 const content::GlobalRequestID& request_id,
                                                 const blink::mojom::ResourceLoadInfo& resource_load_info)
{
    Q_UNUSED(render_frame_host);
    Q_UNUSED(request_id);

    if (resource_load_info.request_destination == network::mojom::RequestDestination::kDocument) {
        m_isDocumentEmpty = (resource_load_info.raw_body_bytes == 0);
    }
}

void WebContentsDelegateQt::InnerWebContentsAttached(content::WebContents *inner_web_contents,
                                        content::RenderFrameHost *render_frame_host,
                                        bool is_full_page)
{
    blink::web_pref::WebPreferences guestPrefs = inner_web_contents->GetOrCreateWebPreferences();
    webEngineSettings()->overrideWebPreferences(inner_web_contents, &guestPrefs);
    inner_web_contents->SetWebPreferences(guestPrefs);
}

FindTextHelper *WebContentsDelegateQt::findTextHelper()
{
    return m_findTextHelper.data();
}

WebEngineSettings *WebContentsDelegateQt::webEngineSettings() const {
   return WebEngineSettings::get(m_viewClient->webEngineSettings());
}

WebContentsAdapter *WebContentsDelegateQt::webContentsAdapter() const
{
    return m_viewClient->webContentsAdapter();
}

void WebContentsDelegateQt::copyStateFrom(WebContentsDelegateQt *source)
{
    m_title = source->m_title;
    NavigationStateChanged(web_contents(), content::INVALIDATE_TYPE_URL);
}

WebContentsDelegateQt::LoadingState WebContentsDelegateQt::determineLoadingState(content::WebContents *contents)
{
    // Based on TabLoadTracker::DetermineLoadingState

    if (contents->ShouldShowLoadingUI() && !contents->IsWaitingForResponse())
        return LoadingState::Loading;

    content::NavigationController &controller = contents->GetController();
    if (controller.GetLastCommittedEntry() != nullptr && !controller.IsInitialNavigation() && !controller.NeedsReload())
        return LoadingState::Loaded;

    return LoadingState::Unloaded;
}

void WebContentsDelegateQt::setLoadingState(LoadingState state)
{
    if (m_loadingState == state)
        return;

    m_loadingState = state;

    webContentsAdapter()->updateRecommendedState();
}

int &WebContentsDelegateQt::streamCount(blink::mojom::MediaStreamType type)
{
    // Based on MediaStreamCaptureIndicator::WebContentsDeviceUsage::GetStreamCount
    switch (type) {
    case blink::mojom::MediaStreamType::DEVICE_AUDIO_CAPTURE:
        return m_audioStreamCount;

    case blink::mojom::MediaStreamType::DEVICE_VIDEO_CAPTURE:
        return m_videoStreamCount;

    case blink::mojom::MediaStreamType::GUM_TAB_AUDIO_CAPTURE:
    case blink::mojom::MediaStreamType::GUM_TAB_VIDEO_CAPTURE:
        return m_mirroringStreamCount;

    case blink::mojom::MediaStreamType::GUM_DESKTOP_VIDEO_CAPTURE:
    case blink::mojom::MediaStreamType::GUM_DESKTOP_AUDIO_CAPTURE:
    case blink::mojom::MediaStreamType::DISPLAY_VIDEO_CAPTURE:
    case blink::mojom::MediaStreamType::DISPLAY_AUDIO_CAPTURE:
    case blink::mojom::MediaStreamType::DISPLAY_VIDEO_CAPTURE_THIS_TAB:
    case blink::mojom::MediaStreamType::DISPLAY_VIDEO_CAPTURE_SET:
        return m_desktopStreamCount;

    case blink::mojom::MediaStreamType::NO_SERVICE:
    case blink::mojom::MediaStreamType::NUM_MEDIA_TYPES:
        NOTREACHED();
        return m_videoStreamCount;
    }
    NOTREACHED();
    return m_videoStreamCount;
}

void WebContentsDelegateQt::addDevices(const blink::mojom::StreamDevices &devices)
{
    if (devices.audio_device.has_value())
        addDevice(devices.audio_device.value());
    if (devices.video_device.has_value())
        addDevice(devices.video_device.value());

    webContentsAdapter()->updateRecommendedState();
}

void WebContentsDelegateQt::removeDevices(const blink::mojom::StreamDevices &devices)
{
    if (devices.audio_device.has_value())
        removeDevice(devices.audio_device.value());
    if (devices.video_device.has_value())
        removeDevice(devices.video_device.value());

    webContentsAdapter()->updateRecommendedState();
}

void WebContentsDelegateQt::addDevice(const blink::MediaStreamDevice &device)
{
    ++streamCount(device.type);
}

void WebContentsDelegateQt::removeDevice(const blink::MediaStreamDevice &device)
{
    --streamCount(device.type);
}

FrameFocusedObserver::FrameFocusedObserver(WebContentsAdapterClient *adapterClient)
    : m_viewClient(adapterClient)
{}

void FrameFocusedObserver::addNode(content::FrameTreeNode *node)
{
   if (m_observedNodes.contains(node))
       return;

   node->AddObserver(this);
   m_observedNodes.append(node);
}

void FrameFocusedObserver::removeNode(content::FrameTreeNode *node)
{
    node->RemoveObserver(this);
    m_observedNodes.removeOne(node);
}

void FrameFocusedObserver::OnFrameTreeNodeFocused(content::FrameTreeNode *node)
{
    Q_UNUSED(node);
    m_viewClient->updateEditActions();
}

void FrameFocusedObserver::OnFrameTreeNodeDestroyed(content::FrameTreeNode *node)
{
    m_observedNodes.removeOne(node);
    m_viewClient->updateEditActions();
}

FrameFocusedObserver::~FrameFocusedObserver()
{
    for (content::FrameTreeNode *node : m_observedNodes)
        node->RemoveObserver(this);
}

} // namespace QtWebEngineCore
