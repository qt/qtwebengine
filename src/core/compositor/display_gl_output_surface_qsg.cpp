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

#include "display_gl_output_surface.h"

#include "compositor_resource_fence.h"
#include "render_widget_host_view_qt_delegate.h"
#include "type_conversion.h"

#include <QOpenGLFunctions>
#include <QSGImageNode>
#include <QSGTexture>

namespace QtWebEngineCore {

class DisplayGLOutputSurface::Texture final : public QSGTexture
{
public:
    Texture(uint32_t id, QSize sizeInPixels, bool hasAlphaChannel, scoped_refptr<CompositorResourceFence> fence)
        : m_id(id)
        , m_sizeInPixels(sizeInPixels)
        , m_hasAlphaChannel(hasAlphaChannel)
        , m_fence(std::move(fence))
    {
    }

    // QSGTexture:
    int textureId() const override { return m_id; }
    QSize textureSize() const override { return m_sizeInPixels; }
    bool hasAlphaChannel() const override { return m_hasAlphaChannel; }
    bool hasMipmaps() const override { return false; }
    void bind() override
    {
        if (m_fence) {
            m_fence->wait();
            m_fence.reset();
        }

        QOpenGLContext *context = QOpenGLContext::currentContext();
        QOpenGLFunctions *funcs = context->functions();
        funcs->glBindTexture(GL_TEXTURE_2D, m_id);
    }

private:
    uint32_t m_id;
    QSize m_sizeInPixels;
    bool m_hasAlphaChannel;
    scoped_refptr<CompositorResourceFence> m_fence;
};

QSGNode *DisplayGLOutputSurface::updatePaintNode(QSGNode *oldNode, RenderWidgetHostViewQtDelegate *delegate)
{
    {
        QMutexLocker locker(&m_mutex);
        if (m_readyToUpdate) {
            std::swap(m_middleBuffer, m_frontBuffer);
            m_taskRunner->PostTask(
                    FROM_HERE,
                    base::BindOnce(&DisplayGLOutputSurface::swapBuffersOnVizThread, base::Unretained(this)));
            m_taskRunner.reset();
            m_readyToUpdate = false;
        }
    }

    if (!m_frontBuffer)
        return oldNode;

    auto node = static_cast<QSGImageNode *>(oldNode);
    if (!node)
        node = delegate->createImageNode();

    QSize sizeInPixels = toQt(m_frontBuffer->shape.sizeInPixels);
    QSizeF sizeInDips = QSizeF(sizeInPixels) / m_frontBuffer->shape.devicePixelRatio;
    QRectF rectInDips(QPointF(0, 0), sizeInDips);
    node->setRect(rectInDips);
    node->setOwnsTexture(true);
    node->setTexture(new Texture(m_frontBuffer->serviceId,
                                 sizeInPixels,
                                 m_frontBuffer->shape.hasAlpha,
                                 m_frontBuffer->fence));
    node->setTextureCoordinatesTransform(QSGImageNode::MirrorVertically);

    return node;
}

} // namespace QtWebEngineCore
