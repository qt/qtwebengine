/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "render_widget_host_view_qt_delegate.h"

#include "backing_store_qt.h"
#include "render_widget_host_view_qt.h"
#include "type_conversion.h"

#include "content/browser/renderer_host/render_view_host_impl.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
#include "cc/quads/draw_quad.h"
#include "cc/quads/render_pass_draw_quad.h"
#include "cc/quads/texture_draw_quad.h"
#include "cc/quads/tile_draw_quad.h"
#include <QSGNode>
#include <QSGSimpleTextureNode>
#include <QSGTexture>
#include <QtQuick/private/qquickclipnode_p.h>
#include <QtQuick/private/qquickwindow_p.h>
#include <QtQuick/private/qsgcontext_p.h>
#include <QtQuick/private/qsgrenderer_p.h>

class RenderPassTexture : public QSGTexture
{
public:
    RenderPassTexture(const cc::RenderPass::Id &id, QSGContext *context);

    cc::RenderPass::Id &id() { return m_id; }
    void bind();

    int textureId() const { return m_fbo ? m_fbo->texture() : 0; }
    QSize textureSize() const { return m_rect.size(); }
    bool hasAlphaChannel() const { return m_format != GL_RGB; }
    bool hasMipmaps() const { return false; }

    void setRect(const QRect &rect) { m_rect = rect; }
    void setFormat(GLenum format) { m_format = format; }
    void setDevicePixelRatio(qreal ratio) { m_device_pixel_ratio = ratio; }
    QSGNode *rootNode() { return m_rootNode.data(); }

    void grab();

private:
    cc::RenderPass::Id m_id;
    QRect m_rect;
    qreal m_device_pixel_ratio;
    GLenum m_format;

    QScopedPointer<QSGRootNode> m_rootNode;
    QScopedPointer<QSGRenderer> m_renderer;
    QScopedPointer<QOpenGLFramebufferObject> m_fbo;

    QSGContext *m_context;
};

RenderPassTexture::RenderPassTexture(const cc::RenderPass::Id &id, QSGContext *context)
    : QSGTexture()
    , m_id(id)
    , m_device_pixel_ratio(1)
    , m_format(GL_RGBA)
    , m_rootNode(new QSGRootNode)
    , m_context(context)
{
}

void RenderPassTexture::bind()
{
    glBindTexture(GL_TEXTURE_2D, m_fbo ? m_fbo->texture() : 0);
    updateBindOptions();
}

void RenderPassTexture::grab()
{
    if (!m_rootNode->firstChild()) {
        m_renderer.reset();
        m_fbo.reset();
        return;
    }
    if (!m_renderer) {
        m_renderer.reset(m_context->createRenderer());
        m_renderer->setRootNode(m_rootNode.data());
    }
    m_renderer->setDevicePixelRatio(m_device_pixel_ratio);

    if (!m_fbo || m_fbo->size() != m_rect.size() || m_fbo->format().internalTextureFormat() != m_format)
    {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        format.setInternalTextureFormat(m_format);

        m_fbo.reset(new QOpenGLFramebufferObject(m_rect.size(), format));
        glBindTexture(GL_TEXTURE_2D, m_fbo->texture());
        updateBindOptions(true);
    }

    m_rootNode->markDirty(QSGNode::DirtyForceUpdate); // Force matrix, clip and opacity update.
    m_renderer->nodeChanged(m_rootNode.data(), QSGNode::DirtyForceUpdate); // Force render list update.

    m_renderer->setDeviceRect(m_rect.size());
    m_renderer->setViewportRect(m_rect.size());
    QRectF mirrored(m_rect.left(), m_rect.bottom(), m_rect.width(), -m_rect.height());
    m_renderer->setProjectionMatrixToRect(mirrored);
    m_renderer->setClearColor(Qt::transparent);

    m_context->renderNextFrame(m_renderer.data(), m_fbo->handle());
}

class RawTexture : public QSGTexture
{
public:
    RawTexture(GLuint textureId, const QSize &textureSize, bool hasAlpha);

