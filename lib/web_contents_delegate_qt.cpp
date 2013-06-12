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

#include "content/public/browser/web_contents.h"
#include "content/public/browser/navigation_controller.h"

#include "web_contents_view_qt.h"
#include "qwebcontentsview.h"
#include "qquickwebcontentsview.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLineEdit>
#include <QQuickView>
#include <QGuiApplication>
#include <QStyleHints>

static const int kTestWindowWidth = 800;
static const int kTestWindowHeight = 600;

std::vector<WebContentsDelegateQt*> WebContentsDelegateQt::m_windows;

WebContentsDelegateQt::WebContentsDelegateQt(content::WebContents* web_contents)
    : m_webContents(web_contents)
{
    m_webContents->SetDelegate(this);
	// const CommandLine& command_line = *CommandLine::ForCurrentProcess();
	// registrar_.Add(this, NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED,
	// Source<WebContents>(web_contents));
	m_windows.push_back(this);

	// if (!shell_created_callback_.is_null()) {
	// 	shell_created_callback_.Run(this);
	// 	shell_created_callback_.Reset();
	// }
}

WebContentsDelegateQt* WebContentsDelegateQt::CreateNewWindow(content::BrowserContext* browser_context, const GURL& url, content::SiteInstance* site_instance, int routing_id, const gfx::Size& initial_size)
{
    content::WebContents::CreateParams create_params(browser_context, site_instance);
    create_params.routing_id = routing_id;
    if (!initial_size.IsEmpty())
        create_params.initial_size = initial_size;
    else
        create_params.initial_size = gfx::Size(kTestWindowWidth, kTestWindowHeight);
    content::WebContents::Create(create_params);
    content::WebContents* web_contents = content::WebContents::Create(create_params);
    WebContentsDelegateQt* delegate = new WebContentsDelegateQt(web_contents);

    content::RendererPreferences* rendererPrefs = delegate->m_webContents->GetMutableRendererPrefs();
    rendererPrefs->use_custom_colors = true;
    // Qt returns a flash time (the whole cycle) in ms, chromium expects just the interval in seconds
    const int qtCursorFlashTime = QGuiApplication::styleHints()->cursorFlashTime();
    rendererPrefs->caret_blink_interval = 0.5 * static_cast<double>(qtCursorFlashTime) / 1000;
    delegate->m_webContents->GetRenderViewHost()->SyncRendererPrefs();

    if (!url.is_empty())
        delegate->LoadURL(url);
    return delegate;
}

content::WebContents* WebContentsDelegateQt::web_contents()
{
    return m_webContents.get();
}


void WebContentsDelegateQt::GoBackOrForward(int offset) {
  m_webContents->GetController().GoToOffset(offset);
  m_webContents->GetView()->Focus();
}

void WebContentsDelegateQt::Reload() {
  m_webContents->GetController().Reload(false);
  m_webContents->GetView()->Focus();
}

void WebContentsDelegateQt::Stop() {
  m_webContents->Stop();
  m_webContents->GetView()->Focus();
}

void WebContentsDelegateQt::LoadURL(const GURL& url)
{
  	LoadURLForFrame(url, std::string());
}

void WebContentsDelegateQt::LoadURLForFrame(const GURL& url, const std::string& frame_name) {
  content::NavigationController::LoadURLParams params(url);
  params.transition_type = content::PageTransitionFromInt(content::PAGE_TRANSITION_TYPED | content::PAGE_TRANSITION_FROM_ADDRESS_BAR);
  params.frame_name = frame_name;
  m_webContents->GetController().LoadURLWithParams(params);
  m_webContents->GetView()->Focus();
}

void WebContentsDelegateQt::Observe(int, const content::NotificationSource&, const content::NotificationDetails&)
{
	// IMPLEMENT THIS!!!!
}
