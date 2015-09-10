/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WEB_CONTENTS_DELEGATE_QT_H
#define WEB_CONTENTS_DELEGATE_QT_H

#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"

#include "base/callback.h"

#include "javascript_dialog_manager_qt.h"
#include <QtCore/qcompilerdetection.h>

QT_FORWARD_DECLARE_CLASS(CertificateErrorController)

namespace content {
    class BrowserContext;
    class SiteInstance;
    class RenderViewHost;
    class JavaScriptDialogManager;
    class WebContents;
    struct WebPreferences;
}

namespace QtWebEngineCore {

class WebContentsAdapterClient;

class WebContentsDelegateQt : public content::WebContentsDelegate
                            , public content::WebContentsObserver
{
public:
    WebContentsDelegateQt(content::WebContents*, WebContentsAdapterClient *adapterClient);
    ~WebContentsDelegateQt() { Q_ASSERT(m_loadingErrorFrameList.isEmpty()); }
    QString lastSearchedString() const { return m_lastSearchedString; }
    void setLastSearchedString(const QString &s) { m_lastSearchedString = s; }
    int lastReceivedFindReply() const { return m_lastReceivedFindReply; }

    // WebContentsDelegate overrides
    virtual content::WebContents *OpenURLFromTab(content::WebContents *source, const content::OpenURLParams &params) Q_DECL_OVERRIDE;
    virtual void NavigationStateChanged(const content::WebContents* source, content::InvalidateTypes changed_flags) Q_DECL_OVERRIDE;
    virtual void AddNewContents(content::WebContents* source, content::WebContents* new_contents, WindowOpenDisposition disposition, const gfx::Rect& initial_pos, bool user_gesture, bool* was_blocked) Q_DECL_OVERRIDE;
    virtual void CloseContents(content::WebContents *source) Q_DECL_OVERRIDE;
    virtual void LoadProgressChanged(content::WebContents* source, double progress) Q_DECL_OVERRIDE;
    virtual void HandleKeyboardEvent(content::WebContents *source, const content::NativeWebKeyboardEvent &event) Q_DECL_OVERRIDE;
    virtual content::JavaScriptDialogManager *GetJavaScriptDialogManager() Q_DECL_OVERRIDE;
    virtual void ToggleFullscreenModeForTab(content::WebContents* web_contents, bool enter_fullscreen) Q_DECL_OVERRIDE;
    virtual bool IsFullscreenForTabOrPending(const content::WebContents* web_contents) const Q_DECL_OVERRIDE;
    virtual void RunFileChooser(content::WebContents *, const content::FileChooserParams &params) Q_DECL_OVERRIDE;
    virtual bool AddMessageToConsole(content::WebContents* source, int32 level, const base::string16& message, int32 line_no, const base::string16& source_id) Q_DECL_OVERRIDE;
    virtual void FindReply(content::WebContents *source, int request_id, int number_of_matches, const gfx::Rect& selection_rect, int active_match_ordinal, bool final_update) Q_DECL_OVERRIDE;
    virtual void RequestMediaAccessPermission(content::WebContents* web_contents, const content::MediaStreamRequest& request, const content::MediaResponseCallback& callback) Q_DECL_OVERRIDE;
    virtual void UpdateTargetURL(content::WebContents* source, const GURL& url) Q_DECL_OVERRIDE;
    virtual void RequestToLockMouse(content::WebContents *web_contents, bool user_gesture, bool last_unlocked_by_target) Q_DECL_OVERRIDE;
    virtual void ShowValidationMessage(content::WebContents *web_contents, const gfx::Rect &anchor_in_root_view, const base::string16 &main_text, const base::string16 &sub_text) Q_DECL_OVERRIDE;
    virtual void HideValidationMessage(content::WebContents *web_contents) Q_DECL_OVERRIDE;
    virtual void MoveValidationMessage(content::WebContents *web_contents, const gfx::Rect &anchor_in_root_view) Q_DECL_OVERRIDE;

    // WebContentsObserver overrides
    virtual void RenderFrameDeleted(content::RenderFrameHost *render_frame_host) Q_DECL_OVERRIDE;
    virtual void DidStartProvisionalLoadForFrame(content::RenderFrameHost *render_frame_host, const GURL &validated_url, bool is_error_page, bool is_iframe_srcdoc) Q_DECL_OVERRIDE;
    virtual void DidCommitProvisionalLoadForFrame(content::RenderFrameHost *render_frame_host, const GURL &url, ui::PageTransition transition_type) Q_DECL_OVERRIDE;
    virtual void DidFailProvisionalLoad(content::RenderFrameHost *render_frame_host, const GURL &validated_url, int error_code, const base::string16 &error_description) Q_DECL_OVERRIDE;
    virtual void DidFailLoad(content::RenderFrameHost *render_frame_host, const GURL &validated_url, int error_code, const base::string16 &error_description) Q_DECL_OVERRIDE;
    virtual void DidFinishLoad(content::RenderFrameHost *render_frame_host, const GURL &validated_url) Q_DECL_OVERRIDE;
    virtual void DidUpdateFaviconURL(const std::vector<content::FaviconURL> &candidates) Q_DECL_OVERRIDE;
    virtual void DidNavigateAnyFrame(content::RenderFrameHost *render_frame_host, const content::LoadCommittedDetails &details, const content::FrameNavigateParams &params) Q_DECL_OVERRIDE;

    void overrideWebPreferences(content::WebContents *, content::WebPreferences*);
    void allowCertificateError(const QSharedPointer<CertificateErrorController> &) ;
    void requestGeolocationPermission(const GURL &requestingFrameOrigin, const base::Callback<void (bool)> &resultCallback);
    void cancelGeolocationPermissionRequest(const GURL &requestingFrameOrigin);
    void geolocationPermissionReply(const QUrl&,  bool permission);

private:
    WebContentsAdapter *createWindow(content::WebContents *new_contents, WindowOpenDisposition disposition, const gfx::Rect& initial_pos, bool user_gesture);

    QHash<QUrl, base::Callback<void (bool)> > m_geolocationPermissionRequests;

    WebContentsAdapterClient *m_viewClient;
    QString m_lastSearchedString;
    int m_lastReceivedFindReply;
    QList<int64> m_loadingErrorFrameList;
};

} // namespace QtWebEngineCore

#endif // WEB_CONTENTS_DELEGATE_QT_H
