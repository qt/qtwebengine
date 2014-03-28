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

// On Mac we need to reset this define in order to prevent definition
// of "check" macros etc. The "check" macro collides with a member function name in QtQuick.
// See AssertMacros.h in the Mac SDK.
#include <QtGlobal> // We need this for the Q_OS_MAC define.
#if defined(Q_OS_MAC)
#undef __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#endif

#include "delegated_frame_node.h"

#include "chromium_gpu_helper.h"
#include "stream_video_node.h"
#include "type_conversion.h"
#include "yuv_video_node.h"

#include "base/message_loop/message_loop.h"
#include "base/bind.h"
#include "cc/output/delegated_frame_data.h"
#include "cc/quads/checkerboard_draw_quad.h"
#include "cc/quads/draw_quad.h"
#include "cc/quads/render_pass_draw_quad.h"
#include "cc/quads/solid_color_draw_quad.h"
#include "cc/quads/stream_video_draw_quad.h"
#include "cc/quads/texture_draw_quad.h"
#include "cc/quads/tile_draw_quad.h"
#include "cc/quads/yuv_video_draw_quad.h"
#include <QOpenGLFramebufferObject>
#include <QSGSimpleTextureNode>
#include <QSGTexture>
#include <QtQuick/private/qquickclipnode_p.h>
#include <QtQuick/private/qsgadaptationlayer_p.h>
#include <QtQuick/private/qsgcontext_p.h>
#include <QtQuick/private/qsgrenderer_p.h>
#include <QtQuick/private/qsgtexture_p.h>

class RenderPassTexture : public QSGTexture
{
public:
    RenderPassTexture(const cc::RenderPass::Id &id, QSGRenderContext *context);

    const cc::RenderPass::Id &id() const { return m_id; }
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

    QSGRenderContext *m_context;
};

class MailboxTexture : public QSGTexture {
public:
    MailboxTexture(const cc::TransferableResource &resource);
    virtual int textureId() const Q_DECL_OVERRIDE { return m_textureId; }
    void setTextureSize(const QSize& size) { m_textureSize = size; }
    virtual QSize textureSize() const Q_DECL_OVERRIDE { return m_textureSize; }
    virtual bool hasAlphaChannel() const Q_DECL_OVERRIDE { return m_hasAlpha; }
    void setHasAlphaChannel(bool hasAlpha) { m_hasAlpha = hasAlpha; }
    virtual bool hasMipmaps() const Q_DECL_OVERRIDE { return false; }
    virtual void bind() Q_DECL_OVERRIDE;

    bool needsToFetch() const { return !m_textureId; }
    cc::TransferableResource &resource() { return m_resource; }
    void fetchTexture(gpu::gles2::MailboxManager *mailboxManager);
    void setTarget(GLenum target);

private:
    cc::TransferableResource m_resource;
    int m_textureId;
    QSize m_textureSize;
    bool m_hasAlpha;
    GLenum m_target;
};

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

RenderPassTexture::RenderPassTexture(const cc::RenderPass::Id &id, QSGRenderContext *context)
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

MailboxTexture::MailboxTexture(const cc::TransferableResource &resource)
    : m_resource(resource)
    , m_textureId(0)
    , m_textureSize(toQt(resource.size))
    , m_hasAlpha(false)
    , m_target(GL_TEXTURE_2D)
{
}

void MailboxTexture::bind()
{
    glBindTexture(m_target, m_textureId);
}

void MailboxTexture::setTarget(GLenum target)
{
    m_target = target;
}

void MailboxTexture::fetchTexture(gpu::gles2::MailboxManager *mailboxManager)
{
    gpu::gles2::Texture *tex = ConsumeTexture(mailboxManager, m_target, *reinterpret_cast<const gpu::gles2::MailboxName*>(m_resource.mailbox.name));

    // The texture might already have been deleted (e.g. when navigating away from a page).
    if (tex)
        m_textureId = service_id(tex);
}

DelegatedFrameNode::DelegatedFrameNode(QSGRenderContext *sgRenderContext)
    : m_sgRenderContext(sgRenderContext)
    , m_numPendingSyncPoints(0)
{
    setFlag(UsePreprocess);
}

DelegatedFrameNode::~DelegatedFrameNode()
{
}

