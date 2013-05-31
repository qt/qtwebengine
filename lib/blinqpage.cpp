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

#include "blinqpage.h"

// Needed to get access to content::GetContentClient()
#define CONTENT_IMPLEMENTATION

#include "content/public/browser/web_contents.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/web_contents_view.h"
#include "net/url_request/url_request_context_getter.h"
#include "base/event_types.h"
#include "ui/gfx/insets.h"
#include "ui/gfx/screen.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/rect_conversions.h"
#include "ui/surface/transport_dib.h"
#include "content/common/view_messages.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/browser/renderer_host/render_view_host_factory.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/backing_store.h"
#include "skia/ext/platform_canvas.h"

#include "backing_store_qt.h"
#include "browser_context_qt.h"
#include "content_browser_client_qt.h"
#include "raster_window.h"
#include "render_widget_host_view_qt.h"
#include "resource_context_qt.h"

#include <QByteArray>
#include <QWindow>
#include <QCoreApplication>
#include <QGuiApplication>
#include <qpa/qplatformwindow.h>
#include <QLabel>
#include <QPainter>
#include <qpa/qplatformnativeinterface.h>

namespace {

class RenderViewHost : public content::RenderViewHostImpl
{
public:
    RenderViewHost(
        content::SiteInstance* instance,
        content::RenderViewHostDelegate* delegate,
        content::RenderWidgetHostDelegate* widget_delegate,
        int routing_id,
        bool swapped_out,
        content::SessionStorageNamespace* session_storage_namespace)
        : content::RenderViewHostImpl(instance, delegate, widget_delegate, routing_id, swapped_out, session_storage_namespace)
    {
        SetView(new content::RenderWidgetHostViewQt(this));
    }
};

}

class BlinqPagePrivate
{
public:
    scoped_ptr<content::BrowserContext> context;
    scoped_ptr<content::WebContents> contents;
};

BlinqPage::BlinqPage()
{
    d.reset(new BlinqPagePrivate);

    d->context.reset(static_cast<ContentBrowserClientQt*>(content::GetContentClient()->browser())->browser_context());
    content::WebContents::CreateParams p(d->context.get());
//    d->contents.reset(content::WebContents::Create(p));

//    d->contents->GetController().LoadURL(GURL(std::string("http://qt-project.org/")),
//                                         content::Referrer(),
//                                         content::PAGE_TRANSITION_TYPED,
//                                         std::string());
}

BlinqPage::~BlinqPage()
{
}

QWindow *BlinqPage::window()
{
  return 0;
}

