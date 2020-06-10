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

#ifndef WEB_CONTENTS_DELEGATE_QT_H
#define WEB_CONTENTS_DELEGATE_QT_H

#include "content/browser/frame_host/frame_tree_node.h"
#include "content/public/browser/media_capture_devices.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "third_party/skia/include/core/SkColor.h"

#include "base/callback.h"

#include "color_chooser_controller.h"
#include "favicon_manager.h"
#include "find_text_helper.h"
#include "javascript_dialog_manager_qt.h"

#include <QtCore/qvector.h>

QT_FORWARD_DECLARE_CLASS(CertificateErrorController)
QT_FORWARD_DECLARE_CLASS(ClientCertSelectController)

namespace content {
    class ColorChooser;
    class SiteInstance;
    class JavaScriptDialogManager;
    class WebContents;
    struct WebPreferences;
    struct ColorSuggestion;
}

namespace QtWebEngineCore {

class WebContentsAdapter;
class WebContentsAdapterClient;
class WebEngineSettings;

class FrameFocusedObserver : public content::FrameTreeNode::Observer
{
public:
    FrameFocusedObserver(WebContentsAdapterClient *adapterClient);
    ~FrameFocusedObserver();
    void addNode(content::FrameTreeNode *node);
    void removeNode(content::FrameTreeNode *node);

protected:
    void OnFrameTreeNodeFocused(content::FrameTreeNode *node) override;
    void OnFrameTreeNodeDestroyed(content::FrameTreeNode *node) override;

private:
    WebContentsAdapterClient *m_viewClient;
    QVector<content::FrameTreeNode *> m_observedNodes;
};

class SavePageInfo
{
public:
    SavePageInfo()
        : requestedFormat(-1)
    {
    }

    SavePageInfo(const QString &filePath, int format)
        : requestedFilePath(filePath), requestedFormat(format)
    {
    }

