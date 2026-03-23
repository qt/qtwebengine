// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "gbm_buffer_factory.h"

#include "ui/gfx/linux/drm_util_linux.h"
#include "ui/gfx/linux/gbm_buffer.h"
#include "ui/gfx/linux/gbm_device.h"
#include "ui/gfx/linux/gbm_util.h"
#include "ui/gfx/linux/gbm_wrapper.h"
#include "ui/ozone/platform/wayland/common/drm_render_node_handle.h"

QT_BEGIN_NAMESPACE

GbmBufferFactory::GbmBufferFactory(const std::string &drmNodePath)
{
    // It is safe to initialize the GBM device on the UI thread and allocate buffers on the GPU
    // thread because m_mutex serializes access and guarantees the device is fully initialized
    // before allocation begins.
    QMutexLocker locker(&m_mutex);

    ui::DrmRenderNodeHandle nodeHandle;
    if (!nodeHandle.Initialize(base::FilePath(drmNodePath))) {
        qWarning("GBM: Failed to initialize DRM render node handle: %s\n", drmNodePath.data());
        return;
    }

    m_drmNodeFD = nodeHandle.PassFD();
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

bool GbmBufferFactory::canCreateNativePixmapForFormat(gfx::BufferFormat format) const
{
    QMutexLocker locker(&m_mutex);
    if (!m_gbmDevice)
        return false;

    const uint32_t fourccFormat = ui::GetFourCCFormatFromBufferFormat(format);
    return m_gbmDevice->CanCreateBufferForFormat(fourccFormat);
}

std::unique_ptr<ui::GbmBuffer>
GbmBufferFactory::createBuffer(gfx::BufferFormat format, gfx::Size size, gfx::BufferUsage usage)
{
    QMutexLocker locker(&m_mutex);
    if (!m_gbmDevice)
        return nullptr;

    const uint32_t fourccFormat = ui::GetFourCCFormatFromBufferFormat(format);
    const uint32_t gbmFlags = ui::BufferUsageToGbmFlags(usage);
    // FIXME: CreateBufferWithModifiers for wayland?
    return m_gbmDevice->CreateBuffer(fourccFormat, size, gbmFlags);
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
