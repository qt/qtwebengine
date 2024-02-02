// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "ozone_platform_qt.h"

#if defined(USE_OZONE)
#include "base/no_destructor.h"
#include "base/task/thread_pool.h"
#include "media/gpu/buildflags.h"
#include "ui/base/buildflags.h"
#include "ui/base/ime/input_method.h"
#include "ui/display/types/native_display_delegate.h"
#include "ui/events/ozone/layout/keyboard_layout_engine_manager.h"
#include "ui/events/ozone/layout/stub/stub_keyboard_layout_engine.h"
#include "ui/gfx/linux/client_native_pixmap_dmabuf.h"
#include "ui/gfx/linux/client_native_pixmap_factory_dmabuf.h"
#include "ui/gfx/linux/gpu_memory_buffer_support_x11.h"
#include "ui/ozone/common/bitmap_cursor_factory.h"
#include "ui/ozone/common/stub_client_native_pixmap_factory.h"
#include "ui/ozone/common/stub_overlay_manager.h"
#include "ui/ozone/public/gpu_platform_support_host.h"
#include "ui/ozone/public/input_controller.h"
#include "ui/ozone/public/ozone_platform.h"
#include "ui/ozone/public/platform_screen.h"
#include "ui/ozone/public/system_input_injector.h"
#include "ui/ozone/platform/x11/gl_egl_utility_x11.h"
#include "ui/ozone/platform/wayland/gpu/wayland_gl_egl_utility.h"
#include "ui/platform_window/platform_window_delegate.h"
#include "ui/platform_window/platform_window_init_properties.h"

#include "surface_factory_qt.h"
#include "platform_window_qt.h"

#if BUILDFLAG(USE_XKBCOMMON)
#include "base/logging.h"
#include "ui/events/ozone/layout/xkb/xkb_evdev_codes.h"
#include "ui/events/ozone/layout/xkb/xkb_keyboard_layout_engine.h"

#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>
#include <filesystem>

extern void *GetQtXDisplay();
#endif // BUILDFLAG(USE_XKBCOMMON)

namespace ui {

namespace {

class OzonePlatformQt : public OzonePlatform {
public:
    OzonePlatformQt();
    ~OzonePlatformQt() override;

    ui::SurfaceFactoryOzone* GetSurfaceFactoryOzone() override;
    ui::CursorFactory* GetCursorFactory() override;
    GpuPlatformSupportHost* GetGpuPlatformSupportHost() override;
    std::unique_ptr<PlatformWindow> CreatePlatformWindow(PlatformWindowDelegate* delegate, PlatformWindowInitProperties properties) override;
    std::unique_ptr<display::NativeDisplayDelegate> CreateNativeDisplayDelegate() override;
    ui::InputController* GetInputController() override;
    std::unique_ptr<ui::SystemInputInjector> CreateSystemInputInjector() override;
    ui::OverlayManagerOzone* GetOverlayManager() override;
    std::unique_ptr<InputMethod> CreateInputMethod(ImeKeyEventDispatcher *ime_key_event_dispatcher, gfx::AcceleratedWidget widget) override;
    std::unique_ptr<ui::PlatformScreen> CreateScreen() override { return nullptr; }
    const PlatformProperties &GetPlatformProperties() override;
    PlatformGLEGLUtility *GetPlatformGLEGLUtility() override;

    const PlatformRuntimeProperties &GetPlatformRuntimeProperties() override
    {
        static OzonePlatform::PlatformRuntimeProperties properties;
#if BUILDFLAG(USE_VAAPI)
        if (has_initialized_gpu()) {
            if (GetQtXDisplay()) {
                // This property is set when the GetPlatformRuntimeProperties is
                // called on the gpu process side.
                properties.supports_native_pixmaps = ui::GpuMemoryBufferSupportX11::GetInstance()->has_gbm_device();
            } else {
                properties.supports_native_pixmaps = true; // buffer_manager_->GetGbmDevice() != nullptr
            }
        }
#endif
        return properties;
    }
    bool IsNativePixmapConfigSupported(gfx::BufferFormat format, gfx::BufferUsage usage) const override;

private:
    bool InitializeUI(const ui::OzonePlatform::InitParams &) override;
    void InitializeGPU(const ui::OzonePlatform::InitParams &) override;

