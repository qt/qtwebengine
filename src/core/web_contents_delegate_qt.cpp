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

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "web_contents_delegate_qt.h"

#include "browser_context_adapter.h"
#include "color_chooser_qt.h"
#include "color_chooser_controller.h"
#include "favicon_manager.h"
#include "favicon_manager_p.h"
#include "file_picker_controller.h"
#include "media_capture_devices_dispatcher.h"
#include "network_delegate_qt.h"
#include "render_widget_host_view_qt.h"
#include "type_conversion.h"
#include "visited_links_manager_qt.h"
#include "web_contents_adapter_client.h"
#include "web_contents_adapter_p.h"
#include "web_engine_context.h"
#include "web_engine_settings.h"

#include "components/web_cache/browser/web_cache_manager.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/public/browser/invalidate_type.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/favicon_url.h"
#include "content/public/common/file_chooser_params.h"
#include "content/public/common/frame_navigate_params.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/web_preferences.h"

#include <QDesktopServices>
#include <QTimer>

namespace QtWebEngineCore {

// Maps the LogSeverity defines in base/logging.h to the web engines message levels.
static WebContentsAdapterClient::JavaScriptConsoleMessageLevel mapToJavascriptConsoleMessageLevel(int32_t messageLevel) {
    if (messageLevel < 1)
        return WebContentsAdapterClient::Info;
    else if (messageLevel > 1)
        return WebContentsAdapterClient::Error;

    return WebContentsAdapterClient::Warning;
}

WebContentsDelegateQt::WebContentsDelegateQt(content::WebContents *webContents, WebContentsAdapterClient *adapterClient)
    : m_viewClient(adapterClient)
    , m_lastReceivedFindReply(0)
    , m_faviconManager(new FaviconManager(new FaviconManagerPrivate(webContents, adapterClient)))
    , m_lastLoadProgress(-1)
{
    webContents->SetDelegate(this);
    Observe(webContents);
}

content::WebContents *WebContentsDelegateQt::OpenURLFromTab(content::WebContents *source, const content::OpenURLParams &params)
{
    content::WebContents *target = source;
    if (params.disposition != WindowOpenDisposition::CURRENT_TAB) {
        QSharedPointer<WebContentsAdapter> targetAdapter = createWindow(0, params.disposition, gfx::Rect(), params.user_gesture);
        if (targetAdapter)
            target = targetAdapter->webContents();
    }
    Q_ASSERT(target);

    content::NavigationController::LoadURLParams load_url_params(params.url);
    load_url_params.source_site_instance = params.source_site_instance;
    load_url_params.referrer = params.referrer;
    load_url_params.frame_tree_node_id = params.frame_tree_node_id;
    load_url_params.redirect_chain = params.redirect_chain;
    load_url_params.transition_type = params.transition;
    load_url_params.extra_headers = params.extra_headers;
    load_url_params.should_replace_current_entry = params.should_replace_current_entry;
    load_url_params.is_renderer_initiated = params.is_renderer_initiated;
    load_url_params.started_from_context_menu = params.started_from_context_menu;
    load_url_params.override_user_agent = content::NavigationController::UA_OVERRIDE_TRUE;
    if (params.uses_post) {
        load_url_params.load_type = content::NavigationController::LOAD_TYPE_HTTP_POST;
        load_url_params.post_data = params.post_data;
    }

    target->GetController().LoadURLWithParams(load_url_params);
    return target;
}

void WebContentsDelegateQt::NavigationStateChanged(content::WebContents* source, content::InvalidateTypes changed_flags)
{
    if (changed_flags & content::INVALIDATE_TYPE_URL) {
        QUrl newUrl = toQt(source->GetVisibleURL());
        if (m_url != newUrl) {
            m_url = newUrl;
            m_viewClient->urlChanged(m_url);
        }
    }

    if (changed_flags & content::INVALIDATE_TYPE_TITLE) {
        QString newTitle = toQt(source->GetTitle());
        if (m_title != newTitle) {
            m_title = newTitle;
            m_viewClient->titleChanged(m_title);
        }
    }

    // NavigationStateChanged gets called with INVALIDATE_TYPE_TAB by AudioStateProvider::Notify,
    // whenever an audio sound gets played or stopped, this is the only way to actually figure out
    // if there was a recently played audio sound.
    // Make sure to only emit the signal when loading isn't in progress, because it causes multiple
    // false signals to be emitted.
    if ((changed_flags & content::INVALIDATE_TYPE_TAB) && !(changed_flags & content::INVALIDATE_TYPE_LOAD)) {
        m_viewClient->recentlyAudibleChanged(source->WasRecentlyAudible());
    }
}

void WebContentsDelegateQt::AddNewContents(content::WebContents* source, content::WebContents* new_contents, WindowOpenDisposition disposition, const gfx::Rect& initial_pos, bool user_gesture, bool* was_blocked)
{
    Q_UNUSED(source)
    QWeakPointer<WebContentsAdapter> newAdapter = createWindow(new_contents, disposition, initial_pos, user_gesture);
    if (was_blocked)
        *was_blocked = !newAdapter;
}

void WebContentsDelegateQt::CloseContents(content::WebContents *source)
{
    m_viewClient->close();
    GetJavaScriptDialogManager(source)->CancelDialogs(source, /* whatever?: */false);
}

void WebContentsDelegateQt::LoadProgressChanged(content::WebContents */*source*/, double progress)
{
    if (!m_loadingErrorFrameList.isEmpty())
        return;
    if (m_lastLoadProgress < 0) // suppress signals that aren't between loadStarted and loadFinished
        return;
    m_lastLoadProgress = qMax(m_lastLoadProgress, qRound(progress * 100)); // ensure monotonicity
    m_lastLoadProgress = qMin(m_lastLoadProgress, 100);
    m_viewClient->loadProgressChanged(m_lastLoadProgress);
}

void WebContentsDelegateQt::HandleKeyboardEvent(content::WebContents *, const content::NativeWebKeyboardEvent &event)
{
    Q_ASSERT(!event.skip_in_browser);
    if (event.os_event)
        m_viewClient->unhandledKeyEvent(reinterpret_cast<QKeyEvent *>(event.os_event));
}

void WebContentsDelegateQt::RenderFrameDeleted(content::RenderFrameHost *render_frame_host)
{
    m_loadingErrorFrameList.removeOne(render_frame_host->GetRoutingID());
}

void WebContentsDelegateQt::EmitLoadStarted(const QUrl &url, bool isErrorPage)
{
    if (m_lastLoadProgress >= 0) // already running
        return;
    m_viewClient->loadStarted(url, isErrorPage);
    m_viewClient->loadProgressChanged(0);
    m_lastLoadProgress = 0;
}

void WebContentsDelegateQt::DidStartNavigation(content::NavigationHandle *navigation_handle)
{
    if (!navigation_handle->IsInMainFrame())
        return;

    // Suppress extra loadStarted signal for data URL with specified base URL.
    if (navigation_handle->GetURL().SchemeIs(url::kDataScheme)) {
        content::NavigationEntry *pending_entry = navigation_handle->GetWebContents()->GetController().GetPendingEntry();

        if (pending_entry && !pending_entry->GetBaseURLForDataURL().is_empty() &&
                navigation_handle->GetURL() == pending_entry->GetURL()) {
            return;
        }
    }

    // Error-pages are not reported as separate started navigations.
    Q_ASSERT(!navigation_handle->IsErrorPage());

    m_loadingErrorFrameList.clear();
    m_faviconManager->resetCandidates();
    EmitLoadStarted(toQt(navigation_handle->GetURL()));
}

void WebContentsDelegateQt::EmitLoadFinished(bool success, const QUrl &url, bool isErrorPage, int errorCode, const QString &errorDescription)
{
    if (m_lastLoadProgress < 0) // not currently running
        return;
    m_lastLoadProgress = -1;
    m_viewClient->loadProgressChanged(100);
    m_viewClient->loadFinished(success, url, isErrorPage, errorCode, errorDescription);
}

void WebContentsDelegateQt::DidFinishNavigation(content::NavigationHandle *navigation_handle)
{
    if (!navigation_handle->IsInMainFrame())
        return;

    if (navigation_handle->HasCommitted() && !navigation_handle->IsErrorPage()) {
        BrowserContextAdapter *browserContextAdapter = m_viewClient->browserContextAdapter().data();
        // VisistedLinksMaster asserts !IsOffTheRecord().
        if (navigation_handle->ShouldUpdateHistory() && browserContextAdapter->trackVisitedLinks()) {
            for (const GURL &url : navigation_handle->GetRedirectChain())
                browserContextAdapter->visitedLinksManager()->addUrl(url);
        }

        // Make sure that we don't set the findNext WebFindOptions on a new frame.
        m_lastSearchedString = QString();

        // This is currently used for canGoBack/Forward values, which is flattened across frames. For other purposes we might have to pass is_main_frame.
        m_viewClient->loadCommitted();
    }
    // Success is reported by DidFinishLoad, but DidFailLoad is now dead code and needs to be handled below
    if (navigation_handle->GetNetErrorCode() == net::OK)
        return;

    // WebContentsObserver::DidFailLoad is not called any longer so we have to report the failure here.
    const net::Error error_code = navigation_handle->GetNetErrorCode();
    const std::string error_description = net::ErrorToString(error_code);
    didFailLoad(toQt(navigation_handle->GetURL()), error_code, toQt(error_description));

    // The load will succede as an error-page load later, and we reported the original error above
    if (navigation_handle->IsErrorPage()) {
        // Now report we are starting to load an error-page.
        m_loadingErrorFrameList.append(navigation_handle->GetRenderFrameHost()->GetRoutingID());
        m_faviconManager->resetCandidates();
        EmitLoadStarted(toQt(GURL(content::kUnreachableWebDataURL)), true);

        // If it is already committed we will not see another DidFinishNavigation call or a DidFinishLoad call.
        if (navigation_handle->HasCommitted()) {
            m_lastSearchedString = QString();
            m_viewClient->loadCommitted();
        }
    }
}

void WebContentsDelegateQt::didFailLoad(const QUrl &url, int errorCode, const QString &errorDescription)
{
    m_viewClient->iconChanged(QUrl());
    EmitLoadFinished(false /* success */ , url, false /* isErrorPage */, errorCode, errorDescription);
}

void WebContentsDelegateQt::DidFailLoad(content::RenderFrameHost* render_frame_host, const GURL& validated_url, int error_code, const base::string16& error_description, bool was_ignored_by_handler)
{
    Q_UNUSED(was_ignored_by_handler);
    if (render_frame_host->GetParent())
        return;

    if (validated_url.spec() == content::kUnreachableWebDataURL) {
        // error-pages should only ever fail due to abort:
        Q_ASSERT(error_code == -3 /* ERR_ABORTED */);
        m_loadingErrorFrameList.removeOne(render_frame_host->GetRoutingID());
        m_viewClient->iconChanged(QUrl());

        EmitLoadFinished(false /* success */, toQt(validated_url), true /* isErrorPage */);
        return;
    }

    didFailLoad(toQt(validated_url), error_code, toQt(error_description));
}

void WebContentsDelegateQt::DidFinishLoad(content::RenderFrameHost* render_frame_host, const GURL& validated_url)
{
    Q_ASSERT(validated_url.is_valid());
    if (validated_url.spec() == content::kUnreachableWebDataURL) {
        m_loadingErrorFrameList.removeOne(render_frame_host->GetRoutingID());
        m_viewClient->iconChanged(QUrl());

        // Trigger LoadFinished signal for main frame's error page only.
        if (!render_frame_host->GetParent())
            EmitLoadFinished(true /* success */, toQt(validated_url), true /* isErrorPage */);

        return;
    }

    if (render_frame_host->GetParent())
        return;

    if (!m_faviconManager->hasCandidate())
        m_viewClient->iconChanged(QUrl());

    EmitLoadFinished(true, toQt(validated_url));
}

void WebContentsDelegateQt::DidUpdateFaviconURL(const std::vector<content::FaviconURL> &candidates)
{
    QList<FaviconInfo> faviconCandidates;
    faviconCandidates.reserve(static_cast<int>(candidates.size()));
    for (const content::FaviconURL &candidate : candidates) {
        // Store invalid candidates too for later debugging via API
        faviconCandidates.append(toFaviconInfo(candidate));
    }

    m_faviconManager->update(faviconCandidates);
}

void WebContentsDelegateQt::WebContentsCreated(content::WebContents */*source_contents*/,
                                               int /*opener_render_process_id*/, int /*opener_render_frame_id*/,
                                               const std::string &/*frame_name*/,
                                               const GURL &target_url, content::WebContents */*new_contents*/)
{
    m_initialTargetUrl = toQt(target_url);
}

content::ColorChooser *WebContentsDelegateQt::OpenColorChooser(content::WebContents *source, SkColor color, const std::vector<content::ColorSuggestion> &suggestion)
{
    Q_UNUSED(suggestion);
    ColorChooserQt *colorChooser = new ColorChooserQt(source, toQt(color));
    m_viewClient->showColorDialog(colorChooser->controller());

    return colorChooser;
}
content::JavaScriptDialogManager *WebContentsDelegateQt::GetJavaScriptDialogManager(content::WebContents *)
{
    return JavaScriptDialogManagerQt::GetInstance();
}

void WebContentsDelegateQt::EnterFullscreenModeForTab(content::WebContents *web_contents, const GURL& origin)
{
    Q_UNUSED(web_contents);
    if (!m_viewClient->isFullScreenMode())
        m_viewClient->requestFullScreenMode(toQt(origin), true);
}

void WebContentsDelegateQt::ExitFullscreenModeForTab(content::WebContents *web_contents)
{
    if (m_viewClient->isFullScreenMode())
        m_viewClient->requestFullScreenMode(toQt(web_contents->GetLastCommittedURL().GetOrigin()), false);
}

bool WebContentsDelegateQt::IsFullscreenForTabOrPending(const content::WebContents* web_contents) const
{
    Q_UNUSED(web_contents);
    return m_viewClient->isFullScreenMode();
}

ASSERT_ENUMS_MATCH(FilePickerController::Open, content::FileChooserParams::Open)
ASSERT_ENUMS_MATCH(FilePickerController::OpenMultiple, content::FileChooserParams::OpenMultiple)
ASSERT_ENUMS_MATCH(FilePickerController::UploadFolder, content::FileChooserParams::UploadFolder)
ASSERT_ENUMS_MATCH(FilePickerController::Save, content::FileChooserParams::Save)

void WebContentsDelegateQt::RunFileChooser(content::RenderFrameHost *frameHost, const content::FileChooserParams &params)
{
    QStringList acceptedMimeTypes;
    acceptedMimeTypes.reserve(params.accept_types.size());
    for (std::vector<base::string16>::const_iterator it = params.accept_types.begin(); it < params.accept_types.end(); ++it)
        acceptedMimeTypes.append(toQt(*it));

    m_filePickerController.reset(new FilePickerController(static_cast<FilePickerController::FileChooserMode>(params.mode),
                                                          frameHost, toQt(params.default_file_name.value()), acceptedMimeTypes));

    // Defer the call to not block base::MessageLoop::RunTask with modal dialogs.
    QTimer::singleShot(0, [this] () {
        m_viewClient->runFileChooser(m_filePickerController);
    });
}

bool WebContentsDelegateQt::DidAddMessageToConsole(content::WebContents *source, int32_t level, const base::string16 &message, int32_t line_no, const base::string16 &source_id)
{
    Q_UNUSED(source)
    m_viewClient->javaScriptConsoleMessage(mapToJavascriptConsoleMessageLevel(level), toQt(message), static_cast<int>(line_no), toQt(source_id));
    return false;
}

void WebContentsDelegateQt::FindReply(content::WebContents *source, int request_id, int number_of_matches, const gfx::Rect& selection_rect, int active_match_ordinal, bool final_update)
{
    Q_UNUSED(source)
    Q_UNUSED(selection_rect)
    Q_UNUSED(active_match_ordinal)
    if (final_update) {
        m_lastReceivedFindReply = request_id;
        m_viewClient->didFindText(request_id, number_of_matches);
    }
}

void WebContentsDelegateQt::RequestMediaAccessPermission(content::WebContents *web_contents, const content::MediaStreamRequest &request, const content::MediaResponseCallback &callback)
{
    MediaCaptureDevicesDispatcher::GetInstance()->processMediaAccessRequest(m_viewClient, web_contents, request, callback);
}

void WebContentsDelegateQt::MoveContents(content::WebContents *source, const gfx::Rect &pos)
{
    Q_UNUSED(source)
    m_viewClient->requestGeometryChange(toQt(pos));
}

bool WebContentsDelegateQt::IsPopupOrPanel(const content::WebContents *source) const
{
    return source->HasOpener();
}

void WebContentsDelegateQt::UpdateTargetURL(content::WebContents* source, const GURL& url)
{
    Q_UNUSED(source)
    m_viewClient->didUpdateTargetURL(toQt(url));
}

void WebContentsDelegateQt::WasShown()
{
    web_cache::WebCacheManager::GetInstance()->ObserveActivity(web_contents()->GetRenderProcessHost()->GetID());
}

void WebContentsDelegateQt::DidFirstVisuallyNonEmptyPaint()
{
    RenderWidgetHostViewQt *rwhv = static_cast<RenderWidgetHostViewQt*>(web_contents()->GetRenderWidgetHostView());
    if (!rwhv)
        return;

    RenderWidgetHostViewQt::LoadVisuallyCommittedState loadVisuallyCommittedState = rwhv->getLoadVisuallyCommittedState();
    if (loadVisuallyCommittedState == RenderWidgetHostViewQt::NotCommitted) {
        rwhv->setLoadVisuallyCommittedState(RenderWidgetHostViewQt::DidFirstVisuallyNonEmptyPaint);
    } else if (loadVisuallyCommittedState == RenderWidgetHostViewQt::DidFirstCompositorFrameSwap) {
        m_viewClient->loadVisuallyCommitted();
        rwhv->setLoadVisuallyCommittedState(RenderWidgetHostViewQt::NotCommitted);
    }
}

void WebContentsDelegateQt::ActivateContents(content::WebContents* contents)
{
    WebEngineSettings *settings = m_viewClient->webEngineSettings();
    if (settings->testAttribute(settings->Attribute::AllowWindowActivationFromJavaScript))
        contents->Focus();
}

void WebContentsDelegateQt::RequestToLockMouse(content::WebContents *web_contents, bool user_gesture, bool last_unlocked_by_target)
{
    Q_UNUSED(user_gesture);

    if (last_unlocked_by_target)
        web_contents->GotResponseToLockMouseRequest(true);
    else
        m_viewClient->runMouseLockPermissionRequest(toQt(web_contents->GetVisibleURL()));
}

void WebContentsDelegateQt::overrideWebPreferences(content::WebContents *, content::WebPreferences *webPreferences)
{
    m_viewClient->webEngineSettings()->overrideWebPreferences(webPreferences);
}

QWeakPointer<WebContentsAdapter> WebContentsDelegateQt::createWindow(content::WebContents *new_contents, WindowOpenDisposition disposition, const gfx::Rect& initial_pos, bool user_gesture)
{
    QSharedPointer<WebContentsAdapter> newAdapter = QSharedPointer<WebContentsAdapter>::create(new_contents);

    m_viewClient->adoptNewWindow(newAdapter, static_cast<WebContentsAdapterClient::WindowOpenDisposition>(disposition), user_gesture, toQt(initial_pos), m_initialTargetUrl);

    // If the client didn't reference the adapter, it will be deleted now, and the weak pointer zeroed.
    return newAdapter;
}

void WebContentsDelegateQt::allowCertificateError(const QSharedPointer<CertificateErrorController> &errorController)
{
    m_viewClient->allowCertificateError(errorController);
}

void WebContentsDelegateQt::requestGeolocationPermission(const QUrl &requestingOrigin)
{
    m_viewClient->runGeolocationPermissionRequest(requestingOrigin);
}

extern WebContentsAdapterClient::NavigationType pageTransitionToNavigationType(ui::PageTransition transition);

void WebContentsDelegateQt::launchExternalURL(const QUrl &url, ui::PageTransition page_transition, bool is_main_frame)
{
    int navigationRequestAction = WebContentsAdapterClient::AcceptRequest;
    m_viewClient->navigationRequested(pageTransitionToNavigationType(page_transition), url, navigationRequestAction, is_main_frame);
#ifndef QT_NO_DESKTOPSERVICES
    if (navigationRequestAction == WebContentsAdapterClient::AcceptRequest)
        QDesktopServices::openUrl(url);
#endif
}

void WebContentsDelegateQt::ShowValidationMessage(content::WebContents *web_contents, const gfx::Rect &anchor_in_root_view, const base::string16 &main_text, const base::string16 &sub_text)
{
    Q_UNUSED(web_contents);
    m_viewClient->showValidationMessage(toQt(anchor_in_root_view), toQt(main_text), toQt(sub_text));
}

void WebContentsDelegateQt::HideValidationMessage(content::WebContents *web_contents)
{
    Q_UNUSED(web_contents);
    m_viewClient->hideValidationMessage();
}

void WebContentsDelegateQt::MoveValidationMessage(content::WebContents *web_contents, const gfx::Rect &anchor_in_root_view)
{
    Q_UNUSED(web_contents);
    m_viewClient->moveValidationMessage(toQt(anchor_in_root_view));
}

void WebContentsDelegateQt::BeforeUnloadFired(content::WebContents *tab, bool proceed, bool *proceed_to_fire_unload)
{
    Q_UNUSED(tab);
    Q_ASSERT(proceed_to_fire_unload);
    *proceed_to_fire_unload = proceed;
    if (!proceed)
        m_viewClient->windowCloseRejected();
}

void WebContentsDelegateQt::BeforeUnloadFired(const base::TimeTicks &proceed_time) {
    Q_UNUSED(proceed_time);
}

bool WebContentsDelegateQt::CheckMediaAccessPermission(content::WebContents *web_contents, const GURL& security_origin, content::MediaStreamType type)
{
    switch (type) {
    case content::MEDIA_DEVICE_AUDIO_CAPTURE:
        return m_viewClient->browserContextAdapter()->checkPermission(toQt(security_origin), BrowserContextAdapter::AudioCapturePermission);
    case content::MEDIA_DEVICE_VIDEO_CAPTURE:
        return m_viewClient->browserContextAdapter()->checkPermission(toQt(security_origin), BrowserContextAdapter::VideoCapturePermission);
    default:
        LOG(INFO) << "WebContentsDelegateQt::CheckMediaAccessPermission: "
                  << "Unsupported media stream type checked" << type;
        return false;
    }
}

FaviconManager *WebContentsDelegateQt::faviconManager()
{
    return m_faviconManager.data();
}

WebEngineSettings *WebContentsDelegateQt::webEngineSettings() const {
    return m_viewClient->webEngineSettings();
}

} // namespace QtWebEngineCore
