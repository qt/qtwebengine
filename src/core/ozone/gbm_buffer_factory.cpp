// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "gbm_buffer_factory.h"

#include "ozone/egl_helper.h"
#include "ozone/glx_helper.h"
#include "ozone/ozone_util_qt.h"

#include "ui/gfx/buffer_format_util.h"
#include "ui/gfx/linux/drm_util_linux.h"
#include "ui/gfx/linux/gbm_buffer.h"
#include "ui/gfx/linux/gbm_defines.h"
#include "ui/gfx/linux/gbm_device.h"
#include "ui/gfx/linux/gbm_util.h"
#include "ui/gfx/linux/gbm_wrapper.h"
#include "ui/ozone/platform/wayland/common/drm_render_node_handle.h"

#include <QtCore/qdebug.h>
#include <QtGui/qtguiglobal.h>

#include <drm_fourcc.h>
#include <string>
#include <sstream>
#include <xf86drm.h>

QT_BEGIN_NAMESPACE

static std::string gbmFlagsToString(uint32_t flags)
{
    if (flags == 0)
        return "NONE (0x0)";

    std::stringstream ss;
    auto append = [&](uint32_t f, const char *name) {
        if (flags & f) {
            if (ss.tellp() > 0)
                ss << " | ";
            ss << name;
        }
    };

    // Based on ui::BufferUsageToGbmFlags() in //ui/gfx/linux/gbm_util.cc
    append(GBM_BO_USE_CAMERA_WRITE, "GBM_BO_USE_CAMERA_WRITE");
    append(GBM_BO_USE_FRONT_RENDERING, "GBM_BO_USE_FRONT_RENDERING");
    append(GBM_BO_USE_HW_VIDEO_DECODER, "GBM_BO_USE_HW_VIDEO_DECODER");
    append(GBM_BO_USE_HW_VIDEO_ENCODER, "GBM_BO_USE_HW_VIDEO_ENCODER");
    append(GBM_BO_USE_LINEAR, "GBM_BO_USE_LINEAR");
    append(GBM_BO_USE_PROTECTED, "GBM_BO_USE_PROTECTED");
    append(GBM_BO_USE_RENDERING, "GBM_BO_USE_RENDERING");
    append(GBM_BO_USE_SCANOUT, "GBM_BO_USE_SCANOUT");
    append(GBM_BO_USE_SW_READ_OFTEN, "GBM_BO_USE_SW_READ_OFTEN");
    append(GBM_BO_USE_TEXTURING, "GBM_BO_USE_TEXTURING");

    ss << " (0x" << std::hex << flags << ")";
    return ss.str();
}

static base::ScopedFD openDrmNodePath(const std::string &path)
{
    ui::DrmRenderNodeHandle nodeHandle;
    if (!nodeHandle.Initialize(base::FilePath(path))) {
        qWarning("GBM: Failed to initialize DRM render node handle: %s\n", path.data());
        return base::ScopedFD();
    }

    return nodeHandle.PassFD();
}

GbmBufferFactory *GbmBufferFactory::instance()
{
#if QT_CONFIG(xcb_glx_plugin)
    if (OzoneUtilQt::usingGLX())
        return GLXHelper::instance()->gbmFactory();
#endif

#if QT_CONFIG(egl)
    if (OzoneUtilQt::usingEGL())
        return EGLHelper::instance()->gbmFactory();
#endif

    return nullptr;
}

// Path-based (EGL)
GbmBufferFactory::GbmBufferFactory(const std::string &drmNodePath)
    : GbmBufferFactory(openDrmNodePath(drmNodePath))
{
}

// FD-based (GLX)
GbmBufferFactory::GbmBufferFactory(base::ScopedFD drmNodeFD) : m_drmNodeFD(std::move(drmNodeFD))
{
    // It is safe to initialize the GBM device on the UI thread and allocate buffers on the GPU
    // thread because m_mutex serializes access and guarantees the device is fully initialized
    // before allocation begins.
    QMutexLocker locker(&m_mutex);

    if (!m_drmNodeFD.is_valid()) {
        qWarning("GBM: Obtained an invalid file descriptor.");
        return;
    }

    m_gbmDevice = ui::CreateGbmDevice(m_drmNodeFD.get());
    if (!m_gbmDevice) {
        qWarning("GBM: Failed to initialize GBM device.");
        return;
    }
}

GbmBufferFactory::~GbmBufferFactory() = default;

std::string GbmBufferFactory::drmRenderNodePath() const
{
    if (!m_drmNodeFD.is_valid())
        return { };

    char *nodeName = drmGetRenderDeviceNameFromFd(m_drmNodeFD.get());
    if (!nodeName)
        return { };

    std::string path(nodeName);
    free(nodeName);

    return path;
}