void DelegatedFrameNode::preprocess()
{
    // With the threaded render loop the GUI thread has been unlocked at this point.
    // We can now wait for the Chromium GPU thread to produce textures that will be
    // rendered on our quads and fetch the IDs from the mailboxes we were given.
    QList<MailboxTexture *> mailboxesToFetch;
    Q_FOREACH (const QSharedPointer<MailboxTexture> &mailboxTexture, m_data->mailboxTextures.values())
        if (mailboxTexture->needsToFetch())
            mailboxesToFetch.append(mailboxTexture.data());

    if (!mailboxesToFetch.isEmpty()) {
        QMutexLocker lock(&m_mutex);
        base::MessageLoop *gpuMessageLoop = gpu_message_loop();
        content::SyncPointManager *syncPointManager = sync_point_manager();

        Q_FOREACH (MailboxTexture *mailboxTexture, mailboxesToFetch) {
            m_numPendingSyncPoints++;
            AddSyncPointCallbackOnGpuThread(gpuMessageLoop, syncPointManager, mailboxTexture->resource().sync_point, base::Bind(&DelegatedFrameNode::syncPointRetired, this, &mailboxesToFetch));
        }

        m_mailboxesFetchedWaitCond.wait(&m_mutex);
    }

    // Then render any intermediate RenderPass in order.
    Q_FOREACH (const QSharedPointer<RenderPassTexture> &renderPass, m_renderPassTextures)
        renderPass->grab();
}

