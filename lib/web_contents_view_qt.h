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

#ifndef WEB_CONTENTS_VIEW_QT_H
#define WEB_CONTENTS_VIEW_QT_H

#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host.h"
#include "content/port/browser/render_view_host_delegate_view.h"
#include "content/port/browser/web_contents_view_port.h"

#include "web_contents_adapter_client.h"
#include "render_widget_host_view_qt.h"
#include "web_contents_delegate_qt.h"
#include "web_engine_context.h"

class WebContentsViewQt
    : public content::WebContentsViewPort
    , public content::RenderViewHostDelegateView
{
public:
    static inline WebContentsViewQt* from(WebContentsView* view) { return static_cast<WebContentsViewQt*>(view); }

    WebContentsViewQt(content::WebContents* webContents)
        : m_webContents(webContents)
        , m_client(0)
        , m_factoryClient(0)
    { }

    void initialize(WebContentsAdapterClient* client);
    WebContentsAdapterClient* client() { return m_client; }

    virtual content::RenderWidgetHostView *CreateViewForWidget(content::RenderWidgetHost* render_widget_host);

    virtual void CreateView(const gfx::Size& initial_size, gfx::NativeView context);

    virtual content::RenderWidgetHostView* CreateViewForPopupWidget(content::RenderWidgetHost* render_widget_host) { return 0; }

    virtual void SetPageTitle(const string16& title);

    virtual void RenderViewCreated(content::RenderViewHost* host) { QT_NOT_YET_IMPLEMENTED }

    virtual void RenderViewSwappedIn(content::RenderViewHost* host) { QT_NOT_YET_IMPLEMENTED }

    virtual void SetOverscrollControllerEnabled(bool enabled) { QT_NOT_YET_IMPLEMENTED }

    virtual gfx::NativeView GetNativeView() const;

    virtual gfx::NativeView GetContentNativeView() const { QT_NOT_USED return 0; }

    virtual gfx::NativeWindow GetTopLevelNativeWindow() const { QT_NOT_USED return 0; }

    virtual void GetContainerBounds(gfx::Rect* out) const;

    virtual void OnTabCrashed(base::TerminationStatus status, int error_code) { QT_NOT_YET_IMPLEMENTED }

    virtual void SizeContents(const gfx::Size& size) { QT_NOT_YET_IMPLEMENTED }

    virtual void Focus();

    virtual void SetInitialFocus();

    virtual void StoreFocus() { QT_NOT_USED }

    virtual void RestoreFocus() { QT_NOT_USED }

    virtual content::DropData* GetDropData() const { QT_NOT_YET_IMPLEMENTED return 0; }

    virtual gfx::Rect GetViewBounds() const { QT_NOT_YET_IMPLEMENTED return gfx::Rect(); }

    virtual void ShowPopupMenu(const gfx::Rect& bounds, int item_height, double item_font_size, int selected_item,
                                const std::vector<content::MenuItem>& items, bool right_aligned, bool allow_multiple_selection) { QT_NOT_YET_IMPLEMENTED }

#if defined(OS_MACOSX)
    virtual void SetAllowOverlappingViews(bool overlapping) { QT_NOT_YET_IMPLEMENTED }
    virtual void CloseTabAfterEventTracking() { QT_NOT_YET_IMPLEMENTED }
    virtual bool GetAllowOverlappingViews() const { QT_NOT_YET_IMPLEMENTED; return false; }
    virtual bool IsEventTracking() const { QT_NOT_YET_IMPLEMENTED; return false; }
#endif // defined(OS_MACOSX)

private:
    content::WebContents *m_webContents;
    WebContentsAdapterClient *m_client;
    WebContentsAdapterClient *m_factoryClient;
};

#endif // WEB_CONTENTS_VIEW_QT_H
