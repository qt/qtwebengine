// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