std::string GbmBufferFactory::drmDeviceString() const
{
    if (!m_drmNodeFD.is_valid())
        return { };

    drmVersionPtr version = drmGetVersion(m_drmNodeFD.get());
    if (!version)
        return { };

    std::string deviceString;
    deviceString.reserve(64);

    deviceString += std::string(version->name);
    deviceString += " v";
    deviceString += std::to_string(version->version_major);
    deviceString += ".";
    deviceString += std::to_string(version->version_minor);
    deviceString += ".";
    deviceString += std::to_string(version->version_patchlevel);
    deviceString += " (";
    deviceString += version->desc;
    deviceString += ")";

    drmFreeVersion(version);
    return deviceString;
}

bool GbmBufferFactory::canCreateNativePixmapForFormat(gfx::BufferFormat format) const
{
    QMutexLocker locker(&m_mutex);
    if (!m_gbmDevice)
        return false;

    const uint32_t fourccFormat = ui::GetFourCCFormatFromBufferFormat(format);
    return m_gbmDevice->CanCreateBufferForFormat(fourccFormat);
}

bool GbmBufferFactory::isSinglePlanar(uint32_t fourccFormat, uint64_t modifier) const
{
    QMutexLocker locker(&m_mutex);

    if (modifier == DRM_FORMAT_MOD_LINEAR || modifier == DRM_FORMAT_MOD_INVALID)
        return true;

    if (!m_gbmDevice)
        return false;

    const int planeCount = gbm_device_get_format_modifier_plane_count(
            m_gbmDevice->GetNativeDevice(), fourccFormat, modifier);
    return (planeCount == 1);
}

std::unique_ptr<ui::GbmBuffer>
GbmBufferFactory::createBufferWithModifiers(gfx::BufferFormat format, gfx::Size size,
                                            gfx::BufferUsage usage,
                                            const std::vector<uint64_t> &modifiers)
{
    QMutexLocker locker(&m_mutex);
    if (!m_gbmDevice)
        return nullptr;

    const uint32_t fourccFormat = ui::GetFourCCFormatFromBufferFormat(format);
    const uint32_t gbmFlags = ui::BufferUsageToGbmFlags(usage);

    // If no modifiers were passed, simply fall back to the LINEAR format modifier.
    // It is expected to work with any GPU driver, though it may not be optimal.
    const std::vector<uint64_t> linearFallback{ DRM_FORMAT_MOD_LINEAR };
    const std::vector<uint64_t> &supportedModifiers =
            modifiers.empty() ? linearFallback : modifiers;

    std::unique_ptr<ui::GbmBuffer> buffer = m_gbmDevice->CreateBufferWithModifiers(
            fourccFormat, size, gbmFlags, supportedModifiers);

    if (!buffer) {
        QStringList modStrings;
        modStrings.reserve(supportedModifiers.size());
        for (uint64_t mod : supportedModifiers)
            modStrings << QLatin1StringView(OzoneUtilQt::drmFormatModifierToString(mod));

        std::string nodePath = drmRenderNodePath();
        qWarning().noquote() << "GBM: Buffer creation failed with the following parameters:\n"
                             << "  Device:   " << (!nodePath.empty() ? nodePath.c_str() : "unknown")
                             << "\n"
                             << "  Format:   " << ui::DrmFormatToString(fourccFormat) << "\n"
                             << "  Size:     " << size.ToString().c_str() << "\n"
                             << "  Flags:    " << gbmFlagsToString(gbmFlags) << "\n"
                             << "  Modifiers:" << modStrings.join(" | ");
    }

    return buffer;
}

std::unique_ptr<ui::GbmBuffer>
GbmBufferFactory::createBufferFromHandle(gfx::BufferFormat format, gfx::Size size,
                                         gfx::NativePixmapHandle handle)
{
    QMutexLocker locker(&m_mutex);
    if (!m_gbmDevice)
        return nullptr;

    const uint32_t fourccFormat = ui::GetFourCCFormatFromBufferFormat(format);
    const uint64_t modifier = handle.modifier;
    const size_t numPlanes = handle.planes.size();
    std::unique_ptr<ui::GbmBuffer> buffer =
            m_gbmDevice->CreateBufferFromHandle(fourccFormat, size, std::move(handle));

    if (!buffer) {
        std::string nodePath = drmRenderNodePath();
        qWarning().noquote()
                << "GBM: Buffer creation from handle failed with the following parameters:\n"
                << "  Device:  " << (!nodePath.empty() ? nodePath.c_str() : "unknown") << "\n"
                << "  Format:  " << ui::DrmFormatToString(fourccFormat) << "\n"
                << "  Size:    " << size.ToString().c_str() << "\n"
                << "  Planes:  " << numPlanes << "\n"
                << "  Modifier:"
                << QLatin1StringView(OzoneUtilQt::drmFormatModifierToString(modifier));
    }

    return buffer;
}

QT_END_NAMESPACE
