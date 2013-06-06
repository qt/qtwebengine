// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/shell.h"

#include <gdk/gdkkeysyms.h>

#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "base/utf_string_conversions.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/renderer_preferences.h"
#include "content/shell/shell_browser_context.h"
#include "content/shell/shell_content_browser_client.h"

#include "qquickwebcontentsview.h"
#include "qwebcontentsview.h"
#include "web_contents_view_qt.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QUrl>

namespace content {

extern QWebContentsView* gWidgetView;
extern QQuickWebContentsView* gQuickView;

void Shell::PlatformInitialize(const gfx::Size& default_window_size)
{
}

void Shell::PlatformCleanUp()
{
}

void Shell::PlatformEnableUIControl(UIControl control, bool is_enabled)
{
}

void Shell::PlatformSetAddressBarURL(const GURL& url)
{
    if (headless_)
        return;

    fprintf(stderr, "Set Address to: %s\n", url.spec().c_str());
    if (gWidgetView)
        gWidgetView->urlChanged(QUrl(QString::fromStdString(url.spec())));
    else if (gQuickView)
        gQuickView->urlChanged();
}


void Shell::PlatformSetIsLoading(bool loading)
{
    // FIXME: we might want to emit some loadStarted signal here or something...
}

void Shell::PlatformCreateWindow(int width, int height) {
    SizeTo(width, height);

    if (headless_)
        return;

    if (gWidgetView) {
        // The layout is used in PlatformSetContents.
        QVBoxLayout* layout = new QVBoxLayout;
        gWidgetView->setLayout(layout);
        window_ = reinterpret_cast<gfx::NativeWindow>(gWidgetView);
    } else if (gQuickView) {
        window_ = reinterpret_cast<gfx::NativeWindow>(gQuickView);
    }
}

void Shell::PlatformSetContents()
{
    if (headless_)
        return;

    content::RendererPreferences* rendererPrefs = web_contents_->GetMutableRendererPrefs();
    rendererPrefs->use_custom_colors = true;
    // Qt returns a flash time (the whole cycle) in ms, chromium expects just the interval in seconds
    rendererPrefs->caret_blink_interval = static_cast<double>(qApp->cursorFlashTime())/2000;
    web_contents_->GetRenderViewHost()->SyncRendererPrefs();

    if (gWidgetView) {
        WebContentsViewQt* content_view = static_cast<WebContentsViewQt*>(web_contents_->GetView());
        QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(gWidgetView->layout());
        if (layout)
            layout->addLayout(content_view->windowContainer()->widget());
    } else if (gQuickView) {
        WebContentsViewQt* content_view = static_cast<WebContentsViewQt*>(web_contents_->GetView());
        QQuickItem* windowContainer = content_view->windowContainer()->qQuickItem();
        windowContainer->setParentItem(gQuickView);
    }
}

void Shell::SizeTo(int width, int height)
{
    QT_NOT_YET_IMPLEMENTED
}

void Shell::PlatformResizeSubViews()
{
    SizeTo(content_width_, content_height_);
}

void Shell::Close()
{
    if (headless_) {
        delete this;
        return;
    }
}

void Shell::OnBackButtonClicked(GtkWidget* widget) { }

void Shell::OnForwardButtonClicked(GtkWidget* widget) { }

void Shell::OnReloadButtonClicked(GtkWidget* widget) { }

void Shell::OnStopButtonClicked(GtkWidget* widget)
{
    Stop();
}

void Shell::OnURLEntryActivate(GtkWidget* entry) { }

// Callback for when the main window is destroyed.
gboolean Shell::OnWindowDestroyed(GtkWidget* window)
{
    delete this;
    return FALSE;  // Don't stop this message.
}

gboolean Shell::OnCloseWindowKeyPressed(GtkAccelGroup* accel_group, GObject* acceleratable, guint keyval, GdkModifierType modifier)
{
    QT_NOT_YET_IMPLEMENTED
    return TRUE;
}

gboolean Shell::OnNewWindowKeyPressed(GtkAccelGroup* accel_group, GObject* acceleratable, guint keyval, GdkModifierType modifier)
{
    ShellBrowserContext* browser_context = ShellContentBrowserClient::Get()->browser_context();
    Shell::CreateNewWindow(browser_context, GURL(), NULL, MSG_ROUTING_NONE, gfx::Size());
    return TRUE;
}

gboolean Shell::OnHighlightURLView(GtkAccelGroup* accel_group, GObject* acceleratable, guint keyval, GdkModifierType modifier)
{
    return TRUE;
}

void Shell::PlatformSetTitle(const string16& title)
{
    if (headless_)
        return;

    if (gWidgetView) {
        std::string title_utf8 = UTF16ToUTF8(title);
        gWidgetView->titleChanged(QString::fromStdString(title_utf8));
    } else if (gQuickView)
        gQuickView->titleChanged();
}

}  // namespace content
