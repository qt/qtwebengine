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

#include "web_contents_delegate_qt.h"

#include "type_conversion.h"
#include "web_contents_adapter.h"
#include "web_contents_adapter_client.h"
#include "web_engine_context.h"

#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/invalidate_type.h"
#include "content/public/common/favicon_url.h"
#include "content/public/common/file_chooser_params.h"

WebContentsDelegateQt::WebContentsDelegateQt(content::WebContents *webContents, WebContentsAdapterClient *adapterClient)
    : m_viewClient(adapterClient)
{
    webContents->SetDelegate(this);
    Observe(webContents);
}

void WebContentsDelegateQt::NavigationStateChanged(const content::WebContents* source, unsigned changed_flags)
{
    if (changed_flags & content::INVALIDATE_TYPE_URL)
        m_viewClient->urlChanged(toQt(source->GetVisibleURL()));
}

void WebContentsDelegateQt::AddNewContents(content::WebContents* source, content::WebContents* new_contents, WindowOpenDisposition disposition, const gfx::Rect& initial_pos, bool user_gesture, bool* was_blocked)
{
    WebContentsAdapter *newAdapter = new WebContentsAdapter(WebEngineContext::current()->renderingMode(), new_contents);
    // Do the first ref-count manually to be able to know if the application is handling adoptNewWindow through the public API.
    newAdapter->ref.ref();

    m_viewClient->adoptNewWindow(newAdapter, static_cast<WebContentsAdapterClient::WindowOpenDisposition>(disposition), toQt(initial_pos));

    if (!newAdapter->ref.deref()) {
        // adoptNewWindow didn't increase the ref-count, new_contents needs to be discarded.
        delete newAdapter;
        newAdapter = 0;
    }

    if (was_blocked)
        *was_blocked = !newAdapter;
}

void WebContentsDelegateQt::CloseContents(content::WebContents *source)
{
    m_viewClient->close();
}

void WebContentsDelegateQt::LoadingStateChanged(content::WebContents* source)
{
    m_viewClient->loadingStateChanged();
}

void WebContentsDelegateQt::LoadProgressChanged(content::WebContents* source, double progress)
{
    m_viewClient->loadProgressChanged(qRound(progress * 100));
}

void WebContentsDelegateQt::DidFailLoad(int64 frame_id, const GURL &validated_url, bool is_main_frame, int error_code, const string16 &error_description, content::RenderViewHost *render_view_host)
{
    if (is_main_frame)
        m_viewClient->loadFinished(false);
}

void WebContentsDelegateQt::DidFinishLoad(int64 frame_id, const GURL &validated_url, bool is_main_frame, content::RenderViewHost *render_view_host)
{
    if (is_main_frame)
        m_viewClient->loadFinished(true);
}

void WebContentsDelegateQt::DidFailProvisionalLoad(int64 frame_id, bool is_main_frame, const GURL& validated_url, int error_code, const string16& error_description, content::RenderViewHost* render_view_host)
{
    if (is_main_frame)
        DidFailLoad(frame_id, validated_url, is_main_frame, error_code, error_description, render_view_host);
}

void WebContentsDelegateQt::DidUpdateFaviconURL(int32 page_id, const std::vector<content::FaviconURL>& candidates)
{
    Q_UNUSED(page_id)
    Q_FOREACH (content::FaviconURL candidate, candidates) {
        if (candidate.icon_type == content::FaviconURL::FAVICON && !candidate.icon_url.is_empty()) {
            m_viewClient->iconChanged(toQt(candidate.icon_url));
            break;
        }
    }
}

content::JavaScriptDialogManager *WebContentsDelegateQt::GetJavaScriptDialogManager()
{
    return JavaScriptDialogManagerQt::GetInstance();
}

Q_STATIC_ASSERT_X(static_cast<int>(WebContentsAdapterClient::Open) == static_cast<int>(content::FileChooserParams::Open), "Enums out of sync");
Q_STATIC_ASSERT_X(static_cast<int>(WebContentsAdapterClient::Save) == static_cast<int>(content::FileChooserParams::Save), "Enums out of sync");

void WebContentsDelegateQt::RunFileChooser(content::WebContents *web_contents, const content::FileChooserParams &params)
{
    Q_UNUSED(web_contents)
    QStringList acceptedMimeTypes;
    acceptedMimeTypes.reserve(params.accept_types.size());
    for (std::vector<base::string16>::const_iterator it = params.accept_types.begin(); it < params.accept_types.end(); ++it)
        acceptedMimeTypes.append(toQt(*it));

    m_viewClient->runFileChooser(static_cast<WebContentsAdapterClient::FileChooserMode>(params.mode), toQt(params.default_file_name.value()), toQt(params.title)
                                 , acceptedMimeTypes);
}
