// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WEB_CONTENTS_DELEGATE_QT_H
#define WEB_CONTENTS_DELEGATE_QT_H

#include "content/browser/renderer_host/frame_tree_node.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "third_party/skia/include/core/SkColor.h"

#include "web_contents_adapter_client.h"

#include <QtCore/qlist.h>

namespace blink {
    namespace web_pref {
        struct WebPreferences;
    }
}

namespace content {
class ColorChooser;
class JavaScriptDialogManager;
class WebContents;
struct MediaStreamRequest;
}

namespace QtWebEngineCore {

class FindTextHelper;
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
    QList<content::FrameTreeNode *> m_observedNodes;
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
    void AddNewContents(content::WebContents *source, std::unique_ptr<content::WebContents> new_contents, const GURL &target_url,
                        WindowOpenDisposition disposition, const blink::mojom::WindowFeatures &window_features, bool user_gesture, bool *was_blocked) override;
    void CloseContents(content::WebContents *source) override;
    void LoadProgressChanged(double progress) override;
    bool HandleKeyboardEvent(content::WebContents *source, const content::NativeWebKeyboardEvent &event) override;
    std::unique_ptr<content::ColorChooser> OpenColorChooser(content::WebContents *source, SkColor color, const std::vector<blink::mojom::ColorSuggestionPtr> &suggestions) override;
    void WebContentsCreated(content::WebContents *source_contents, int opener_render_process_id, int opener_render_frame_id,
                            const std::string &frame_name, const GURL &target_url, content::WebContents *new_contents) override;
    content::JavaScriptDialogManager *GetJavaScriptDialogManager(content::WebContents *source) override;
    void EnterFullscreenModeForTab(content::RenderFrameHost *requesting_frame, const blink::mojom::FullscreenOptions &options) override;
    void ExitFullscreenModeForTab(content::WebContents*) override;
    bool IsFullscreenForTabOrPending(const content::WebContents* web_contents) override;
    void RunFileChooser(content::RenderFrameHost* render_frame_host,
                        scoped_refptr<content::FileSelectListener> listener,
                        const blink::mojom::FileChooserParams& params) override;
    bool DidAddMessageToConsole(content::WebContents *source, blink::mojom::ConsoleMessageLevel log_level,
                                const std::u16string &message, int32_t line_no, const std::u16string &source_id) override;
    void FindReply(content::WebContents *source, int request_id, int number_of_matches, const gfx::Rect& selection_rect, int active_match_ordinal, bool final_update) override;
    void RequestMediaAccessPermission(content::WebContents *web_contents,
                                      const content::MediaStreamRequest &request,
                                      content::MediaResponseCallback callback) override;
    void SetContentsBounds(content::WebContents *source, const gfx::Rect &bounds) override;
    void UpdateTargetURL(content::WebContents* source, const GURL& url) override;
    void RequestToLockMouse(content::WebContents *web_contents, bool user_gesture, bool last_unlocked_by_target) override;
    void BeforeUnloadFired(content::WebContents* tab, bool proceed, bool* proceed_to_fire_unload) override;
    bool CheckMediaAccessPermission(content::RenderFrameHost* render_frame_host, const GURL& security_origin, blink::mojom::MediaStreamType type) override;
    void RegisterProtocolHandler(content::RenderFrameHost* frame_host, const std::string& protocol, const GURL& url, bool user_gesture) override;
    void UnregisterProtocolHandler(content::RenderFrameHost* frame_host, const std::string& protocol, const GURL& url, bool user_gesture) override;
    bool TakeFocus(content::WebContents *source, bool reverse) override;
    void ContentsZoomChange(bool zoom_in) override;

    // WebContentsObserver overrides
    void RenderFrameCreated(content::RenderFrameHost *render_frame_host) override;
    void PrimaryMainFrameRenderProcessGone(base::TerminationStatus status) override;
    void RenderFrameHostChanged(content::RenderFrameHost *old_host, content::RenderFrameHost *new_host) override;
    void RenderViewHostChanged(content::RenderViewHost *old_host, content::RenderViewHost *new_host) override;
    void RenderViewReady() override;
    void DidStartNavigation(content::NavigationHandle *navigation_handle) override;
    void DidFinishNavigation(content::NavigationHandle *navigation_handle) override;
    void PrimaryPageChanged(content::Page &page) override;
    void DidStopLoading() override;
    void DidFailLoad(content::RenderFrameHost* render_frame_host, const GURL& validated_url, int error_code) override;
    void DidFinishLoad(content::RenderFrameHost *render_frame_host, const GURL &validated_url) override;
    void ActivateContents(content::WebContents* contents) override;
    void ResourceLoadComplete(content::RenderFrameHost* render_frame_host,
                              const content::GlobalRequestID& request_id,
                              const blink::mojom::ResourceLoadInfo& resource_load_info) override;
    void InnerWebContentsAttached(content::WebContents *inner_web_contents,
                                  content::RenderFrameHost *render_frame_host,
                                  bool is_full_page) override;

