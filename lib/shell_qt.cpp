// Copyright (c) 2012 The Chromium Authors. All rights reserved.
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
#include "raster_window.h"
#include "signal_connector.h"

#include "web_contents_view_qt.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QWidget>
#include <QWindow>

namespace content {

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

  QLineEdit* addressLine = reinterpret_cast<QWidget*>(window_)->findChild<QLineEdit*>("AddressLineEdit");
  addressLine->setText(QString::fromStdString(url.spec()));
}


void Shell::PlatformSetIsLoading(bool loading)
{
    // FIXME: we might want to emit some loadStarted signal here or something...
}

void Shell::PlatformCreateWindow(int width, int height) {
  SizeTo(width, height);

  if (headless_)
    return;

  if (!window_) {

    // Use oxygen as a fallback.
    if (QIcon::themeName().isEmpty())
      QIcon::setThemeName("oxygen");

    QWidget* window = new QWidget;
    window_ = reinterpret_cast<gfx::NativeWindow>(window);

    window->setGeometry(100,100, width, height);

    QVBoxLayout* layout = new QVBoxLayout;

    // Create a widget based address bar.
    QHBoxLayout* addressBar = new QHBoxLayout;

    int buttonWidth = 26;
    QToolButton* backButton = new QToolButton;
    backButton->setIcon(QIcon::fromTheme("go-previous"));
    backButton->setObjectName("BackButton");
    addressBar->addWidget(backButton);

    QToolButton* forwardButton = new QToolButton;
    forwardButton->setIcon(QIcon::fromTheme("go-next"));
    forwardButton->setObjectName("ForwardButton");
    addressBar->addWidget(forwardButton);

    QToolButton* reloadButton = new QToolButton;
    reloadButton->setIcon(QIcon::fromTheme("view-refresh"));
    reloadButton->setObjectName("ReloadButton");
    addressBar->addWidget(reloadButton);

    QLineEdit* lineEdit =  new QLineEdit;
    lineEdit->setObjectName("AddressLineEdit");
    addressBar->addWidget(lineEdit);

    layout->addLayout(addressBar);


    window->setLayout(layout);
    window->show();

    // SignalConnector will act as a proxy for the QObject signals received from
    // m_window. m_window will take ownership of the SignalConnector.
    // The SignalConnector will search the children list of m_window
    // for back/forward/reload buttons and for the address line edit.
    // Therefore the layout must be set and completed before the SignalConnector
    // is created.
    SignalConnector* signalConnector = new SignalConnector(this, window);
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

    WebContentsViewQt* content_view = static_cast<WebContentsViewQt*>(web_contents_->GetView());
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(reinterpret_cast<QWidget*>(window_)->layout());
    if (layout)
        layout->addLayout(content_view->windowContainer());
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

  std::string title_utf8 = UTF16ToUTF8(title);
}

}  // namespace content
