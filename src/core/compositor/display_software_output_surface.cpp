/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#include "display_software_output_surface.h"

#include "compositor.h"
#include "render_widget_host_view_qt_delegate.h"
#include "type_conversion.h"

#include "base/threading/thread_task_runner_handle.h"
#include "components/viz/service/display/display.h"
#include "components/viz/service/display/output_surface_frame.h"

#include <QMutex>
#include <QPainter>

namespace QtWebEngineCore {

class DisplaySoftwareOutputSurface::Device final : public viz::SoftwareOutputDevice,
                                                   public Compositor
{
public:
    Device();

    // Overridden from viz::SoftwareOutputDevice.
    void Resize(const gfx::Size &sizeInPixels, float devicePixelRatio) override;
    void OnSwapBuffers(SwapBuffersCallback swap_ack_callback) override;

    // Overridden from Compositor.
    void swapFrame() override;
    QImage image() override;
    float devicePixelRatio() override;
    QSize size() override;
    bool hasAlphaChannel() override;

private:
    mutable QMutex m_mutex;
    float m_devicePixelRatio = 1.0;
    scoped_refptr<base::SingleThreadTaskRunner> m_taskRunner;
    SwapBuffersCallback m_swapCompletionCallback;
    QImage m_image;
    float m_imageDevicePixelRatio = 1.0;
};

DisplaySoftwareOutputSurface::Device::Device()
    : Compositor(Type::Software)
{}

void DisplaySoftwareOutputSurface::Device::Resize(const gfx::Size &sizeInPixels, float devicePixelRatio)
{
    if (viewport_pixel_size_ == sizeInPixels && m_devicePixelRatio == devicePixelRatio)
        return;
    m_devicePixelRatio = devicePixelRatio;
    viewport_pixel_size_ = sizeInPixels;
    surface_ = SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(sizeInPixels.width(), sizeInPixels.height()));
}

void DisplaySoftwareOutputSurface::Device::OnSwapBuffers(SwapBuffersCallback swap_ack_callback)
{
    { // MEMO don't hold a lock together with an 'observer', as the call from Qt's scene graph may come at the same time
        QMutexLocker locker(&m_mutex);
        m_taskRunner = base::ThreadTaskRunnerHandle::Get();
        m_swapCompletionCallback = std::move(swap_ack_callback);
    }

    if (auto obs = observer())
        obs->readyToSwap();
}

inline QImage::Format imageFormat(SkColorType colorType)
{
    switch (colorType) {
    case kBGRA_8888_SkColorType:
        return QImage::Format_ARGB32_Premultiplied;
    case kRGBA_8888_SkColorType:
        return QImage::Format_RGBA8888_Premultiplied;
    default:
        Q_UNREACHABLE();
        return QImage::Format_ARGB32_Premultiplied;
    }
}

void DisplaySoftwareOutputSurface::Device::swapFrame()
{
    QMutexLocker locker(&m_mutex);

    if (!m_swapCompletionCallback)
        return;

    SkPixmap skPixmap;
    surface_->peekPixels(&skPixmap);
    QImage image(reinterpret_cast<const uchar *>(skPixmap.addr()), viewport_pixel_size_.width(),
                 viewport_pixel_size_.height(), skPixmap.rowBytes(),
                 imageFormat(skPixmap.colorType()));
    if (m_image.size() == image.size()) {
        QRect damageRect = toQt(damage_rect_);
        QPainter painter(&m_image);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawImage(damageRect, image, damageRect);
    } else {
        m_image = image;
        m_image.detach();
    }
    m_imageDevicePixelRatio = m_devicePixelRatio;
    m_taskRunner->PostTask(
            FROM_HERE, base::BindOnce(std::move(m_swapCompletionCallback), toGfx(m_image.size())));
    m_taskRunner.reset();
}

QImage DisplaySoftwareOutputSurface::Device::image()
{
    return m_image;
}

float DisplaySoftwareOutputSurface::Device::devicePixelRatio()
{
    return m_imageDevicePixelRatio;
}

QSize DisplaySoftwareOutputSurface::Device::size()
{
    return m_image.size();
}

bool DisplaySoftwareOutputSurface::Device::hasAlphaChannel()
{
    return m_image.format() == QImage::Format_ARGB32_Premultiplied;
}

DisplaySoftwareOutputSurface::DisplaySoftwareOutputSurface()
    : SoftwareOutputSurface(std::make_unique<Device>())
{}

DisplaySoftwareOutputSurface::~DisplaySoftwareOutputSurface() {}

// Called from viz::Display::Initialize.
void DisplaySoftwareOutputSurface::SetFrameSinkId(const viz::FrameSinkId &id)
{
    static_cast<Device *>(software_device())->bind(id);
}

} // namespace QtWebEngineCore
