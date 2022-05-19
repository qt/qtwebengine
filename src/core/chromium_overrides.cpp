// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "type_conversion.h"
#include "ozone/gl_context_qt.h"
#include "qtwebenginecoreglobal_p.h"
#include "web_contents_view_qt.h"
#include "web_engine_library_info.h"

#include "base/values.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/common/font_list.h"
#include "extensions/buildflags/buildflags.h"
#include "extensions/common/constants.h"
#include "ui/base/dragdrop/os_exchange_data.h"
#include "ui/base/dragdrop/os_exchange_data_provider_factory.h"

#include <QGuiApplication>
#include <QFontDatabase>
#include <QLibraryInfo>

#if !QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
#include "chrome/browser/extensions/api/webrtc_logging_private/webrtc_logging_private_api.h"
#endif

void *GetQtXDisplay()
{
    return GLContextHelper::getXDisplay();
}

namespace content {
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

#if defined(Q_OS_DARWIN)
#if defined(QT_MAC_FRAMEWORK_BUILD)
base::FilePath getSandboxPath()
{
    return WebEngineLibraryInfo::getPath(QT_FRAMEWORK_BUNDLE);
}
#else
base::FilePath getSandboxPath()
{
    const QString prefix = QLibraryInfo::location(QLibraryInfo::PrefixPath);
    return QtWebEngineCore::toFilePath(prefix);
}
#endif
#endif
} // namespace content

#if defined(USE_AURA) || defined(USE_OZONE)
namespace content {

// content/common/font_list.h
std::unique_ptr<base::ListValue> GetFontList_SlowBlocking()
{
    std::unique_ptr<base::ListValue> font_list(new base::ListValue);

    for (auto family : QFontDatabase::families()){
        std::unique_ptr<base::ListValue> font_item(new base::ListValue());
        font_item->Append(family.toStdString());
        font_item->Append(family.toStdString());  // localized name.
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

std::unique_ptr<ui::OSExchangeDataProvider> ui::OSExchangeDataProviderFactory::CreateProvider()
{
    return nullptr;
}

#if !BUILDFLAG(ENABLE_EXTENSIONS)
namespace extensions {
  const char kExtensionScheme[] = "chrome-extension";
}
#endif

#if !QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
namespace extensions {
ExtensionFunction::ResponseAction WebrtcLoggingPrivateSetMetaDataFunction::Run()
{
    return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateStartFunction::Run()
{
    return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateSetUploadOnRenderCloseFunction::Run()
{
    return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateStopFunction::Run()
{
    return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateStoreFunction::Run()
{
    return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateUploadStoredFunction::Run()
{
    return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateUploadFunction::Run()
{
    return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateDiscardFunction::Run()
{
    return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateStartRtpDumpFunction::Run()
{
    return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateStopRtpDumpFunction::Run()
{
    return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateStartAudioDebugRecordingsFunction::Run()
{
    return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateStopAudioDebugRecordingsFunction::Run()
{
    return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateStartEventLoggingFunction::Run()
{
    return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateGetLogsDirectoryFunction::Run()
{
    return RespondNow(NoArguments());
}
} // namespace extensions
#endif // !QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