    void bind();
    int textureId() const { return m_textureId; }
    QSize textureSize() const { return m_textureSize; }
    bool hasAlphaChannel() const { return m_hasAlpha; }
    bool hasMipmaps() const { return false; }

private:
    GLuint m_textureId;
    QSize m_textureSize;
    uint m_hasAlpha : 1;
};

RawTexture::RawTexture(GLuint textureId, const QSize &textureSize, bool hasAlpha)
    : QSGTexture()
    , m_textureId(textureId)
    , m_textureSize(textureSize)
    , m_hasAlpha(hasAlpha)
{
}

void RawTexture::bind()
{
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    updateBindOptions();
}

class RawTextureNode : public QSGSimpleTextureNode {
public:
    RawTextureNode(GLuint textureId, const QSize &textureSize, bool hasAlpha);

private:
    RawTexture m_texture;
};

RawTextureNode::RawTextureNode(GLuint textureId, const QSize &textureSize, bool hasAlpha)
    : m_texture(textureId, textureSize, hasAlpha)
{
    setTexture(&m_texture);
}

class DelegatedFrameNode : public QSGNode {
public:
    DelegatedFrameNode();
    ~DelegatedFrameNode();
    void preprocess();
    QList<QSharedPointer<RenderPassTexture> > renderPassTextures;

    const size_t testTexturesSize;
    GLuint testTextures[Qt::transparent - Qt::red];
};

DelegatedFrameNode::DelegatedFrameNode()
    : testTexturesSize(sizeof(testTextures) / sizeof(GLuint))
{
    setFlag(UsePreprocess);

    // Generate plain color textures to be used until we can use the real textures from the ResourceProvider.
    glGenTextures(testTexturesSize, testTextures);
    for (unsigned i = 0; i < testTexturesSize; ++i) {
        QImage image(1, 1, QImage::Format_ARGB32_Premultiplied);
        image.fill(static_cast<Qt::GlobalColor>(i + Qt::red));

        // Swizzle
        const int width = image.width();
        const int height = image.height();
        for (int j = 0; j < height; ++j) {
            uint *p = (uint *) image.scanLine(j);
            for (int x = 0; x < width; ++x)
                p[x] = ((p[x] << 16) & 0xff0000) | ((p[x] >> 16) & 0xff) | (p[x] & 0xff00ff00);
        }

        glBindTexture(GL_TEXTURE_2D, testTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.constBits() );
    }
}

DelegatedFrameNode::~DelegatedFrameNode()
{
    glDeleteTextures(testTexturesSize, testTextures);
}

void DelegatedFrameNode::preprocess()
{
    // Render any intermediate RenderPass in order.
    Q_FOREACH (const QSharedPointer<RenderPassTexture> &texture, renderPassTextures)
        texture->grab();
}

static inline QSharedPointer<RenderPassTexture> findRenderPassTexture(const cc::RenderPass::Id &id, const QList<QSharedPointer<RenderPassTexture> > &list)
{
    Q_FOREACH (const QSharedPointer<RenderPassTexture> &texture, list)
        if (texture->id() == id)
            return texture;
    return QSharedPointer<RenderPassTexture>();
}

