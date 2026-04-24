// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "gbm_buffer_factory.h"

#include "compositor/compositor.h"

#include "ui/gfx/linux/drm_util_linux.h"
#include "ui/gfx/linux/gbm_buffer.h"
#include "ui/gfx/linux/gbm_device.h"
#include "ui/gfx/linux/gbm_util.h"
#include "ui/gfx/linux/gbm_wrapper.h"
#include "ui/ozone/platform/wayland/common/drm_render_node_handle.h"

#include <drm_fourcc.h>
#include <xf86drm.h>

QT_BEGIN_NAMESPACE

static base::ScopedFD openDrmNodePath(const std::string &path)
{
    ui::DrmRenderNodeHandle nodeHandle;
    if (!nodeHandle.Initialize(base::FilePath(path))) {
        qWarning("GBM: Failed to initialize DRM render node handle: %s\n", path.data());
        return base::ScopedFD();
    }

    return nodeHandle.PassFD();
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

    if (Q_UNLIKELY(QtWebEngineCore::lcWebEngineCompositor().isDebugEnabled())) {
        if (drmVersionPtr drmVersion = drmGetVersion(m_drmNodeFD.get())) {
            qCDebug(QtWebEngineCore::lcWebEngineCompositor,
                    "GBM: DRM device found: %s v%d.%d.%d (%s)", drmVersion->name,
                    drmVersion->version_major, drmVersion->version_minor,
                    drmVersion->version_patchlevel, drmVersion->desc);
            drmFreeVersion(drmVersion);
        } else {
            qCDebug(QtWebEngineCore::lcWebEngineCompositor, "GBM: Failed to identify DRM device.");
        }
    }

    m_gbmDevice = ui::CreateGbmDevice(m_drmNodeFD.get());
    if (!m_gbmDevice) {
        qWarning("GBM: Failed to initialize GBM device.");
        return;
    }
}

GbmBufferFactory::~GbmBufferFactory() = default;

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
    if (modifiers.empty()) {
        return m_gbmDevice->CreateBufferWithModifiers(fourccFormat, size, gbmFlags,
                                                      { DRM_FORMAT_MOD_LINEAR });
    }

    return m_gbmDevice->CreateBufferWithModifiers(fourccFormat, size, gbmFlags, modifiers);
}

std::unique_ptr<ui::GbmBuffer>
GbmBufferFactory::createBufferFromHandle(gfx::BufferFormat format, gfx::Size size,
                                         gfx::NativePixmapHandle handle)
{
    QMutexLocker locker(&m_mutex);
    if (!m_gbmDevice)
        return nullptr;

    const uint32_t fourccFormat = ui::GetFourCCFormatFromBufferFormat(format);
    return m_gbmDevice->CreateBufferFromHandle(fourccFormat, size, std::move(handle));
}

QT_END_NAMESPACE
