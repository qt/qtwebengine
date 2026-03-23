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

namespace ui {
class GbmBuffer;
class GbmDevice;
}

QT_BEGIN_NAMESPACE

class GbmBufferFactory
{
public:
    GbmBufferFactory(const std::string &drmNodePath);
    ~GbmBufferFactory();

    bool hasDevice() const { return m_gbmDevice.get() != nullptr; }
    bool canCreateNativePixmapForFormat(gfx::BufferFormat format) const;
    std::unique_ptr<ui::GbmBuffer> createBuffer(gfx::BufferFormat format, gfx::Size size,
                                                gfx::BufferUsage usage);
    std::unique_ptr<ui::GbmBuffer> createBufferFromHandle(gfx::BufferFormat format, gfx::Size size,
                                                          gfx::NativePixmapHandle handle);

private:
    base::ScopedFD m_drmNodeFD;
    std::unique_ptr<ui::GbmDevice> m_gbmDevice;

    mutable QMutex m_mutex;
};

QT_END_NAMESPACE

#endif // GBM_BUFFER_FACTORY_H
