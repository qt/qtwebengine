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
#include "web_contents_adapter_client.h"

#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/invalidate_type.h"
#include "content/public/common/renderer_preferences.h"

#include <QGuiApplication>
#include <QStyleHints>
#include <QUrl>

static const int kTestWindowWidth = 800;
static const int kTestWindowHeight = 600;

WebContentsDelegateQt::WebContentsDelegateQt(content::BrowserContext* browser_context, content::SiteInstance* site_instance, int routing_id, const gfx::Size& initial_size)
    : m_viewClient(0)
{
    content::WebContents::CreateParams create_params(browser_context, site_instance);
    create_params.routing_id = routing_id;
    if (!initial_size.IsEmpty())
        create_params.initial_size = initial_size;
    else
        create_params.initial_size = gfx::Size(kTestWindowWidth, kTestWindowHeight);

    m_webContents.reset(content::WebContents::Create(create_params));

    content::RendererPreferences* rendererPrefs = m_webContents->GetMutableRendererPrefs();
    rendererPrefs->use_custom_colors = true;
    // Qt returns a flash time (the whole cycle) in ms, chromium expects just the interval in seconds
    const int qtCursorFlashTime = QGuiApplication::styleHints()->cursorFlashTime();
    rendererPrefs->caret_blink_interval = 0.5 * static_cast<double>(qtCursorFlashTime) / 1000;
    m_webContents->GetRenderViewHost()->SyncRendererPrefs();

    m_webContents->SetDelegate(this);
    this->Observe(m_webContents.get());
}

void WebContentsDelegateQt::NavigationStateChanged(const content::WebContents* source, unsigned changed_flags)
{
    if (changed_flags & content::INVALIDATE_TYPE_URL) {
        Q_ASSERT(m_viewClient);
        m_viewClient->urlChanged(toQt(web_contents()->GetVisibleURL()));
    }
}

void WebContentsDelegateQt::LoadingStateChanged(content::WebContents* source)
{
    m_viewClient->loadingStateChanged();
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


content::WebContents* WebContentsDelegateQt::web_contents()
{
    return m_webContents.get();
}


