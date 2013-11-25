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

#ifndef WEB_CONTENTS_DELEGATE_QT_H
#define WEB_CONTENTS_DELEGATE_QT_H

#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"

#include "javascript_dialog_manager_qt.h"
#include <QtCore/qcompilerdetection.h>

namespace content {
    class BrowserContext;
    class SiteInstance;
    class RenderViewHost;
    class JavaScriptDialogManager;
    class WebContents;
}
class WebContentsAdapterClient;

class WebContentsDelegateQt : public content::WebContentsDelegate
                            , public content::WebContentsObserver
{
public:
    WebContentsDelegateQt(content::WebContents*, WebContentsAdapterClient *adapterClient);

    virtual void NavigationStateChanged(const content::WebContents* source, unsigned changed_flags) Q_DECL_OVERRIDE;
    virtual void AddNewContents(content::WebContents* source, content::WebContents* new_contents, WindowOpenDisposition disposition, const gfx::Rect& initial_pos, bool user_gesture, bool* was_blocked) Q_DECL_OVERRIDE;
    virtual void CloseContents(content::WebContents *source) Q_DECL_OVERRIDE;
    virtual void LoadingStateChanged(content::WebContents* source) Q_DECL_OVERRIDE;
    // FIXME: with the next chromium update LoadProgressChanged with progress becomes available. Q_DECL_OVERRIDE must then be activated.
    virtual void LoadProgressChanged(content::WebContents* source, double progress) /*Q_DECL_OVERRIDE*/;
    virtual void DidFailLoad(int64 frame_id, const GURL &validated_url, bool is_main_frame, int error_code, const string16 &error_description, content::RenderViewHost *render_view_host) Q_DECL_OVERRIDE;
    virtual void DidFinishLoad(int64 frame_id, const GURL &validated_url, bool is_main_frame, content::RenderViewHost *render_view_host) Q_DECL_OVERRIDE;
    virtual void DidUpdateFaviconURL(int32 page_id, const std::vector<content::FaviconURL>& candidates) Q_DECL_OVERRIDE;
    virtual void DidFailProvisionalLoad(int64 frame_id, bool is_main_frame, const GURL& validated_url, int error_code, const string16& error_description, content::RenderViewHost* render_view_host) Q_DECL_OVERRIDE;
    virtual content::JavaScriptDialogManager *GetJavaScriptDialogManager() Q_DECL_OVERRIDE;
    virtual void RunFileChooser(content::WebContents *, const content::FileChooserParams &params) Q_DECL_OVERRIDE;

private:
    WebContentsAdapterClient *m_viewClient;
};

#endif // WEB_CONTENTS_DELEGATE_QT_H
