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

#include "display_frame_sink.h"
#include "render_widget_host_view_qt_delegate.h"
#include "type_conversion.h"

#include "base/threading/thread_task_runner_handle.h"
#include "components/viz/service/display/display.h"
#include "components/viz/service/display/output_surface_frame.h"

#include <QMutex>
#include <QPainter>
#include <QSGImageNode>

namespace QtWebEngineCore {

class DisplaySoftwareOutputSurface::Device final : public viz::SoftwareOutputDevice, public DisplayProducer
{
public:
    ~Device();

    // Called from DisplaySoftwareOutputSurface.
    void bind(viz::FrameSinkId frameSinkId);

    // Overridden from viz::SoftwareOutputDevice.
    void Resize(const gfx::Size &sizeInPixels, float devicePixelRatio) override;
    void OnSwapBuffers(SwapBuffersCallback swap_ack_callback) override;

    // Overridden from DisplayProducer.
    QSGNode *updatePaintNode(QSGNode *oldNode, RenderWidgetHostViewQtDelegate *delegate) override;

private:
    mutable QMutex m_mutex;
    scoped_refptr<DisplayFrameSink> m_sink;
    float m_devicePixelRatio = 1.0;
    scoped_refptr<base::SingleThreadTaskRunner> m_taskRunner;
    SwapBuffersCallback m_swapCompletionCallback;
    QImage m_image;
    float m_imageDevicePixelRatio = 1.0;
};

DisplaySoftwareOutputSurface::Device::~Device()
{
    if (m_sink)
        m_sink->disconnect(this);
}

void DisplaySoftwareOutputSurface::Device::bind(viz::FrameSinkId frameSinkId)
{
    m_sink = DisplayFrameSink::findOrCreate(frameSinkId);
    m_sink->connect(this);
}

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
    QMutexLocker locker(&m_mutex);
    m_taskRunner = base::ThreadTaskRunnerHandle::Get();
    m_swapCompletionCallback = std::move(swap_ack_callback);
    m_sink->scheduleUpdate();
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

QSGNode *DisplaySoftwareOutputSurface::Device::updatePaintNode(
        QSGNode *oldNode, RenderWidgetHostViewQtDelegate *delegate)
{
    QMutexLocker locker(&m_mutex);

    // Delete old node to make sure refcount of m_image is at most 1.
    delete oldNode;
    QSGImageNode *node = delegate->createImageNode();

    if (m_swapCompletionCallback) {
        SkPixmap skPixmap;
        surface_->peekPixels(&skPixmap);
        QImage image(reinterpret_cast<const uchar *>(skPixmap.addr()),
                     viewport_pixel_size_.width(), viewport_pixel_size_.height(),
                     skPixmap.rowBytes(), imageFormat(skPixmap.colorType()));
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
        m_taskRunner->PostTask(FROM_HERE, base::BindOnce(std::move(m_swapCompletionCallback), toGfx(m_image.size())));
        m_taskRunner.reset();
    }

    QSizeF sizeInDips = QSizeF(m_image.size()) / m_imageDevicePixelRatio;
    node->setRect(QRectF(QPointF(0, 0), sizeInDips));
    node->setOwnsTexture(true);
    node->setTexture(delegate->createTextureFromImage(m_image));

    return node;
}

DisplaySoftwareOutputSurface::DisplaySoftwareOutputSurface()
    : SoftwareOutputSurface(std::make_unique<Device>())
{}

DisplaySoftwareOutputSurface::~DisplaySoftwareOutputSurface() {}

// Called from viz::Display::Initialize.
void DisplaySoftwareOutputSurface::BindToClient(viz::OutputSurfaceClient *client)
{
    auto display = static_cast<viz::Display *>(client);
    auto device = static_cast<Device *>(software_device());
    device->bind(display->frame_sink_id());
    SoftwareOutputSurface::BindToClient(client);
}

} // namespace QtWebEngineCore
