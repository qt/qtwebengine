// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef GBM_BUFFER_FACTORY_H
#define GBM_BUFFER_FACTORY_H

#include <QtCore/qmutex.h>

#include "base/files/scoped_file.h"
#include "ui/gfx/buffer_types.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/native_pixmap_handle.h"

#include <memory>
#include <string>
#include <vector>

namespace ui {
class GbmBuffer;
class GbmDevice;
}

QT_BEGIN_NAMESPACE

class GbmBufferFactory
{
public:
    static GbmBufferFactory *instance();

    GbmBufferFactory(const std::string &drmNodePath);
    GbmBufferFactory(base::ScopedFD drmNodeFD);
    ~GbmBufferFactory();

    bool hasDevice() const { return m_gbmDevice.get() != nullptr; }
    std::string drmRenderNodePath() const;
    std::string drmDeviceString() const;
    bool canCreateNativePixmapForFormat(gfx::BufferFormat format) const;
    bool isSinglePlanar(uint32_t fourccFormat, uint64_t modifier) const;
    std::unique_ptr<ui::GbmBuffer>
    createBufferWithModifiers(gfx::BufferFormat format, gfx::Size size, gfx::BufferUsage usage,
                              const std::vector<uint64_t> &modifiers);
    std::unique_ptr<ui::GbmBuffer> createBufferFromHandle(gfx::BufferFormat format, gfx::Size size,
                                                          gfx::NativePixmapHandle handle);

private:
    base::ScopedFD m_drmNodeFD;
    std::unique_ptr<ui::GbmDevice> m_gbmDevice;

    mutable QMutex m_mutex;
};

QT_END_NAMESPACE

#endif // GBM_BUFFER_FACTORY_H
