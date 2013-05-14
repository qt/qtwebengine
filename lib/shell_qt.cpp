// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/shell.h"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "base/utf_string_conversions.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/renderer_preferences.h"
#include "content/shell/shell_browser_context.h"
#include "content/shell/shell_content_browser_client.h"

#include <QWindow>
#include <QLineEdit>
#include <QWidget>
#include <QVBoxLayout>

namespace content {

void Shell::PlatformInitialize(const gfx::Size& default_window_size)
{
}

void Shell::PlatformCleanUp()
{
  // Nothing to clean up; GTK will clean up the widgets shortly after.
}

void Shell::PlatformEnableUIControl(UIControl control, bool is_enabled)
{
  // if (headless_)
  //   return;

  // GtkToolItem* item = NULL;
  // switch (control) {
  //   case BACK_BUTTON:
  //     item = back_button_;
  //     break;
  //   case FORWARD_BUTTON:
  //     item = forward_button_;
  //     break;
  //   case STOP_BUTTON:
  //     item = stop_button_;
  //     break;
  //   default:
  //     NOTREACHED() << "Unknown UI control";
  //     return;
  // }
  // gtk_widget_set_sensitive(GTK_WIDGET(item), is_enabled);
}

void Shell::PlatformSetAddressBarURL(const GURL& url)
{
  if (headless_)
    return;

  gtk_entry_set_text(GTK_ENTRY(url_edit_view_), url.spec().c_str());
}


void Shell::PlatformSetIsLoading(bool loading)
{
  if (headless_)
    return;

  if (loading)
    gtk_spinner_start(GTK_SPINNER(spinner_));
  else
    gtk_spinner_stop(GTK_SPINNER(spinner_));
}

void Shell::PlatformCreateWindow(int width, int height) {
  SizeTo(width, height);

  if (headless_)
    return;

  if (!m_window) {
    m_window = new QWidget;
    m_window->setGeometry(100,100, width, height);

    QVBoxLayout* layout = new QVBoxLayout;

    // Create a widget based address bar.
    QLineEdit* lineEdit =  new QLineEdit;
    layout->addWidget(lineEdit);

    m_window->setLayout(layout);
    m_window->show();
  }
}

void Shell::PlatformSetContents()
{
  if (headless_)
    return;

  WebContentsView* content_view = web_contents_->GetView();

  QWidget* container = QWidget::createWindowContainer(content_view->GetNativeViewQt());
  m_window->layout()->addWidget(container);
}

void Shell::SizeTo(int width, int height) {
  fprintf(stderr, "Shell::SizeTo\n");
  content_width_ = width;
  content_height_ = height;
  if (web_contents_) {
    gtk_widget_set_size_request(web_contents_->GetView()->GetNativeView(),
                                width, height);
  }
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

  gtk_widget_destroy(GTK_WIDGET(window_));
}

void Shell::OnBackButtonClicked(GtkWidget* widget)
{
  GoBackOrForward(-1);
}

void Shell::OnForwardButtonClicked(GtkWidget* widget)
{
  GoBackOrForward(1);
}

void Shell::OnReloadButtonClicked(GtkWidget* widget)
{
  Reload();
}

void Shell::OnStopButtonClicked(GtkWidget* widget)
{
  Stop();
}

void Shell::OnURLEntryActivate(GtkWidget* entry)
{
  const gchar* str = gtk_entry_get_text(GTK_ENTRY(entry));
  GURL url(str);
  if (!url.has_scheme())
    url = GURL(std::string("http://") + std::string(str));
  LoadURL(GURL(url));
}

// Callback for when the main window is destroyed.
gboolean Shell::OnWindowDestroyed(GtkWidget* window)
{
  delete this;
  return FALSE;  // Don't stop this message.
}

gboolean Shell::OnCloseWindowKeyPressed(GtkAccelGroup* accel_group, GObject* acceleratable, guint keyval, GdkModifierType modifier)
{
  gtk_widget_destroy(GTK_WIDGET(window_));
  return TRUE;
}

gboolean Shell::OnNewWindowKeyPressed(GtkAccelGroup* accel_group, GObject* acceleratable, guint keyval, GdkModifierType modifier)
{
  ShellBrowserContext* browser_context = ShellContentBrowserClient::Get()->browser_context();
  Shell::CreateNewWindow(browser_context,
                         GURL(),
                         NULL,
                         MSG_ROUTING_NONE,
                         gfx::Size());
  return TRUE;
}

gboolean Shell::OnHighlightURLView(GtkAccelGroup* accel_group, GObject* acceleratable, guint keyval, GdkModifierType modifier)
{
  gtk_widget_grab_focus(GTK_WIDGET(url_edit_view_));
  return TRUE;
}

void Shell::PlatformSetTitle(const string16& title)
{
  if (headless_)
    return;

  std::string title_utf8 = UTF16ToUTF8(title);
  gtk_window_set_title(GTK_WINDOW(window_), title_utf8.c_str());
}

}  // namespace content