    QString requestedFilePath;
    int requestedFormat;
};

class WebContentsDelegateQt : public content::WebContentsDelegate
                            , public content::WebContentsObserver
{
public:
    WebContentsDelegateQt(content::WebContents*, WebContentsAdapterClient *adapterClient);
    ~WebContentsDelegateQt();

    QUrl url(content::WebContents *source) const;
    QString title() const { return m_title; }

    // WebContentsDelegate overrides
    content::WebContents *OpenURLFromTab(content::WebContents *source, const content::OpenURLParams &params) override;
    void NavigationStateChanged(content::WebContents* source, content::InvalidateTypes changed_flags) override;
    void AddNewContents(content::WebContents *source, std::unique_ptr<content::WebContents> new_contents, WindowOpenDisposition disposition, const gfx::Rect &initial_pos, bool user_gesture, bool *was_blocked) override;
    void CloseContents(content::WebContents *source) override;
    void LoadProgressChanged(double progress) override;
    bool HandleKeyboardEvent(content::WebContents *source, const content::NativeWebKeyboardEvent &event) override;
    content::ColorChooser* OpenColorChooser(content::WebContents *source, SkColor color, const std::vector<blink::mojom::ColorSuggestionPtr> &suggestions) override;
    void WebContentsCreated(content::WebContents *source_contents, int opener_render_process_id, int opener_render_frame_id,
                            const std::string &frame_name, const GURL &target_url, content::WebContents *new_contents) override;
    content::JavaScriptDialogManager *GetJavaScriptDialogManager(content::WebContents *source) override;
    void EnterFullscreenModeForTab(content::WebContents *web_contents, const GURL &origin, const blink::mojom::FullscreenOptions &options) override;
    void ExitFullscreenModeForTab(content::WebContents*) override;
    bool IsFullscreenForTabOrPending(const content::WebContents* web_contents) override;
    void RunFileChooser(content::RenderFrameHost* render_frame_host,
                        std::unique_ptr<content::FileSelectListener> listener,
                        const blink::mojom::FileChooserParams& params) override;
    bool DidAddMessageToConsole(content::WebContents *source, blink::mojom::ConsoleMessageLevel log_level,
                                const base::string16 &message, int32_t line_no, const base::string16 &source_id) override;
    void FindReply(content::WebContents *source, int request_id, int number_of_matches, const gfx::Rect& selection_rect, int active_match_ordinal, bool final_update) override;
    void RequestMediaAccessPermission(content::WebContents *web_contents,
                                      const content::MediaStreamRequest &request,
                                      content::MediaResponseCallback callback) override;
    void SetContentsBounds(content::WebContents *source, const gfx::Rect &bounds) override;
    void UpdateTargetURL(content::WebContents* source, const GURL& url) override;
    void RequestToLockMouse(content::WebContents *web_contents, bool user_gesture, bool last_unlocked_by_target) override;
    void BeforeUnloadFired(content::WebContents* tab, bool proceed, bool* proceed_to_fire_unload) override;
    bool CheckMediaAccessPermission(content::RenderFrameHost* render_frame_host, const GURL& security_origin, blink::mojom::MediaStreamType type) override;
    void RegisterProtocolHandler(content::WebContents* web_contents, const std::string& protocol, const GURL& url, bool user_gesture) override;
    void UnregisterProtocolHandler(content::WebContents* web_contents, const std::string& protocol, const GURL& url, bool user_gesture) override;
    bool TakeFocus(content::WebContents *source, bool reverse) override;
    void ContentsZoomChange(bool zoom_in) override;

    // WebContentsObserver overrides
    void RenderFrameCreated(content::RenderFrameHost *render_frame_host) override;
    void RenderFrameDeleted(content::RenderFrameHost *render_frame_host) override;
    void RenderProcessGone(base::TerminationStatus status) override;
    void RenderFrameHostChanged(content::RenderFrameHost *old_host, content::RenderFrameHost *new_host) override;
    void RenderViewHostChanged(content::RenderViewHost *old_host, content::RenderViewHost *new_host) override;
    void DidStartNavigation(content::NavigationHandle *navigation_handle) override;
    void DidFinishNavigation(content::NavigationHandle *navigation_handle) override;
    void DidStartLoading() override;
    void DidReceiveResponse() override;
    void DidStopLoading() override;
    void DidFailLoad(content::RenderFrameHost* render_frame_host, const GURL& validated_url, int error_code) override;
    void DidFinishLoad(content::RenderFrameHost *render_frame_host, const GURL &validated_url) override;
    void BeforeUnloadFired(bool proceed, const base::TimeTicks& proceed_time) override;
    void DidUpdateFaviconURL(const std::vector<blink::mojom::FaviconURLPtr> &candidates) override;
    void OnVisibilityChanged(content::Visibility visibility) override;
    void DidFirstVisuallyNonEmptyPaint() override;
    void ActivateContents(content::WebContents* contents) override;
    bool ShouldNavigateOnBackForwardMouseButtons() override;

    void didFailLoad(const QUrl &url, int errorCode, const QString &errorDescription);
    void overrideWebPreferences(content::WebContents *, content::WebPreferences*);
    void allowCertificateError(const QSharedPointer<CertificateErrorController> &);
    void selectClientCert(const QSharedPointer<ClientCertSelectController> &);
    void requestFeaturePermission(ProfileAdapter::PermissionType feature, const QUrl &requestingOrigin);
    void launchExternalURL(const QUrl &url, ui::PageTransition page_transition, bool is_main_frame, bool has_user_gesture);
    FaviconManager *faviconManager();
    FindTextHelper *findTextHelper();

    void setSavePageInfo(const SavePageInfo &spi) { m_savePageInfo = spi; }
    const SavePageInfo &savePageInfo() { return m_savePageInfo; }

    WebEngineSettings *webEngineSettings() const;
    WebContentsAdapter *webContentsAdapter() const;
    WebContentsAdapterClient *adapterClient() const { return m_viewClient; }

    void copyStateFrom(WebContentsDelegateQt *source);

    using LoadingState = WebContentsAdapterClient::LoadingState;
    LoadingState loadingState() const { return m_loadingState; }

    void addDevices(const blink::MediaStreamDevices &devices);
    void removeDevices(const blink::MediaStreamDevices &devices);

    bool isCapturingAudio() const { return m_audioStreamCount > 0; }
    bool isCapturingVideo() const { return m_videoStreamCount > 0; }
    bool isMirroring() const { return m_mirroringStreamCount > 0; }
    bool isCapturingDesktop() const { return m_desktopStreamCount > 0; }

    base::WeakPtr<WebContentsDelegateQt> AsWeakPtr() { return m_weakPtrFactory.GetWeakPtr(); }

private:
    QSharedPointer<WebContentsAdapter>
    createWindow(std::unique_ptr<content::WebContents> new_contents,
                 WindowOpenDisposition disposition, const gfx::Rect &initial_pos,
                 bool user_gesture);
    void EmitLoadStarted(const QUrl &url, bool isErrorPage = false);
    void EmitLoadFinished(bool success, const QUrl &url, bool isErrorPage = false, int errorCode = 0, const QString &errorDescription = QString());
    void EmitLoadCommitted();

    LoadingState determineLoadingState(content::WebContents *contents);
    void setLoadingState(LoadingState state);

    int &streamCount(blink::mojom::MediaStreamType type);

    WebContentsAdapterClient *m_viewClient;
    QVector<int64_t> m_loadingErrorFrameList;
    QScopedPointer<FaviconManager> m_faviconManager;
    QScopedPointer<FindTextHelper> m_findTextHelper;
    SavePageInfo m_savePageInfo;
    QSharedPointer<FilePickerController> m_filePickerController;
    QUrl m_initialTargetUrl;
    int m_lastLoadProgress;
    LoadingState m_loadingState;
    bool m_didStartLoadingSeen;
    FrameFocusedObserver m_frameFocusedObserver;

    QString m_title;
    int m_audioStreamCount = 0;
    int m_videoStreamCount = 0;
    int m_mirroringStreamCount = 0;
    int m_desktopStreamCount = 0;
    mutable bool m_pendingUrlUpdate = false;

    base::WeakPtrFactory<WebContentsDelegateQt> m_weakPtrFactory { this };
};

} // namespace QtWebEngineCore

#endif // WEB_CONTENTS_DELEGATE_QT_H