    void InitScreen(ui::PlatformScreen *) override {}

    std::unique_ptr<QtWebEngineCore::SurfaceFactoryQt> surface_factory_ozone_;
    std::unique_ptr<CursorFactory> cursor_factory_;

    std::unique_ptr<GpuPlatformSupportHost> gpu_platform_support_host_;
    std::unique_ptr<InputController> input_controller_;
    std::unique_ptr<OverlayManagerOzone> overlay_manager_;

#if BUILDFLAG(USE_XKBCOMMON)
    XkbEvdevCodes m_xkbEvdevCodeConverter;
#endif
    std::unique_ptr<KeyboardLayoutEngine> m_keyboardLayoutEngine;
    std::unique_ptr<PlatformGLEGLUtility> gl_egl_utility_;
};


OzonePlatformQt::OzonePlatformQt() {}

OzonePlatformQt::~OzonePlatformQt() {}

const ui::OzonePlatform::PlatformProperties &OzonePlatformQt::GetPlatformProperties()
{
    static base::NoDestructor<ui::OzonePlatform::PlatformProperties> properties;
    static bool initialized = false;
    if (!initialized) {
        properties->fetch_buffer_formats_for_gmb_on_gpu = true;
#if BUILDFLAG(USE_VAAPI)
        properties->supports_vaapi = true;
#endif
        initialized = true;
    }

    return *properties;
}

ui::SurfaceFactoryOzone* OzonePlatformQt::GetSurfaceFactoryOzone()
{
    return surface_factory_ozone_.get();
}

ui::CursorFactory* OzonePlatformQt::GetCursorFactory()
{
    return cursor_factory_.get();
}

GpuPlatformSupportHost* OzonePlatformQt::GetGpuPlatformSupportHost()
{
    return gpu_platform_support_host_.get();
}

std::unique_ptr<PlatformWindow> OzonePlatformQt::CreatePlatformWindow(PlatformWindowDelegate* delegate, PlatformWindowInitProperties properties)
{
    return base::WrapUnique(new PlatformWindowQt(delegate, properties.bounds));
}

ui::InputController* OzonePlatformQt::GetInputController()
{
    return input_controller_.get();
}

std::unique_ptr<ui::SystemInputInjector> OzonePlatformQt::CreateSystemInputInjector()
{
    return nullptr;  // no input injection support.
}

ui::OverlayManagerOzone* OzonePlatformQt::GetOverlayManager()
{
    return overlay_manager_.get();
}

std::unique_ptr<display::NativeDisplayDelegate> OzonePlatformQt::CreateNativeDisplayDelegate()
{
    NOTREACHED();
    return nullptr;
}

#if BUILDFLAG(USE_XKBCOMMON)
static std::string getCurrentKeyboardLayout()
{
    Display *dpy = static_cast<Display *>(GetQtXDisplay());
    if (dpy == nullptr)
        return std::string();

    int d;
    if (!XkbQueryExtension(dpy, &d, &d, &d, &d, &d)) {
        // no Xkb extension
        return std::string();
    }

    XkbStateRec state;
    if (XkbGetState(dpy, XkbUseCoreKbd, &state) != 0)
        return std::string();

    XkbRF_VarDefsRec vdr {}; // zero initialize it
    struct Cleanup {
        XkbRF_VarDefsRec &vdr;
        Cleanup(XkbRF_VarDefsRec &vdr) : vdr(vdr) { }
        ~Cleanup() {
            free (vdr.model);
            free (vdr.layout);
            free (vdr.variant);
            free (vdr.options);
        }
    } cleanup(vdr);
    if (XkbRF_GetNamesProp(dpy, nullptr, &vdr) == 0)
        return std::string();

    if (!vdr.layout)
        return std::string();

    if (!vdr.variant)
        return std::string(vdr.layout);

    std::string layoutWithVariant = vdr.layout;
    layoutWithVariant = layoutWithVariant.append("-");
    layoutWithVariant = layoutWithVariant.append(vdr.variant);

    return layoutWithVariant;
}
#endif // BUILDFLAG(USE_XKBCOMMON)

bool OzonePlatformQt::InitializeUI(const ui::OzonePlatform::InitParams &)
{
#if BUILDFLAG(USE_XKBCOMMON)
    std::string xkb_path("/usr/share/X11/xkb");
    std::string layout = getCurrentKeyboardLayout();
    if (layout.empty() || !std::filesystem::exists(xkb_path) || std::filesystem::is_empty(xkb_path)) {
        LOG(WARNING) << "Failed to load keymap file, falling back to StubKeyboardLayoutEngine";
        m_keyboardLayoutEngine = std::make_unique<StubKeyboardLayoutEngine>();
    } else {
        m_keyboardLayoutEngine = std::make_unique<XkbKeyboardLayoutEngine>(m_xkbEvdevCodeConverter);
        m_keyboardLayoutEngine->SetCurrentLayoutByName(layout);
    }
#else
    m_keyboardLayoutEngine = std::make_unique<StubKeyboardLayoutEngine>();
#endif // BUILDFLAG(USE_XKBCOMMON)

    KeyboardLayoutEngineManager::SetKeyboardLayoutEngine(m_keyboardLayoutEngine.get());

    overlay_manager_.reset(new StubOverlayManager());
    input_controller_ = CreateStubInputController();
    cursor_factory_.reset(new BitmapCursorFactory());
    gpu_platform_support_host_.reset(ui::CreateStubGpuPlatformSupportHost());
    return true;
}

void OzonePlatformQt::InitializeGPU(const ui::OzonePlatform::InitParams &params)
{
    surface_factory_ozone_.reset(new QtWebEngineCore::SurfaceFactoryQt());

#if BUILDFLAG(OZONE_PLATFORM_X11) && BUILDFLAG(USE_VAAPI_X11)
    if (params.enable_native_gpu_memory_buffers) {
        base::ThreadPool::PostTask(FROM_HERE,
                                   base::BindOnce([]()
            {
                ui::GpuMemoryBufferSupportX11::GetInstance();
            }));
    }
#endif
}

std::unique_ptr<InputMethod> OzonePlatformQt::CreateInputMethod(ImeKeyEventDispatcher *, gfx::AcceleratedWidget)
{
    NOTREACHED();
    return nullptr;
}

bool OzonePlatformQt::IsNativePixmapConfigSupported(gfx::BufferFormat format, gfx::BufferUsage usage) const
{
#if BUILDFLAG(USE_VAAPI)
    return gfx::ClientNativePixmapDmaBuf::IsConfigurationSupported(format, usage);
#else
    return false;
#endif
}

PlatformGLEGLUtility *OzonePlatformQt::GetPlatformGLEGLUtility()
{
#if BUILDFLAG(USE_VAAPI)
    if (!gl_egl_utility_) {
#if BUILDFLAG(OZONE_PLATFORM_X11)
        if (GetQtXDisplay())
            gl_egl_utility_ = std::make_unique<GLEGLUtilityX11>();
        else
#endif
            gl_egl_utility_ = std::make_unique<WaylandGLEGLUtility>();
    }
#endif
    return gl_egl_utility_.get();
}


} // namespace

OzonePlatform* CreateOzonePlatformQt() { return new OzonePlatformQt; }

gfx::ClientNativePixmapFactory *CreateClientNativePixmapFactoryQt()
{
#if BUILDFLAG(USE_VAAPI)
    return gfx::CreateClientNativePixmapFactoryDmabuf();
#else
    return CreateStubClientNativePixmapFactory();
#endif
}

}  // namespace ui

#endif

