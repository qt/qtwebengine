// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "type_conversion.h"
#include "ozone/gl_context_qt.h"
#include "qtwebenginecoreglobal_p.h"
#include "web_contents_view_qt.h"
#include "web_engine_library_info.h"

#include "base/values.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/browser/web_contents/web_contents_view.h"
#include "content/common/font_list.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "extensions/buildflags/buildflags.h"
#include "extensions/common/constants.h"
#include "gpu/vulkan/buildflags.h"
#include "ui/base/dragdrop/os_exchange_data.h"
#include "ui/base/dragdrop/os_exchange_data_provider_factory.h"

#include <QGuiApplication>
#include <QFontDatabase>
#include <QLibraryInfo>

#if !QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
#include "chrome/browser/extensions/api/webrtc_logging_private/webrtc_logging_private_api.h"
#endif

#if BUILDFLAG(ENABLE_VULKAN)
#include "compositor/vulkan_implementation_qt.h"

#include "gpu/vulkan/init/vulkan_factory.h"

#if defined(USE_OZONE)
#include "ui/ozone/public/ozone_platform.h"
#include "ui/ozone/public/surface_factory_ozone.h"
#endif // defined(USE_OZONE)
#endif // defined(ENABLE_VULKAN)

void *GetQtXDisplay()
{
    return GLContextHelper::getXDisplay();
}

namespace content {
class RenderViewHostDelegateView;

std::unique_ptr<WebContentsView> CreateWebContentsView(WebContentsImpl *web_contents,
    std::unique_ptr<WebContentsViewDelegate> delegate,
    RenderViewHostDelegateView **render_view_host_delegate_view)
{
    QtWebEngineCore::WebContentsViewQt* rv = new QtWebEngineCore::WebContentsViewQt(web_contents);
    *render_view_host_delegate_view = rv;
    return std::unique_ptr<WebContentsView>(rv);
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
base::Value::List GetFontList_SlowBlocking()
{
    base::Value::List font_list;

    for (auto family : QFontDatabase::families()){
        base::Value::List font_item;
        font_item.Append(family.toStdString());
        font_item.Append(family.toStdString());  // localized name.
        // TODO(yusukes): Support localized family names.
        font_list.Append(std::move(font_item));
    }
    return font_list;
}

} // namespace content
#endif // defined(USE_AURA) || defined(USE_OZONE)

#if BUILDFLAG(ENABLE_VULKAN)
namespace gpu {
std::unique_ptr<VulkanImplementation> CreateVulkanImplementation(bool use_swiftshader,
                                                                 bool allow_protected_memory)
{
#if QT_CONFIG(webengine_vulkan)
#if BUILDFLAG(IS_APPLE)
    // TODO: Investigate if we can support MoltenVK.
    NOTIMPLEMENTED();
    return nullptr;
#else
#if defined(USE_OZONE)
    return ui::OzonePlatform::GetInstance()->GetSurfaceFactoryOzone()->CreateVulkanImplementation(
            use_swiftshader, allow_protected_memory);
#endif

#if !BUILDFLAG(IS_WIN)
    // TODO(samans): Support Swiftshader on more platforms.
    // https://crbug.com/963988
    DCHECK(!use_swiftshader) << "Vulkan Swiftshader is not supported on this platform.";
#endif // !BUILDFLAG(IS_WIN)

    // Protected memory is supported only on Fuchsia, which uses Ozone, i.e.
    // VulkanImplementation is initialized above.
    DCHECK(!allow_protected_memory) << "Protected memory is not supported on this platform.";

    return std::make_unique<VulkanImplementationQt>();
#endif // BUILDFLAG(IS_APPLE)
#else
    NOTREACHED();
    return nullptr;
#endif // QT_CONFIG(webengine_vulkan)
}
} // namespace gpu
#endif // BUILDFLAG(ENABLE_VULKAN)

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
