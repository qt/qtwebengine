// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "display_software_output_surface.h"

#include "compositor.h"
#include "type_conversion.h"

#include "base/task/single_thread_task_runner.h"
#include "components/viz/service/display/display.h"
#include "components/viz/service/display/output_surface_frame.h"

#include <QMutex>
#include <QPainter>
#include <QQuickWindow>

namespace QtWebEngineCore {

class DisplaySoftwareOutputSurface::Device final : public viz::SoftwareOutputDevice,
                                                   public Compositor
{
public:
    Device(bool requiresAlpha);

    // Overridden from viz::SoftwareOutputDevice.
    void Resize(const gfx::Size &sizeInPixels, float devicePixelRatio) override;
    void OnSwapBuffers(SwapBuffersCallback swap_ack_callback, gfx::FrameData data) override;

    // Overridden from Compositor.
    void swapFrame() override;
    QSGTexture *texture(QQuickWindow *win, uint32_t) override;
    bool textureIsFlipped() override;
    float devicePixelRatio() override;
    QSize size() override;
    bool requiresAlphaChannel() override;

private:
    mutable QMutex m_mutex;
    float m_devicePixelRatio = 1.0;
    bool m_requiresAlpha;
    scoped_refptr<base::SingleThreadTaskRunner> m_taskRunner;
    SwapBuffersCallback m_swapCompletionCallback;
    QImage m_image;
    float m_imageDevicePixelRatio = 1.0;
};

DisplaySoftwareOutputSurface::Device::Device(bool requiresAlpha)
    : Compositor(Type::Software)
    , m_requiresAlpha(requiresAlpha)
{
}

void DisplaySoftwareOutputSurface::Device::Resize(const gfx::Size &sizeInPixels, float devicePixelRatio)
{
    if (viewport_pixel_size_ == sizeInPixels && m_devicePixelRatio == devicePixelRatio)
        return;
    m_devicePixelRatio = devicePixelRatio;
    viewport_pixel_size_ = sizeInPixels;
    surface_ = SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(sizeInPixels.width(), sizeInPixels.height()));
}

void DisplaySoftwareOutputSurface::Device::OnSwapBuffers(SwapBuffersCallback swap_ack_callback, gfx::FrameData data)
{
    { // MEMO don't hold a lock together with an 'observer', as the call from Qt's scene graph may come at the same time
        QMutexLocker locker(&m_mutex);
        m_taskRunner = base::SingleThreadTaskRunner::GetCurrentDefault();
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

QSGTexture *DisplaySoftwareOutputSurface::Device::texture(QQuickWindow *win, uint32_t)
{
    return win->createTextureFromImage(m_image);
}

bool DisplaySoftwareOutputSurface::Device::textureIsFlipped()
{
    return false;
}

float DisplaySoftwareOutputSurface::Device::devicePixelRatio()
{
    return m_imageDevicePixelRatio;
}

QSize DisplaySoftwareOutputSurface::Device::size()
{
    return m_image.size();
}

bool DisplaySoftwareOutputSurface::Device::requiresAlphaChannel()
{
    return m_requiresAlpha;
}

DisplaySoftwareOutputSurface::DisplaySoftwareOutputSurface(bool requiresAlpha)
    : SoftwareOutputSurface(std::make_unique<Device>(requiresAlpha))
{}

DisplaySoftwareOutputSurface::~DisplaySoftwareOutputSurface() {}

// Called from viz::Display::Initialize.
void DisplaySoftwareOutputSurface::SetFrameSinkId(const viz::FrameSinkId &id)
{
    static_cast<Device *>(software_device())->bind(id);
}

} // namespace QtWebEngineCore