void DelegatedFrameNode::commit(DelegatedFrameNodeData* data, cc::ReturnedResourceArray *resourcesToRelease)
{
    m_data = data;
    cc::DelegatedFrameData* frameData = m_data->frameData.get();
    if (!frameData)
        return;

    // DelegatedFrameNode is a transform node only for the purpose of
    // countering the scale of devicePixel-scaled tiles when rendering them
    // to the final surface.
    QMatrix4x4 matrix;
    matrix.scale(1 / m_data->frameDevicePixelRatio, 1 / m_data->frameDevicePixelRatio);
    setMatrix(matrix);

    // Keep the old texture lists around to find the ones we can re-use.
    QList<QSharedPointer<RenderPassTexture> > oldRenderPassTextures;
    m_renderPassTextures.swap(oldRenderPassTextures);
    QHash<unsigned, QSharedPointer<MailboxTexture> > mailboxTextureCandidates;
    m_data->mailboxTextures.swap(mailboxTextureCandidates);

    // A frame's resource_list only contains the new resources to be added to the scene. Quads can
    // still reference resources that were added in previous frames. Add them to the list of
    // candidates to be picked up by quads, it's then our responsibility to return unused resources
    // to the producing child compositor.
    for (unsigned i = 0; i < frameData->resource_list.size(); ++i) {
        const cc::TransferableResource &res = frameData->resource_list.at(i);
        mailboxTextureCandidates[res.id] = QSharedPointer<MailboxTexture>(new MailboxTexture(res));
    }

    frameData->resource_list.clear();

    // The RenderPasses list is actually a tree where a parent RenderPass is connected
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
            if (!rpTexture)
                rpTexture = QSharedPointer<RenderPassTexture>(new RenderPassTexture(pass->id, m_sgRenderContext));
            m_renderPassTextures.append(rpTexture);
            rpTexture->setDevicePixelRatio(m_data->frameDevicePixelRatio);
            rpTexture->setRect(toQt(pass->output_rect));
            rpTexture->setFormat(pass->has_transparent_background ? GL_RGBA : GL_RGB);
            renderPassParent = rpTexture->rootNode();
        } else
            renderPassParent = this;

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

            switch (quad->material) {
            case cc::DrawQuad::CHECKERBOARD: {
                const cc::CheckerboardDrawQuad *cbquad = cc::CheckerboardDrawQuad::MaterialCast(quad);
                QSGRectangleNode *rectangleNode = m_sgRenderContext->sceneGraphContext()->createRectangleNode();

                rectangleNode->setRect(toQt(quad->rect));
                rectangleNode->setColor(toQt(cbquad->color));
                rectangleNode->update();
                currentLayerChain->appendChildNode(rectangleNode);
                break;
            } case cc::DrawQuad::RENDER_PASS: {
                const cc::RenderPassDrawQuad *renderPassQuad = cc::RenderPassDrawQuad::MaterialCast(quad);
                QSGTexture *texture = findRenderPassTexture(renderPassQuad->render_pass_id, m_renderPassTextures).data();
                // cc::GLRenderer::DrawRenderPassQuad silently ignores missing render passes.
                if (!texture)
                    continue;

                QSGSimpleTextureNode *textureNode = new QSGSimpleTextureNode;
                textureNode->setRect(toQt(quad->rect));
                textureNode->setTexture(texture);
                currentLayerChain->appendChildNode(textureNode);
                break;
            } case cc::DrawQuad::TEXTURE_CONTENT: {
                const cc::TextureDrawQuad *tquad = cc::TextureDrawQuad::MaterialCast(quad);
                QSharedPointer<MailboxTexture> &texture = m_data->mailboxTextures[tquad->resource_id] = mailboxTextureCandidates.take(tquad->resource_id);
                Q_ASSERT(texture);

                // FIXME: TransferableResource::size isn't always set properly for TextureDrawQuads, use the size of its DrawQuad::rect instead.
                texture->setTextureSize(toQt(quad->rect.size()));

                // TransferableResource::format seems to always be GL_BGRA even though it might not
                // contain any pixel with alpha < 1.0. The information about if they need blending
                // for the contents itself is actually stored in quads.
                // Tell the scene graph to enable blending for a texture only when at least one quad asks for it.
                // Do not rely on DrawQuad::ShouldDrawWithBlending() since the shared_quad_state->opacity
                // case will be handled by QtQuick by fetching this information from QSGOpacityNodes.
                if (!quad->visible_rect.IsEmpty() && !quad->opaque_rect.Contains(quad->visible_rect))
                    texture->setHasAlphaChannel(true);

                QSGSimpleTextureNode *textureNode = new QSGSimpleTextureNode;
                textureNode->setTextureCoordinatesTransform(tquad->flipped ? QSGSimpleTextureNode::MirrorVertically : QSGSimpleTextureNode::NoTransform);
                textureNode->setRect(toQt(quad->rect));
                textureNode->setFiltering(texture->resource().filter == GL_LINEAR ? QSGTexture::Linear : QSGTexture::Nearest);
                textureNode->setTexture(texture.data());
                currentLayerChain->appendChildNode(textureNode);
                break;
            } case cc::DrawQuad::SOLID_COLOR: {
                const cc::SolidColorDrawQuad *scquad = cc::SolidColorDrawQuad::MaterialCast(quad);
                QSGRectangleNode *rectangleNode = m_sgRenderContext->sceneGraphContext()->createRectangleNode();

                // Qt only supports MSAA and this flag shouldn't be needed.
                // If we ever want to use QSGRectangleNode::setAntialiasing for this we should
                // try to see if we can do something similar for tile quads first.
                Q_UNUSED(scquad->force_anti_aliasing_off);

                rectangleNode->setRect(toQt(quad->rect));
                rectangleNode->setColor(toQt(scquad->color));
                rectangleNode->update();
                currentLayerChain->appendChildNode(rectangleNode);
                break;
            } case cc::DrawQuad::TILED_CONTENT: {
                const cc::TileDrawQuad *tquad = cc::TileDrawQuad::MaterialCast(quad);
                QSharedPointer<MailboxTexture> &texture = m_data->mailboxTextures[tquad->resource_id] = mailboxTextureCandidates.take(tquad->resource_id);
                Q_ASSERT(texture);

                if (!quad->visible_rect.IsEmpty() && !quad->opaque_rect.Contains(quad->visible_rect))
                    texture->setHasAlphaChannel(true);

                QSGSimpleTextureNode *textureNode = new QSGSimpleTextureNode;
                textureNode->setRect(toQt(quad->rect));
                textureNode->setFiltering(texture->resource().filter == GL_LINEAR ? QSGTexture::Linear : QSGTexture::Nearest);
                textureNode->setTexture(texture.data());

                // FIXME: Find out if we can implement a QSGSimpleTextureNode::setSourceRect instead of this hack.
                // This has to be done at the end since many QSGSimpleTextureNode methods would overwrite this.
                QSGGeometry::updateTexturedRectGeometry(textureNode->geometry(), textureNode->rect(), textureNode->texture()->convertToNormalizedSourceRect(toQt(tquad->tex_coord_rect)));
                currentLayerChain->appendChildNode(textureNode);
                break;
            } case cc::DrawQuad::YUV_VIDEO_CONTENT: {
                const cc::YUVVideoDrawQuad *vquad = cc::YUVVideoDrawQuad::MaterialCast(quad);
                QSharedPointer<MailboxTexture> &yTexture = m_data->mailboxTextures[vquad->y_plane_resource_id] = mailboxTextureCandidates.take(vquad->y_plane_resource_id);
                QSharedPointer<MailboxTexture> &uTexture = m_data->mailboxTextures[vquad->u_plane_resource_id] = mailboxTextureCandidates.take(vquad->u_plane_resource_id);
                QSharedPointer<MailboxTexture> &vTexture = m_data->mailboxTextures[vquad->v_plane_resource_id] = mailboxTextureCandidates.take(vquad->v_plane_resource_id);
                Q_ASSERT(yTexture && uTexture && vTexture);

                // Do not use a reference for this one, it might be null.
                QSharedPointer<MailboxTexture> aTexture;
                // This currently requires --enable-vp8-alpha-playback and needs a video with alpha data to be triggered.
                if (vquad->a_plane_resource_id)
                    aTexture = m_data->mailboxTextures[vquad->a_plane_resource_id] = mailboxTextureCandidates.take(vquad->a_plane_resource_id);

                YUVVideoNode *videoNode = new YUVVideoNode(yTexture.data(), uTexture.data(), vTexture.data(), aTexture.data(), toQt(vquad->tex_scale));
                videoNode->setRect(toQt(quad->rect));
                currentLayerChain->appendChildNode(videoNode);
                break;
#ifdef GL_OES_EGL_image_external
            } case cc::DrawQuad::STREAM_VIDEO_CONTENT: {
                const cc::StreamVideoDrawQuad *squad = cc::StreamVideoDrawQuad::MaterialCast(quad);
                QSharedPointer<MailboxTexture> &texture = m_data->mailboxTextures[squad->resource_id] = mailboxTextureCandidates.take(squad->resource_id);
                texture->setTarget(GL_TEXTURE_EXTERNAL_OES); // since this is not default TEXTURE_2D type
                Q_ASSERT(texture);

                StreamVideoNode *svideoNode = new StreamVideoNode(texture.data());
                svideoNode->setRect(toQt(squad->rect));
                currentLayerChain->appendChildNode(svideoNode);
                break;
#endif
            } default:
                qWarning("Unimplemented quad material: %d", quad->material);
            }
        }
    }

    // Send resources of remaining candidates back to the child compositors so that they can be freed or reused.
    Q_FOREACH (const QSharedPointer<MailboxTexture> &mailboxTexture, mailboxTextureCandidates.values()) {
        // The ResourceProvider ensures that the resource isn't used by the parent compositor's GL
        // context in the GPU process by inserting a sync point to be waited for by the child
        // compositor's GL context. We don't need this since we are triggering the delegated frame
        // ack directly from our rendering thread. At this point (in updatePaintNode) we know that
        // a frame that was compositing any of those resources has already been swapped.
        // Save a bit of overhead by resetting the sync point that has initially been put there
        // for us (mainly to clean the output of --enable-gpu-service-logging).
        mailboxTexture->resource().sync_point = 0;

        resourcesToRelease->push_back(mailboxTexture->resource().ToReturnedResource());
    }
}