static QSGNode *buildRenderPassChain(QSGNode *chainParent, const cc::RenderPass *renderPass)
{
    // Chromium already ordered the quads from back to front for us, however the
    // Qt scene graph layers individual geometries in their own z-range and uses
    // the depth buffer to visually stack nodes according to their item tree order.
    // This finds the z-span of all layers so that we can z-compress them to fit
    // them between 0.0 and 1.0 on the z axis.
    double minZ = 0;
    double maxZ = 1;
    double src2[8];
    double dst4[16];
    // topleft.x, topleft.y, topRight.y and bottomLeft.x
    src2[0] = src2[1] = src2[3] = src2[4] = 0;

    // Go through each layer in this pass and find out their transformed rect.
    cc::SharedQuadStateList::const_iterator it = renderPass->shared_quad_state_list.begin();
    cc::SharedQuadStateList::const_iterator sharedStateEnd = renderPass->shared_quad_state_list.end();
    for (; it != sharedStateEnd; ++it) {
        gfx::Size &layerSize = (*it)->content_bounds;
        // topRight.x
        src2[2] = layerSize.width();
        // bottomLeft.y
        src2[5] = layerSize.height();
        // bottomRight
        src2[6] = layerSize.width();
        src2[7] = layerSize.height();
        (*it)->content_to_target_transform.matrix().map2(src2, 4, dst4);
        // Check the mapped corner's z value and track the boundaries.
        minZ = std::min(std::min(std::min(std::min(minZ, dst4[2]), dst4[6]), dst4[10]), dst4[14]);
        maxZ = std::max(std::max(std::max(std::max(maxZ, dst4[2]), dst4[6]), dst4[10]), dst4[14]);
    }

    QSGTransformNode *zCompressNode = new QSGTransformNode;
    QMatrix4x4 zCompressMatrix;
    zCompressMatrix.scale(1, 1, 1 / (maxZ - minZ));
    zCompressMatrix.translate(0, 0, -minZ);
    zCompressNode->setMatrix(zCompressMatrix);
    chainParent->appendChildNode(zCompressNode);
    return zCompressNode;
}

static QSGNode *buildLayerChain(QSGNode *chainParent, const cc::SharedQuadState *layerState)
{
    QSGNode *layerChain = chainParent;
    if (layerState->is_clipped) {
        QQuickDefaultClipNode *clipNode = new QQuickDefaultClipNode(toQt(layerState->clip_rect));
        clipNode->update();
        layerChain->appendChildNode(clipNode);
        layerChain = clipNode;
    }
    if (!layerState->content_to_target_transform.IsIdentity()) {
        QSGTransformNode *transformNode = new QSGTransformNode;
        transformNode->setMatrix(toQt(layerState->content_to_target_transform.matrix()));
        layerChain->appendChildNode(transformNode);
        layerChain = transformNode;
    }
    if (layerState->opacity < 1.0) {
        QSGOpacityNode *opacityNode = new QSGOpacityNode;
        opacityNode->setOpacity(layerState->opacity);
        layerChain->appendChildNode(opacityNode);
        layerChain = opacityNode;
    }
    return layerChain;
}
#endif // QT_VERSION

RenderWidgetHostViewQtDelegate::RenderWidgetHostViewQtDelegate()
    : m_view(0), m_backingStore(0)
{
}

RenderWidgetHostViewQtDelegate::~RenderWidgetHostViewQtDelegate()
{
}

