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

#include "chromium_overrides.h"

#include "qtwebenginecoreglobal.h"
#include "base/values.h"
#include "content/browser/renderer_host/render_widget_host_view_base.h"

#include "content/browser/renderer_host/pepper/pepper_truetype_font_list.h"
#include "content/common/font_list.h"

#include <QGuiApplication>
#include <QScreen>
#include <QWindow>
#include <qpa/qplatformnativeinterface.h>

#if defined(OS_ANDROID)
#include "media/video/capture/fake_video_capture_device.h"
#endif

#if defined(USE_X11)
#include "base/message_loop/message_pump_x11.h"
#include <X11/Xlib.h>
#endif

#if defined(USE_AURA) && !defined(USE_OZONE)
#include "ui/base/dragdrop/os_exchange_data.h"
#include "ui/base/dragdrop/os_exchange_data_provider_aura.h"
#include "ui/gfx/render_text.h"
#include "ui/gfx/platform_font.h"
#endif

void GetScreenInfoFromNativeWindow(QWindow* window, blink::WebScreenInfo* results)
{
    QScreen* screen = window->screen();

    blink::WebScreenInfo r;
    r.deviceScaleFactor = screen->devicePixelRatio();
    r.depthPerComponent = 8;
    r.depth = screen->depth();
    r.isMonochrome = (r.depth == 1);

    QRect screenGeometry = screen->geometry();
    r.rect = blink::WebRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width(), screenGeometry.height());
    QRect available = screen->availableGeometry();
    r.availableRect = blink::WebRect(available.x(), available.y(), available.width(), available.height());
    *results = r;
}

namespace base {

#if defined(USE_X11)
Display* MessagePumpForUI::GetDefaultXDisplay() {
  static void *display = qApp->platformNativeInterface()->nativeResourceForScreen(QByteArrayLiteral("display"), qApp->primaryScreen());
  if (!display) {
    // XLib isn't available or has not been initialized, which is a decision we wish to
    // support, for example for the GPU process.
    static Display* xdisplay = XOpenDisplay(NULL);
    return xdisplay;
  }
  return static_cast<Display*>(display);
}
#endif

}

namespace content {
class WebContentsImpl;
class WebContentsViewPort;
class WebContentsViewDelegate;
class RenderViewHostDelegateView;

WebContentsViewPort* CreateWebContentsView(WebContentsImpl*,
                                           WebContentsViewDelegate*,
                                           RenderViewHostDelegateView**)
{
    return 0;
}

RenderWidgetHostView* RenderWidgetHostView::CreateViewForWidget(RenderWidgetHost*) {
    // WebContentsViewQt should take care of this directly.
    Q_UNREACHABLE();
    return NULL;
}

// static
void RenderWidgetHostViewPort::GetDefaultScreenInfo(blink::WebScreenInfo* results) {
    QWindow dummy;
    GetScreenInfoFromNativeWindow(&dummy, results);
}

#if defined(USE_AURA) && !defined(USE_OZONE)
// content/common/font_list.h
scoped_ptr<base::ListValue> GetFontList_SlowBlocking()
{
    QT_NOT_USED
    return scoped_ptr<base::ListValue>(new base::ListValue);
}
#endif
}

#if defined(USE_AURA) && !defined(USE_OZONE)
namespace content {

#if defined(ENABLE_PLUGINS)
// content/browser/renderer_host/pepper/pepper_truetype_font_list.h
void GetFontFamilies_SlowBlocking(std::vector<std::string> *)
{
    QT_NOT_USED
}

void GetFontsInFamily_SlowBlocking(const std::string &, std::vector<ppapi::proxy::SerializedTrueTypeFontDesc> *)
{
    QT_NOT_USED
}
#endif //defined(ENABLE_PLUGINS)

} // namespace content

namespace ui {

OSExchangeData::Provider* OSExchangeData::CreateProvider()
{
    QT_NOT_USED
    return 0;
}

}

namespace gfx {

// Stubs for these unused functions that are stripped in case
// of a release aura build but a debug build needs the symbols.

RenderText* RenderText::CreateInstance()
{
    QT_NOT_USED;
    return 0;
}

PlatformFont* PlatformFont::CreateDefault()
{
    QT_NOT_USED;
    return 0;
}

PlatformFont* PlatformFont::CreateFromNativeFont(NativeFont)
{
    QT_NOT_USED;
    return 0;
}

PlatformFont* PlatformFont::CreateFromNameAndSize(const std::string&, int)
{
    QT_NOT_USED;
    return 0;
}

} // namespace gfx

#endif // defined(USE_AURA) && !defined(USE_OZONE)

#if defined(OS_ANDROID)
namespace ui {
bool GrabViewSnapshot(gfx::NativeView /*view*/, std::vector<unsigned char>* /*png_representation*/, const gfx::Rect& /*snapshot_bounds*/)
{
    NOTIMPLEMENTED();
    return false;
}
}

namespace media {
const std::string FakeVideoCaptureDevice::Name::GetModel() const
{
    return "";
}
}
#endif