void DelegatedFrameNode::fetchTexturesAndUnlockQt(DelegatedFrameNode *frameNode, QList<MailboxTexture *> *mailboxesToFetch)
{
    // Fetch texture IDs from the mailboxes while we're on the GPU thread, where the MailboxManager lives.
    gpu::gles2::MailboxManager *mailboxManager = mailbox_manager();
    Q_FOREACH (MailboxTexture *mailboxTexture, *mailboxesToFetch)
        mailboxTexture->fetchTexture(mailboxManager);

    // glFlush before yielding to the SG thread, whose context might already start using
    // some shared resources provided by the unflushed context here, on the Chromium GPU thread.
    glFlush();

    // Chromium provided everything we were waiting for, let Qt start rendering.
    QMutexLocker lock(&frameNode->m_mutex);
    frameNode->m_mailboxesFetchedWaitCond.wakeOne();
}

void DelegatedFrameNode::syncPointRetired(DelegatedFrameNode *frameNode, QList<MailboxTexture *> *mailboxesToFetch)
{
    // The way that sync points are normally used by the GpuCommandBufferStub is that it asks
    // the GpuScheduler to resume the work of the associated GL command stream / context once
    // the sync point has been retired by the dependency's context. In other words, a produced
    // texture means that the mailbox can be consumed, but the texture itself isn't expected
    // to be ready until to control is given back to the GpuScheduler through the event loop.
    // Do the same for our implementation by posting a message to the event loop once the last
    // of our syncpoints has been retired (the syncpoint callback is called synchronously) and
    // only at this point we wake the Qt rendering thread.
    QMutexLocker lock(&frameNode->m_mutex);
    if (!--frameNode->m_numPendingSyncPoints)
        base::MessageLoop::current()->PostTask(FROM_HERE, base::Bind(&DelegatedFrameNode::fetchTexturesAndUnlockQt, frameNode, mailboxesToFetch));
}

