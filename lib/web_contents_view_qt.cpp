// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "web_contents_view_qt.h"

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <algorithm>

#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "build/build_config.h"
#include "content/browser/renderer_host/render_view_host_factory.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "render_widget_host_view_qt.h"
#include "content/browser/web_contents/interstitial_page_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/browser/web_contents/web_drag_dest_gtk.h"
#include "content/browser/web_contents/web_drag_source_gtk.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "raster_window.h"
#include "ui/base/gtk/gtk_expanded_container.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/point.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/size.h"
#include "webkit/glue/webdropdata.h"

using WebKit::WebDragOperation;
using WebKit::WebDragOperationsMask;

namespace content {
namespace {

// Called when the mouse leaves the widget. We notify our delegate.
gboolean OnLeaveNotify(GtkWidget* widget, GdkEventCrossing* event,
                       WebContentsImpl* web_contents) {
  if (web_contents->GetDelegate())
    web_contents->GetDelegate()->ContentsMouseEvent(
        web_contents, gfx::Point(event->x_root, event->y_root), false);
  return FALSE;
}

// Called when the mouse moves within the widget. We notify our delegate.
gboolean OnMouseMove(GtkWidget* widget, GdkEventMotion* event,
                     WebContentsImpl* web_contents) {
  if (web_contents->GetDelegate())
    web_contents->GetDelegate()->ContentsMouseEvent(
        web_contents, gfx::Point(event->x_root, event->y_root), true);
  return FALSE;
}

// See tab_contents_view_views.cc for discussion of mouse scroll zooming.
gboolean OnMouseScroll(GtkWidget* widget, GdkEventScroll* event,
                       WebContentsImpl* web_contents) {
  if ((event->state & gtk_accelerator_get_default_mod_mask()) !=
      GDK_CONTROL_MASK) {
    return FALSE;
  }

  WebContentsDelegate* delegate = web_contents->GetDelegate();
  if (!delegate)
    return FALSE;

  if (!(event->direction == GDK_SCROLL_DOWN ||
        event->direction == GDK_SCROLL_UP)) {
    return FALSE;
  }

  delegate->ContentsZoomChange(event->direction == GDK_SCROLL_UP);
  return TRUE;
}

}  // namespace

WebContentsViewPort* CreateWebContentsView(
    WebContentsImpl* web_contents,
    WebContentsViewDelegate* delegate,
    RenderViewHostDelegateView** render_view_host_delegate_view) {
  WebContentsViewGtk* rv = new WebContentsViewGtk(web_contents, delegate);
  *render_view_host_delegate_view = rv;
  return rv;
}

WebContentsViewGtk::WebContentsViewGtk(
    WebContentsImpl* web_contents,
    WebContentsViewDelegate* delegate)
    : web_contents_(web_contents),
      expanded_(gtk_expanded_container_new()),
      delegate_(delegate),
      m_windowContainer(new RasterWindowContainer) {
  gtk_widget_set_name(expanded_.get(), "chrome-web-contents-view");
  g_signal_connect(expanded_.get(), "size-allocate",
                   G_CALLBACK(OnSizeAllocateThunk), this);
  g_signal_connect(expanded_.get(), "child-size-request",
                   G_CALLBACK(OnChildSizeRequestThunk), this);

  gtk_widget_show(expanded_.get());
  drag_source_.reset(new WebDragSourceGtk(web_contents));

  if (delegate_)
    delegate_->Initialize(expanded_.get(), &focus_store_);
}

WebContentsViewGtk::~WebContentsViewGtk() {
  expanded_.Destroy();
}

gfx::NativeView WebContentsViewGtk::GetNativeView() const {
  if (delegate_)
    return delegate_->GetNativeView();

  return expanded_.get();
}

RasterWindow* WebContentsViewGtk::GetNativeViewQt() const {
  RenderWidgetHostView* rwhv = web_contents_->GetRenderWidgetHostView();
  if (!rwhv)
    return NULL;
  return rwhv->GetNativeViewQt();
}

gfx::NativeView WebContentsViewGtk::GetContentNativeView() const {
  RenderWidgetHostView* rwhv = web_contents_->GetRenderWidgetHostView();
  if (!rwhv)
    return NULL;
  return rwhv->GetNativeView();
}

gfx::NativeWindow WebContentsViewGtk::GetTopLevelNativeWindow() const {
  GtkWidget* window = gtk_widget_get_ancestor(GetNativeView(), GTK_TYPE_WINDOW);
  return window ? GTK_WINDOW(window) : NULL;
}

void WebContentsViewGtk::GetContainerBounds(gfx::Rect* out) const {
  // This is used for positioning the download shelf arrow animation,
  // as well as sizing some other widgets in Windows.  In GTK the size is
  // managed for us, so it appears to be only used for the download shelf
  // animation.
  int x = 0;
  int y = 0;
  GdkWindow* expanded_window = gtk_widget_get_window(expanded_.get());
  if (expanded_window)
    gdk_window_get_origin(expanded_window, &x, &y);

  GtkAllocation allocation;
  gtk_widget_get_allocation(expanded_.get(), &allocation);
  out->SetRect(x + allocation.x, y + allocation.y,
               requested_size_.width(), requested_size_.height());
}

void WebContentsViewGtk::OnTabCrashed(base::TerminationStatus status,
                                      int error_code) {
}

void WebContentsViewGtk::Focus() {
  if (web_contents_->ShowingInterstitialPage()) {
    web_contents_->GetInterstitialPage()->Focus();
  } else if (delegate_) {
    delegate_->Focus();
  }
}

void WebContentsViewGtk::SetInitialFocus() {
  if (web_contents_->FocusLocationBarByDefault())
    web_contents_->SetFocusToLocationBar(false);
  else
    Focus();
}

void WebContentsViewGtk::StoreFocus() {
  focus_store_.Store(GetNativeView());
}

void WebContentsViewGtk::RestoreFocus() {
  if (focus_store_.widget())
    gtk_widget_grab_focus(focus_store_.widget());
  else
    SetInitialFocus();
}

WebDropData* WebContentsViewGtk::GetDropData() const {
  return drag_dest_->current_drop_data();
}

gfx::Rect WebContentsViewGtk::GetViewBounds() const {
  gfx::Rect rect;
  GdkWindow* window = gtk_widget_get_window(GetNativeView());
  if (!window) {
    rect.SetRect(0, 0, requested_size_.width(), requested_size_.height());
    return rect;
  }
  int x = 0, y = 0, w, h;
  gdk_window_get_geometry(window, &x, &y, &w, &h, NULL);
  rect.SetRect(x, y, w, h);
  return rect;
}

void WebContentsViewGtk::CreateView(
    const gfx::Size& initial_size, gfx::NativeView context) {
  requested_size_ = initial_size;
}

RenderWidgetHostView* WebContentsViewGtk::CreateViewForWidget(
    RenderWidgetHost* render_widget_host) {
  if (render_widget_host->GetView()) {
    // During testing, the view will already be set up in most cases to the
    // test view, so we don't want to clobber it with a real one. To verify that
    // this actually is happening (and somebody isn't accidentally creating the
    // view twice), we check for the RVH Factory, which will be set when we're
    // making special ones (which go along with the special views).
    DCHECK(RenderViewHostFactory::has_factory());
    return render_widget_host->GetView();
  }

  RenderWidgetHostView* view =
      RenderWidgetHostView::CreateViewForWidget(render_widget_host);
  view->InitAsChild(reinterpret_cast<gfx::NativeView>(m_windowContainer));
  gfx::NativeView content_view = view->GetNativeView();
  g_signal_connect(content_view, "focus", G_CALLBACK(OnFocusThunk), this);
  g_signal_connect(content_view, "leave-notify-event",
                   G_CALLBACK(OnLeaveNotify), web_contents_);
  g_signal_connect(content_view, "motion-notify-event",
                   G_CALLBACK(OnMouseMove), web_contents_);
  g_signal_connect(content_view, "scroll-event",
                   G_CALLBACK(OnMouseScroll), web_contents_);
  gtk_widget_add_events(content_view, GDK_LEAVE_NOTIFY_MASK |
                        GDK_POINTER_MOTION_MASK);
  InsertIntoContentArea(content_view);

  // We don't want to change any state in this class for swapped out RVHs
  // because they will not be visible at this time.
  if (render_widget_host->IsRenderView()) {
    RenderViewHost* rvh = RenderViewHost::From(render_widget_host);
    if (!static_cast<RenderViewHostImpl*>(rvh)->is_swapped_out())
      UpdateDragDest(rvh);
  }

  return view;
}

RenderWidgetHostView* WebContentsViewGtk::CreateViewForPopupWidget(
    RenderWidgetHost* render_widget_host) {
  return RenderWidgetHostViewPort::CreateViewForWidget(render_widget_host);
}

void WebContentsViewGtk::SetPageTitle(const string16& title) {
  // Set the window name to include the page title so it's easier to spot
  // when debugging (e.g. via xwininfo -tree).
  gfx::NativeView content_view = GetContentNativeView();
  if (content_view) {
    GdkWindow* content_window = gtk_widget_get_window(content_view);
    if (content_window) {
      gdk_window_set_title(content_window, UTF16ToUTF8(title).c_str());
    }
  }
}

void WebContentsViewGtk::SizeContents(const gfx::Size& size) {
  // We don't need to manually set the size of of widgets in GTK+, but we do
  // need to pass the sizing information on to the RWHV which will pass the
  // sizing information on to the renderer.
  requested_size_ = size;
  RenderWidgetHostView* rwhv = web_contents_->GetRenderWidgetHostView();
  if (rwhv)
    rwhv->SetSize(size);
}

void WebContentsViewGtk::RenderViewCreated(RenderViewHost* host) {
}

void WebContentsViewGtk::RenderViewSwappedIn(RenderViewHost* host) {
  UpdateDragDest(host);
}

void WebContentsViewGtk::SetOverscrollControllerEnabled(bool enabled) {
}

WebContents* WebContentsViewGtk::web_contents() {
  return web_contents_;
}

RasterWindowContainer* WebContentsViewGtk::windowContainer()
{
  return m_windowContainer;
}

void WebContentsViewGtk::UpdateDragCursor(WebDragOperation operation) {
  drag_dest_->UpdateDragStatus(operation);
}

void WebContentsViewGtk::GotFocus() {
  // This is only used in the views FocusManager stuff but it bleeds through
  // all subclasses. http://crbug.com/21875
}

// This is called when the renderer asks us to take focus back (i.e., it has
// iterated past the last focusable element on the page).
void WebContentsViewGtk::TakeFocus(bool reverse) {
  if (!web_contents_->GetDelegate())
    return;
  if (!web_contents_->GetDelegate()->TakeFocus(web_contents_, reverse) &&
      GetTopLevelNativeWindow()) {
    gtk_widget_child_focus(GTK_WIDGET(GetTopLevelNativeWindow()),
        reverse ? GTK_DIR_TAB_BACKWARD : GTK_DIR_TAB_FORWARD);
  }
}

void WebContentsViewGtk::InsertIntoContentArea(GtkWidget* widget) {
  gtk_container_add(GTK_CONTAINER(expanded_.get()), widget);
}

void WebContentsViewGtk::UpdateDragDest(RenderViewHost* host) {
  gfx::NativeView content_view = host->GetView()->GetNativeView();

  // If the host is already used by the drag_dest_, there's no point in deleting
  // the old one to create an identical copy.
  if (drag_dest_.get() && drag_dest_->widget() == content_view)
    return;

  // Clear the currently connected drag drop signals by deleting the old
  // drag_dest_ before creating the new one.
  drag_dest_.reset();
  // Create the new drag_dest_.
  drag_dest_.reset(new WebDragDestGtk(web_contents_, content_view));

  if (delegate_)
    drag_dest_->set_delegate(delegate_->GetDragDestDelegate());
}

// Called when the content view gtk widget is tabbed to, or after the call to
// gtk_widget_child_focus() in TakeFocus(). We return true
// and grab focus if we don't have it. The call to
// FocusThroughTabTraversal(bool) forwards the "move focus forward" effect to
// webkit.
gboolean WebContentsViewGtk::OnFocus(GtkWidget* widget,
                                     GtkDirectionType focus) {
  // Give our view wrapper first chance at this event.
  if (delegate_) {
    gboolean return_value = FALSE;
    if (delegate_->OnNativeViewFocusEvent(widget, focus, &return_value))
      return return_value;
  }

  // If we already have focus, let the next widget have a shot at it. We will
  // reach this situation after the call to gtk_widget_child_focus() in
  // TakeFocus().
  if (gtk_widget_is_focus(widget))
    return FALSE;

  gtk_widget_grab_focus(widget);
  bool reverse = focus == GTK_DIR_TAB_BACKWARD;
  web_contents_->FocusThroughTabTraversal(reverse);
  return TRUE;
}

void WebContentsViewGtk::ShowContextMenu(
    const ContextMenuParams& params,
    ContextMenuSourceType type) {
  if (delegate_)
    delegate_->ShowContextMenu(params, type);
  else
    DLOG(ERROR) << "Cannot show context menus without a delegate.";
}

void WebContentsViewGtk::ShowPopupMenu(const gfx::Rect& bounds,
                                       int item_height,
                                       double item_font_size,
                                       int selected_item,
                                       const std::vector<WebMenuItem>& items,
                                       bool right_aligned,
                                       bool allow_multiple_selection) {
  // External popup menus are only used on Mac and Android.
  NOTIMPLEMENTED();
}

// Render view DnD -------------------------------------------------------------

void WebContentsViewGtk::StartDragging(const WebDropData& drop_data,
                                       WebDragOperationsMask ops,
                                       const gfx::ImageSkia& image,
                                       const gfx::Vector2d& image_offset,
                                       const DragEventSourceInfo& event_info) {
  DCHECK(GetContentNativeView());

  RenderWidgetHostView* view_gtk = static_cast<RenderWidgetHostView*>(
      web_contents_->GetRenderWidgetHostView());
  if (!view_gtk || !view_gtk->GetLastMouseDown() ||
      !drag_source_->StartDragging(drop_data, ops, view_gtk->GetLastMouseDown(),
                                   *image.bitmap(), image_offset)) {
    web_contents_->SystemDragEnded();
  }
}

// -----------------------------------------------------------------------------

void WebContentsViewGtk::OnChildSizeRequest(GtkWidget* widget,
                                            GtkWidget* child,
                                            GtkRequisition* requisition) {
  if (web_contents_->GetDelegate()) {
    requisition->height +=
        web_contents_->GetDelegate()->GetExtraRenderViewHeight();
  }
}

void WebContentsViewGtk::OnSizeAllocate(GtkWidget* widget,
                                        GtkAllocation* allocation) {
  int width = allocation->width;
  int height = allocation->height;
  // |delegate()| can be NULL here during browser teardown.
  if (web_contents_->GetDelegate())
    height += web_contents_->GetDelegate()->GetExtraRenderViewHeight();
  gfx::Size size(width, height);
  requested_size_ = size;

  // We manually tell our RWHV to resize the renderer content.  This avoids
  // spurious resizes from GTK+.
  RenderWidgetHostView* rwhv = web_contents_->GetRenderWidgetHostView();
  if (rwhv)
    rwhv->SetSize(size);
  if (web_contents_->GetInterstitialPage())
    web_contents_->GetInterstitialPage()->SetSize(size);
}

}  // namespace content
