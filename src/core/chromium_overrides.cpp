/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "ozone/gl_context_qt.h"
#include "qtwebenginecoreglobal_p.h"
#include "web_contents_view_qt.h"

#include "base/values.h"
#include "content/browser/accessibility/accessibility_tree_formatter_blink.h"
#include "content/browser/accessibility/accessibility_tree_formatter_browser.h"
#include "content/browser/renderer_host/render_widget_host_view_base.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/common/font_list.h"
#include "ui/base/dragdrop/os_exchange_data.h"
#include "ui/base/dragdrop/os_exchange_data_provider_factory.h"
#include "ui/events/devices/device_data_manager.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/snapshot/snapshot.h"
#include "ppapi/buildflags/buildflags.h"

#include <QGuiApplication>
#include <QScreen>
#include <QWindow>
#include <QFontDatabase>
#include <QStringList>
#include <QLibraryInfo>

#if defined(USE_AURA) && !defined(USE_OZONE)
#include "ui/base/dragdrop/os_exchange_data.h"
#include "ui/base/dragdrop/os_exchange_data_provider_aura.h"
#include "ui/gfx/render_text.h"
#include "ui/gfx/platform_font.h"
#endif

#if defined(USE_OPENSSL_CERTS)
#include "net/ssl/openssl_client_key_store.h"
#endif

void *GetQtXDisplay()
{
    return GLContextHelper::getXDisplay();
}

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

#if defined(Q_OS_MACOS)
std::string getQtPrefix()
{
    const QString prefix = QLibraryInfo::location(QLibraryInfo::PrefixPath);
    return prefix.toStdString();
}
#endif

} // namespace content

#if defined(USE_AURA) || defined(USE_OZONE)
namespace content {

// content/common/font_list.h
std::unique_ptr<base::ListValue> GetFontList_SlowBlocking()
{
    std::unique_ptr<base::ListValue> font_list(new base::ListValue);

    QFontDatabase database;
    for (auto family : database.families()){
        std::unique_ptr<base::ListValue> font_item(new base::ListValue());
        font_item->AppendString(family.toStdString());
        font_item->AppendString(family.toStdString());  // localized name.
        // TODO(yusukes): Support localized family names.
        font_list->Append(std::move(font_item));
    }
    return font_list;
}

} // namespace content

namespace aura {
class Window;
}

namespace wm {
class ActivationClient;

ActivationClient *GetActivationClient(aura::Window *)
{
    return nullptr;
}

} // namespace wm
#endif // defined(USE_AURA) || defined(USE_OZONE)

namespace content {
std::vector<AccessibilityTreeFormatter::TestPass> AccessibilityTreeFormatter::GetTestPasses()
{
    return {
        {"blink", &AccessibilityTreeFormatterBlink::CreateBlink, nullptr},
        {"native", &AccessibilityTreeFormatter::Create, nullptr},
    };
}
} // namespace content

#if defined(USE_AURA)
namespace ui {

bool GrabWindowSnapshot(gfx::NativeWindow window,
                        const gfx::Rect& snapshot_bounds,
                        gfx::Image* image)
{
    NOTIMPLEMENTED();
    return false;
}

bool GrabViewSnapshot(gfx::NativeView view,
                      const gfx::Rect& snapshot_bounds,
                      gfx::Image* image)
{
    NOTIMPLEMENTED();
    return false;
}

void GrabWindowSnapshotAndScaleAsync(gfx::NativeWindow window,
                                     const gfx::Rect& source_rect,
                                     const gfx::Size& target_size,
                                     GrabWindowSnapshotAsyncCallback callback)
{
    NOTIMPLEMENTED();
    std::move(callback).Run(gfx::Image());
}

void GrabWindowSnapshotAsync(gfx::NativeWindow window,
                             const gfx::Rect& source_rect,
                             GrabWindowSnapshotAsyncCallback callback)
{
    NOTIMPLEMENTED();
    std::move(callback).Run(gfx::Image());
}

void GrabViewSnapshotAsync(gfx::NativeView view,
                           const gfx::Rect& source_rect,
                           GrabWindowSnapshotAsyncCallback callback)
{
    NOTIMPLEMENTED();
    std::move(callback).Run(gfx::Image());
}

} // namespace ui
#endif // defined(USE_AURA)

std::unique_ptr<ui::OSExchangeData::Provider>
ui::OSExchangeDataProviderFactory::CreateProvider() {
    return nullptr;
}