void RenderWidgetHostViewQtDelegate::paint(QPainter *painter, const QRectF &boundingRect)
{
    if (m_backingStore)
        m_backingStore->paintToTarget(painter, boundingRect);
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
QSGNode *RenderWidgetHostViewQtDelegate::updatePaintNode(QSGNode *oldNode, QQuickWindow *window)
{
    cc::DelegatedFrameData *frameData = m_view->pendingDelegatedFrame();
    if (!frameData) {
        delete oldNode;
        return 0;
    }
    DelegatedFrameNode *frameNode = static_cast<DelegatedFrameNode *>(oldNode);
    if (!frameNode)
        frameNode = new DelegatedFrameNode;

    // Keep the old list around to find the ones we can re-use.
    QList<QSharedPointer<RenderPassTexture> > oldRenderPassTextures;
    frameNode->renderPassTextures.swap(oldRenderPassTextures);

    // The RenderPasses list is actually a tree where a parent RenderPasse is connected
    // to its dependencies through a RenderPass::Id reference in one or more RenderPassQuads.
    // The list is already ordered with intermediate RenderPasses placed before their
    // parent, with the last one in the list being the root RenderPass, the one
    // that we displayed to the user.
    // All RenderPasses except the last one are rendered to an FBO.
    cc::RenderPass *rootRenderPass = frameData->render_pass_list.back();

    for (unsigned i = 0; i < frameData->render_pass_list.size(); ++i) {
        cc::RenderPass *pass = frameData->render_pass_list.at(i);

        QSGNode *renderPassParent = 0;
        if (pass != rootRenderPass) {
            QSharedPointer<RenderPassTexture> rpTexture = findRenderPassTexture(pass->id, oldRenderPassTextures);
            if (!rpTexture) {
                QSGContext *sgContext = static_cast<QQuickWindowPrivate *>(QObjectPrivate::get(window))->context;
                rpTexture = QSharedPointer<RenderPassTexture>(new RenderPassTexture(pass->id, sgContext));
            }
            frameNode->renderPassTextures.append(rpTexture);
            rpTexture->setDevicePixelRatio(window->devicePixelRatio());
            rpTexture->setRect(toQt(pass->output_rect));
            rpTexture->setFormat(pass->has_transparent_background ? GL_RGBA : GL_RGB);
            renderPassParent = rpTexture->rootNode();
        } else
            renderPassParent = frameNode;

        // There is currently no way to know which and how quads changed since the last frame.
        // We have to reconstruct the node chain with their geometries on every update.
        while (QSGNode *oldChain = renderPassParent->firstChild())
            delete oldChain;

        QSGNode *renderPassChain = buildRenderPassChain(renderPassParent, pass);
        const cc::SharedQuadState *currentLayerState = 0;
        QSGNode *currentLayerChain = 0;

        cc::QuadList::ConstBackToFrontIterator it = pass->quad_list.BackToFrontBegin();
        cc::QuadList::ConstBackToFrontIterator end = pass->quad_list.BackToFrontEnd();
        for (; it != end; ++it) {
            cc::DrawQuad *quad = *it;

            if (currentLayerState != quad->shared_quad_state) {
                currentLayerState = quad->shared_quad_state;
                currentLayerChain = buildLayerChain(renderPassChain, currentLayerState);
            }

            QSGSimpleTextureNode *textureNode = 0;
            switch (quad->material) {
            case cc::DrawQuad::RENDER_PASS: {
                const cc::RenderPassDrawQuad *renderPassQuad = cc::RenderPassDrawQuad::MaterialCast(quad);
                QSGTexture *texture = findRenderPassTexture(renderPassQuad->render_pass_id, frameNode->renderPassTextures).data();
                if (texture) {
                    textureNode = new QSGSimpleTextureNode;
                    textureNode->setTexture(texture);
                } else {
                    qWarning("Unknown RenderPass layer: Id %d", renderPassQuad->render_pass_id.layer_id);
                    textureNode = new RawTextureNode(0, QSize(1, 1), false);
                }
                break;
            } case cc::DrawQuad::TEXTURE_CONTENT: {
                uint32 resourceId = cc::TextureDrawQuad::MaterialCast(quad)->resource_id;
                textureNode = new RawTextureNode(frameNode->testTextures[resourceId % frameNode->testTexturesSize], QSize(1, 1), false);
                break;
            } case cc::DrawQuad::TILED_CONTENT: {
                uint32 resourceId = cc::TileDrawQuad::MaterialCast(quad)->resource_id;
                textureNode = new RawTextureNode(frameNode->testTextures[resourceId % frameNode->testTexturesSize], QSize(1, 1), false);
                break;
            } default:
                qWarning("Unimplemented quad material: %d", quad->material);
                textureNode = new RawTextureNode(0, QSize(1, 1), false);
            }

            textureNode->setRect(toQt(quad->rect));
            textureNode->setFiltering(QSGTexture::Linear);
            currentLayerChain->appendChildNode(textureNode);
        }
    }

    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
        base::Bind(&RenderWidgetHostViewQt::releaseAndAckDelegatedFrame, m_view->AsWeakPtr()));

    return frameNode;
}
#endif // QT_VERSION

void RenderWidgetHostViewQtDelegate::fetchBackingStore()
{
    Q_ASSERT(m_view);
    m_backingStore = m_view->GetBackingStore();
}

void RenderWidgetHostViewQtDelegate::notifyResize()
{
    Q_ASSERT(m_view);
    m_view->GetRenderWidgetHost()->WasResized();
}

bool RenderWidgetHostViewQtDelegate::forwardEvent(QEvent *event)
{
    return (m_view && m_view->handleEvent(event));
}

void RenderWidgetHostViewQtDelegate::setView(RenderWidgetHostViewQt* view)
{
    m_view = view;
}
