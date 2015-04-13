/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "chromium_overrides.h"

#include "gl_context_qt.h"
#include "qtwebenginecoreglobal.h"
#include "web_contents_view_qt.h"
#include "base/values.h"
#include "content/browser/renderer_host/pepper/pepper_truetype_font_list.h"
#include "content/browser/renderer_host/render_widget_host_view_base.h"
#include "content/common/font_list.h"

#include <QGuiApplication>
#include <QScreen>
#include <QWindow>

#if defined(USE_X11)
#include "ui/gfx/x/x11_types.h"
#endif

#if defined(USE_AURA) && !defined(USE_OZONE)
#include "ui/base/dragdrop/os_exchange_data.h"
#include "ui/base/dragdrop/os_exchange_data_provider_aura.h"
#include "ui/gfx/render_text.h"
#include "ui/gfx/platform_font.h"
#endif

namespace QtWebEngineCore {
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

} // namespace QtWebEngineCore

#if defined(USE_X11)
XDisplay* GetQtXDisplay()
{
    return static_cast<XDisplay*>(GLContextHelper::getXDisplay());
}
#endif

namespace content {
class WebContentsImpl;
class WebContentsView;
class WebContentsViewDelegate;
class RenderViewHostDelegateView;

WebContentsView* CreateWebContentsView(WebContentsImpl *web_contents,
    WebContentsViewDelegate *,
    RenderViewHostDelegateView **render_view_host_delegate_view)
{
    QtWebEngineCore::WebContentsViewQt* rv = new QtWebEngineCore::WebContentsViewQt(web_contents);
    *render_view_host_delegate_view = rv;
    return rv;
}

// static
void RenderWidgetHostViewBase::GetDefaultScreenInfo(blink::WebScreenInfo* results) {
    QWindow dummy;
    QtWebEngineCore::GetScreenInfoFromNativeWindow(&dummy, results);
}

} // namespace content

#if defined(USE_AURA) && !defined(USE_OZONE)
namespace content {

// content/common/font_list.h
scoped_ptr<base::ListValue> GetFontList_SlowBlocking()
{
    QT_NOT_USED
    return scoped_ptr<base::ListValue>(new base::ListValue);
}

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

RenderText* RenderText::CreateNativeInstance()
{
    QT_NOT_USED;
    return 0;
}

#if defined(OS_LINUX)
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
#endif

} // namespace gfx

#endif // defined(USE_AURA) && !defined(USE_OZONE)