    void didFailLoad(const QUrl &url, int errorCode, const QString &errorDescription);
    void overrideWebPreferences(content::WebContents *, blink::web_pref::WebPreferences*);
    void allowCertificateError(const QSharedPointer<CertificateErrorController> &);
    void selectClientCert(const QSharedPointer<ClientCertSelectController> &);
    void requestFeaturePermission(ProfileAdapter::PermissionType feature, const QUrl &requestingOrigin);
    void launchExternalURL(const QUrl &url, ui::PageTransition page_transition, bool is_main_frame, bool has_user_gesture);
    FindTextHelper *findTextHelper();

    void setSavePageInfo(SavePageInfo *spi) { m_savePageInfo.reset(spi); }
    SavePageInfo *savePageInfo() { return m_savePageInfo.get(); }

    WebEngineSettings *webEngineSettings() const;
    WebContentsAdapter *webContentsAdapter() const;
    WebContentsAdapterClient *adapterClient() const { return m_viewClient; }

    void copyStateFrom(WebContentsDelegateQt *source);

    using LoadingState = WebContentsAdapterClient::LoadingState;
    LoadingState loadingState() const { return m_loadingState; }

    void addDevices(const blink::mojom::StreamDevices &devices);
    void removeDevices(const blink::mojom::StreamDevices &devices);
    void addDevice(const blink::MediaStreamDevice &device);
    void removeDevice(const blink::MediaStreamDevice &device);

    bool isCapturingAudio() const { return m_audioStreamCount > 0; }
    bool isCapturingVideo() const { return m_videoStreamCount > 0; }
    bool isMirroring() const { return m_mirroringStreamCount > 0; }
    bool isCapturingDesktop() const { return m_desktopStreamCount > 0; }

    base::WeakPtr<WebContentsDelegateQt> AsWeakPtr() { return m_weakPtrFactory.GetWeakPtr(); }

private:
    QSharedPointer<WebContentsAdapter>
    createWindow(std::unique_ptr<content::WebContents> new_contents,
                 WindowOpenDisposition disposition, const gfx::Rect &initial_pos,
                 const QUrl &url,
                 bool user_gesture);
    void emitLoadStarted(bool isErrorPage = false);
    void emitLoadFinished(bool isErrorPage = false);
    void emitLoadCommitted();

    LoadingState determineLoadingState(content::WebContents *contents);
    void setLoadingState(LoadingState state);

    int &streamCount(blink::mojom::MediaStreamType type);

    WebContentsAdapterClient *m_viewClient;
    QScopedPointer<FindTextHelper> m_findTextHelper;
    std::unique_ptr<SavePageInfo> m_savePageInfo;
    QSharedPointer<FilePickerController> m_filePickerController;
    LoadingState m_loadingState;
    FrameFocusedObserver m_frameFocusedObserver;

    QString m_title;
    int m_audioStreamCount = 0;
    int m_videoStreamCount = 0;
    int m_mirroringStreamCount = 0;
    int m_desktopStreamCount = 0;
    mutable bool m_pendingUrlUpdate = false;

    struct LoadingInfo {
        bool success = false;
        int progress = -1;
        bool isLoading() const { return progress >= 0; }
        QUrl url;
        bool isErrorPage = false;
        int errorCode = 0, errorDomain = 0;
        QString errorDescription;
        bool triggersErrorPage = false;
        QMultiMap<QByteArray, QByteArray> responseHeaders;
        void clear() { *this = LoadingInfo(); }
    } m_loadingInfo;

    bool m_isDocumentEmpty = true;
    base::WeakPtrFactory<WebContentsDelegateQt> m_weakPtrFactory { this };
    QList<QWeakPointer<CertificateErrorController>> m_certificateErrorControllers;
};

} // namespace QtWebEngineCore

#endif // WEB_CONTENTS_DELEGATE_QT_H
