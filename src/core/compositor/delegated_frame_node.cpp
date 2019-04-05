/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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
#include "compositor_resource_tracker.h"

#include "base/bind.h"
#include "cc/base/math_util.h"
#include "components/viz/common/quads/compositor_frame.h"
#include "components/viz/common/quads/debug_border_draw_quad.h"
#include "components/viz/common/quads/draw_quad.h"
#include "components/viz/common/quads/render_pass_draw_quad.h"
#include "components/viz/common/quads/solid_color_draw_quad.h"
#include "components/viz/common/quads/stream_video_draw_quad.h"
#include "components/viz/common/quads/texture_draw_quad.h"
#include "components/viz/common/quads/tile_draw_quad.h"
#include "components/viz/common/quads/yuv_video_draw_quad.h"
#include "components/viz/service/display/bsp_tree.h"
#include "components/viz/service/display_embedder/server_shared_bitmap_manager.h"

#ifndef QT_NO_OPENGL
# include <QOpenGLContext>
# include <QOpenGLFunctions>
# include <QSGFlatColorMaterial>
#endif
#include <QSGTexture>
#include <private/qsgadaptationlayer_p.h>

#include <QSGImageNode>
#include <QSGRectangleNode>

#if !defined(QT_NO_EGL)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

#ifndef GL_TEXTURE_RECTANGLE
#define GL_TEXTURE_RECTANGLE              0x84F5
#endif

#ifndef GL_NEAREST
#define GL_NEAREST                        0x2600
#endif

#ifndef GL_LINEAR
#define GL_LINEAR                         0x2601
#endif

#ifndef GL_RGBA
#define GL_RGBA                           0x1908
#endif

#ifndef GL_RGB
#define GL_RGB                            0x1907
#endif

#ifndef GL_LINE_LOOP
#define GL_LINE_LOOP                      0x0002
#endif

#ifndef QT_NO_OPENGL
QT_BEGIN_NAMESPACE
Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();
QT_END_NAMESPACE
#endif

namespace QtWebEngineCore {
#ifndef QT_NO_OPENGL
class MailboxTexture : public QSGTexture, protected QOpenGLFunctions {
public:
    MailboxTexture(const CompositorResource *resource, bool hasAlphaChannel, int target = -1);
    ~MailboxTexture();
    // QSGTexture:
    int textureId() const override { return m_textureId; }
    QSize textureSize() const override { return m_textureSize; }
    bool hasAlphaChannel() const override { return m_hasAlpha; }
    bool hasMipmaps() const override { return false; }
    void bind() override;

private:
    int m_textureId;
    scoped_refptr<CompositorResourceFence> m_fence;
    QSize m_textureSize;
    bool m_hasAlpha;
    GLenum m_target;
#if defined(USE_OZONE)
    bool m_ownsTexture;
#endif
#ifdef Q_OS_QNX
    EGLStreamData m_eglStreamData;
#endif
    friend class DelegatedFrameNode;
};
#endif // QT_NO_OPENGL

class RectClipNode : public QSGClipNode
{
public:
    RectClipNode(const QRectF &);
private:
    QSGGeometry m_geometry;
};

class DelegatedNodeTreeHandler
{
public:
    DelegatedNodeTreeHandler(QVector<QSGNode*> *sceneGraphNodes)
        : m_sceneGraphNodes(sceneGraphNodes)
    {
    }

    virtual ~DelegatedNodeTreeHandler(){}

    virtual void setupRenderPassNode(QSGTexture *, const QRect &, const QRectF &, QSGNode *) = 0;
    virtual void setupTextureContentNode(QSGTexture *, const QRect &, const QRectF &,
                                         QSGImageNode::TextureCoordinatesTransformMode,
                                         QSGNode *) = 0;
    virtual void setupSolidColorNode(const QRect &, const QColor &, QSGNode *) = 0;

#ifndef QT_NO_OPENGL
    virtual void setupDebugBorderNode(QSGGeometry *, QSGFlatColorMaterial *, QSGNode *) = 0;
    virtual void setupYUVVideoNode(QSGTexture *, QSGTexture *, QSGTexture *, QSGTexture *,
                           const QRectF &, const QRectF &, const QSizeF &, const QSizeF &,
                           gfx::ColorSpace, float, float, const QRectF &,
                                   QSGNode *) = 0;
#ifdef GL_OES_EGL_image_external
    virtual void setupStreamVideoNode(MailboxTexture *, const QRectF &,
                                      const QMatrix4x4 &, QSGNode *) = 0;
#endif // GL_OES_EGL_image_external
#endif // QT_NO_OPENGL
protected:
    QVector<QSGNode*> *m_sceneGraphNodes;
};

class DelegatedNodeTreeUpdater : public DelegatedNodeTreeHandler
{
public:
    DelegatedNodeTreeUpdater(QVector<QSGNode*> *sceneGraphNodes)
        : DelegatedNodeTreeHandler(sceneGraphNodes)
        , m_nodeIterator(sceneGraphNodes->begin())
    {
    }

    void setupRenderPassNode(QSGTexture *layer, const QRect &rect, const QRectF &sourceRect, QSGNode *) override
    {
        Q_ASSERT(layer);
        Q_ASSERT(m_nodeIterator != m_sceneGraphNodes->end());
        QSGInternalImageNode *imageNode = static_cast<QSGInternalImageNode*>(*m_nodeIterator++);
        imageNode->setTargetRect(rect);
        imageNode->setInnerTargetRect(rect);
        imageNode->setSubSourceRect(layer->convertToNormalizedSourceRect(sourceRect));
        imageNode->setTexture(layer);
        imageNode->update();
    }

    void setupTextureContentNode(QSGTexture *texture, const QRect &rect, const QRectF &sourceRect,
                                 QSGImageNode::TextureCoordinatesTransformMode texCoordTransForm,
                                 QSGNode *) override
    {
        Q_ASSERT(m_nodeIterator != m_sceneGraphNodes->end());
        QSGImageNode *textureNode = static_cast<QSGImageNode*>(*m_nodeIterator++);
        if (textureNode->texture() != texture) {
            // Chromium sometimes uses textures that doesn't completely fit
            // in which case the geometry needs to be recalculated even if
            // rect and src-rect matches.
            if (textureNode->texture()->textureSize() != texture->textureSize())
                textureNode->markDirty(QSGImageNode::DirtyGeometry);
            textureNode->setTexture(texture);
        }
        if (textureNode->textureCoordinatesTransform() != texCoordTransForm)
            textureNode->setTextureCoordinatesTransform(texCoordTransForm);
        if (textureNode->rect() != rect)
            textureNode->setRect(rect);
        if (textureNode->sourceRect() != sourceRect)
            textureNode->setSourceRect(sourceRect);
        if (textureNode->filtering() != texture->filtering())
            textureNode->setFiltering(texture->filtering());
    }
    void setupSolidColorNode(const QRect &rect, const QColor &color, QSGNode *) override
    {
        Q_ASSERT(m_nodeIterator != m_sceneGraphNodes->end());
         QSGRectangleNode *rectangleNode = static_cast<QSGRectangleNode*>(*m_nodeIterator++);

         if (rectangleNode->rect() != rect)
             rectangleNode->setRect(rect);
         if (rectangleNode->color() != color)
             rectangleNode->setColor(color);
    }
#ifndef QT_NO_OPENGL
    void setupDebugBorderNode(QSGGeometry *geometry, QSGFlatColorMaterial *material,
                              QSGNode *) override
    {
        Q_ASSERT(m_nodeIterator != m_sceneGraphNodes->end());
        QSGGeometryNode *geometryNode = static_cast<QSGGeometryNode*>(*m_nodeIterator++);

        geometryNode->setGeometry(geometry);
        geometryNode->setMaterial(material);
    }

    void setupYUVVideoNode(QSGTexture *, QSGTexture *, QSGTexture *, QSGTexture *,
                           const QRectF &, const QRectF &, const QSizeF &, const QSizeF &,
                           gfx::ColorSpace, float, float, const QRectF &,
                           QSGNode *) override
    {
        Q_UNREACHABLE();
    }
#ifdef GL_OES_EGL_image_external
    void setupStreamVideoNode(MailboxTexture *, const QRectF &,
                              const QMatrix4x4 &, QSGNode *) override
    {
        Q_UNREACHABLE();
    }
#endif // GL_OES_EGL_image_external
#endif // QT_NO_OPENGL

private:
    QVector<QSGNode*>::iterator m_nodeIterator;
};

class DelegatedNodeTreeCreator : public DelegatedNodeTreeHandler
{
public:
    DelegatedNodeTreeCreator(QVector<QSGNode*> *sceneGraphNodes,
                             RenderWidgetHostViewQtDelegate *apiDelegate)
        : DelegatedNodeTreeHandler(sceneGraphNodes)
        , m_apiDelegate(apiDelegate)
    {
    }

    void setupRenderPassNode(QSGTexture *layer, const QRect &rect, const QRectF &sourceRect,
                             QSGNode *layerChain) override
    {
        Q_ASSERT(layer);
        // Only QSGInternalImageNode currently supports QSGLayer textures.
        QSGInternalImageNode *imageNode = m_apiDelegate->createInternalImageNode();
        imageNode->setTargetRect(rect);
        imageNode->setInnerTargetRect(rect);
        imageNode->setSubSourceRect(layer->convertToNormalizedSourceRect(sourceRect));
        imageNode->setTexture(layer);
        imageNode->update();

        layerChain->appendChildNode(imageNode);
        m_sceneGraphNodes->append(imageNode);
    }

    void setupTextureContentNode(QSGTexture *texture, const QRect &rect, const QRectF &sourceRect,
                                 QSGImageNode::TextureCoordinatesTransformMode texCoordTransForm,
                                 QSGNode *layerChain) override
    {
        QSGImageNode *textureNode = m_apiDelegate->createImageNode();
        textureNode->setTextureCoordinatesTransform(texCoordTransForm);
        textureNode->setRect(rect);
        textureNode->setSourceRect(sourceRect);
        textureNode->setTexture(texture);
        textureNode->setFiltering(texture->filtering());

        layerChain->appendChildNode(textureNode);
        m_sceneGraphNodes->append(textureNode);
    }

    void setupSolidColorNode(const QRect &rect, const QColor &color,
                             QSGNode *layerChain) override
    {
        QSGRectangleNode *rectangleNode = m_apiDelegate->createRectangleNode();
        rectangleNode->setRect(rect);
        rectangleNode->setColor(color);

        layerChain->appendChildNode(rectangleNode);
        m_sceneGraphNodes->append(rectangleNode);
    }

#ifndef QT_NO_OPENGL
    void setupDebugBorderNode(QSGGeometry *geometry, QSGFlatColorMaterial *material,
                              QSGNode *layerChain) override
    {
        QSGGeometryNode *geometryNode = new QSGGeometryNode;
        geometryNode->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial);

        geometryNode->setGeometry(geometry);
        geometryNode->setMaterial(material);

        layerChain->appendChildNode(geometryNode);
        m_sceneGraphNodes->append(geometryNode);
    }

    void setupYUVVideoNode(QSGTexture *yTexture, QSGTexture *uTexture, QSGTexture *vTexture,
                           QSGTexture *aTexture, const QRectF &yaTexCoordRect,
                           const QRectF &uvTexCoordRect, const QSizeF &yaTexSize,
                           const QSizeF &uvTexSize, gfx::ColorSpace colorspace,
                           float rMul, float rOff, const QRectF &rect,
                           QSGNode *layerChain) override
    {
        YUVVideoNode *videoNode = new YUVVideoNode(
                    yTexture,
                    uTexture,
                    vTexture,
                    aTexture,
                    yaTexCoordRect,
                    uvTexCoordRect,
                    yaTexSize,
                    uvTexSize,
                    colorspace,
                    rMul,
                    rOff);
        videoNode->setRect(rect);

        layerChain->appendChildNode(videoNode);
        m_sceneGraphNodes->append(videoNode);
    }
#ifdef GL_OES_EGL_image_external
    void setupStreamVideoNode(MailboxTexture *texture, const QRectF &rect,
                              const QMatrix4x4 &textureMatrix, QSGNode *layerChain) override
    {
        StreamVideoNode *svideoNode = new StreamVideoNode(texture, false, ExternalTarget);
        svideoNode->setRect(rect);
        svideoNode->setTextureMatrix(textureMatrix);
        layerChain->appendChildNode(svideoNode);
        m_sceneGraphNodes->append(svideoNode);
    }
#endif // GL_OES_EGL_image_external
#endif // QT_NO_OPENGL

private:
    RenderWidgetHostViewQtDelegate *m_apiDelegate;
};


static inline QSharedPointer<QSGLayer> findRenderPassLayer(const int &id, const QVector<QPair<int, QSharedPointer<QSGLayer> > > &list)
{
    typedef QPair<int, QSharedPointer<QSGLayer> > Pair;
    for (const Pair &pair : list)
        if (pair.first == id)
            return pair.second;
    return QSharedPointer<QSGLayer>();
}

static QSGNode *buildRenderPassChain(QSGNode *chainParent)
{
    // Chromium already ordered the quads from back to front for us, however the
    // Qt scene graph layers individual geometries in their own z-range and uses
    // the depth buffer to visually stack nodes according to their item tree order.

    // This gets rid of the z component of all quads, once any x and y perspective
    // transformation has been applied to vertices not on the z=0 plane. Qt will
    // use an orthographic projection to render them.
    QSGTransformNode *zCompressNode = new QSGTransformNode;
    QMatrix4x4 zCompressMatrix;
    zCompressMatrix.scale(1, 1, 0);
    zCompressNode->setMatrix(zCompressMatrix);
    chainParent->appendChildNode(zCompressNode);
    return zCompressNode;
}

static QSGNode *buildLayerChain(QSGNode *chainParent, const viz::SharedQuadState *layerState)
{
    QSGNode *layerChain = chainParent;
    if (layerState->is_clipped) {
        RectClipNode *clipNode = new RectClipNode(toQt(layerState->clip_rect));
        layerChain->appendChildNode(clipNode);
        layerChain = clipNode;
    }
    if (!layerState->quad_to_target_transform.IsIdentity()) {
        QSGTransformNode *transformNode = new QSGTransformNode;
        QMatrix4x4 qMatrix;
        convertToQt(layerState->quad_to_target_transform.matrix(), qMatrix);
        transformNode->setMatrix(qMatrix);
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

#ifndef QT_NO_OPENGL
MailboxTexture::MailboxTexture(const CompositorResource *resource, bool hasAlphaChannel, int target)
    : m_textureId(resource->texture_id)
    , m_fence(resource->texture_fence)
    , m_textureSize(toQt(resource->size))
    , m_hasAlpha(hasAlphaChannel)
    , m_target(target >= 0 ? target : GL_TEXTURE_2D)
#if defined(USE_OZONE)
    , m_ownsTexture(false)
#endif
{
    initializeOpenGLFunctions();

    // Assume that resources without a size will be used with a full source rect.
    // Setting a size of 1x1 will let any texture node compute a normalized source
    // rect of (0, 0) to (1, 1) while an empty texture size would set (0, 0) on all corners.
    if (m_textureSize.isEmpty())
        m_textureSize = QSize(1, 1);
}

MailboxTexture::~MailboxTexture()
{
#if defined(USE_OZONE)
   // This is rare case, where context is not shared
   // we created extra texture in current context, so
   // delete it now
   if (m_ownsTexture) {
       QOpenGLContext *currentContext = QOpenGLContext::currentContext() ;
       QOpenGLFunctions *funcs = currentContext->functions();
       GLuint id(m_textureId);
       funcs->glDeleteTextures(1, &id);
   }
#endif
}

void MailboxTexture::bind()
{
    if (m_fence)
        m_fence->wait();
    glBindTexture(m_target, m_textureId);
#ifdef Q_OS_QNX
    if (m_target == GL_TEXTURE_EXTERNAL_OES) {
        static bool resolved = false;
        static PFNEGLSTREAMCONSUMERACQUIREKHRPROC eglStreamConsumerAcquire = 0;

        if (!resolved) {
            QOpenGLContext *context = QOpenGLContext::currentContext();
            eglStreamConsumerAcquire = (PFNEGLSTREAMCONSUMERACQUIREKHRPROC)context->getProcAddress("eglStreamConsumerAcquireKHR");
            resolved = true;
        }
        if (eglStreamConsumerAcquire)
            eglStreamConsumerAcquire(m_eglStreamData.egl_display, m_eglStreamData.egl_str_handle);
    }
#endif
}
#endif // !QT_NO_OPENGL

RectClipNode::RectClipNode(const QRectF &rect)
    : m_geometry(QSGGeometry::defaultAttributes_Point2D(), 4)
{
    QSGGeometry::updateRectGeometry(&m_geometry, rect);
    setGeometry(&m_geometry);
    setClipRect(rect);
    setIsRectangular(true);
}

DelegatedFrameNode::DelegatedFrameNode()
#if defined(USE_OZONE) && !defined(QT_NO_OPENGL)
    : m_contextShared(true)
#endif
{
    setFlag(UsePreprocess);
#if defined(USE_OZONE) && !defined(QT_NO_OPENGL)
    QOpenGLContext *currentContext = QOpenGLContext::currentContext() ;
    QOpenGLContext *sharedContext = qt_gl_global_share_context();
    if (currentContext && sharedContext && !QOpenGLContext::areSharing(currentContext, sharedContext)) {
        static bool allowNotSharedContextWarningShown = true;
        if (allowNotSharedContextWarningShown) {
            allowNotSharedContextWarningShown = false;
            qWarning("Context is not shared, textures will be copied between contexts.");
        }
        m_offsurface.reset(new QOffscreenSurface);
        m_offsurface->create();
        m_contextShared = false;
    }
#endif
}

DelegatedFrameNode::~DelegatedFrameNode()
{
}

void DelegatedFrameNode::preprocess()
{
    // Then render any intermediate RenderPass in order.
    typedef QPair<int, QSharedPointer<QSGLayer> > Pair;
    for (const Pair &pair : qAsConst(m_sgObjects.renderPassLayers)) {
        // The layer is non-live, request a one-time update here.
        pair.second->scheduleUpdate();
        // Proceed with the actual update.
        pair.second->updateTexture();
    }
}

static bool areSharedQuadStatesEqual(const viz::SharedQuadState *layerState,
                                     const viz::SharedQuadState *prevLayerState)
{
    if (layerState->sorting_context_id != 0 || prevLayerState->sorting_context_id != 0)
        return false;
    if (layerState->is_clipped != prevLayerState->is_clipped
        || layerState->clip_rect != prevLayerState->clip_rect)
        return false;
    if (layerState->quad_to_target_transform != prevLayerState->quad_to_target_transform)
        return false;
    return qFuzzyCompare(layerState->opacity, prevLayerState->opacity);
}

// Compares if the frame data that we got from the Chromium Compositor is
// *structurally* equivalent to the one of the previous frame.
// If it is, we will just reuse and update the old nodes where necessary.
static bool areRenderPassStructuresEqual(const viz::CompositorFrame *frameData,
                                         const viz::CompositorFrame *previousFrameData)
{
    if (!previousFrameData)
        return false;

    if (previousFrameData->render_pass_list.size() != frameData->render_pass_list.size())
        return false;

    for (unsigned i = 0; i < frameData->render_pass_list.size(); ++i) {
        viz::RenderPass *newPass = frameData->render_pass_list.at(i).get();
        viz::RenderPass *prevPass = previousFrameData->render_pass_list.at(i).get();

        if (newPass->id != prevPass->id)
            return false;

        if (newPass->quad_list.size() != prevPass->quad_list.size())
            return false;

        viz::QuadList::ConstBackToFrontIterator it = newPass->quad_list.BackToFrontBegin();
        viz::QuadList::ConstBackToFrontIterator end = newPass->quad_list.BackToFrontEnd();
        viz::QuadList::ConstBackToFrontIterator prevIt = prevPass->quad_list.BackToFrontBegin();
        viz::QuadList::ConstBackToFrontIterator prevEnd = prevPass->quad_list.BackToFrontEnd();
        for (; it != end && prevIt != prevEnd; ++it, ++prevIt) {
            const viz::DrawQuad *quad = *it;
            const viz::DrawQuad *prevQuad = *prevIt;
            if (quad->material != prevQuad->material)
                return false;
#ifndef QT_NO_OPENGL
            if (quad->material == viz::DrawQuad::YUV_VIDEO_CONTENT)
                return false;
#ifdef GL_OES_EGL_image_external
            if (quad->material == viz::DrawQuad::STREAM_VIDEO_CONTENT)
                return false;
#endif // GL_OES_EGL_image_external
#endif // QT_NO_OPENGL
            if (!areSharedQuadStatesEqual(quad->shared_quad_state, prevQuad->shared_quad_state))
                return false;
            if (quad->shared_quad_state->is_clipped && quad->visible_rect != prevQuad->visible_rect) {
                gfx::Rect targetRect1 =
                        cc::MathUtil::MapEnclosingClippedRect(quad->shared_quad_state->quad_to_target_transform, quad->visible_rect);
                gfx::Rect targetRect2 =
                        cc::MathUtil::MapEnclosingClippedRect(quad->shared_quad_state->quad_to_target_transform, prevQuad->visible_rect);
                targetRect1.Intersect(quad->shared_quad_state->clip_rect);
                targetRect2.Intersect(quad->shared_quad_state->clip_rect);
                if (targetRect1.IsEmpty() != targetRect2.IsEmpty())
                    return false;
            }
        }
    }
    return true;
}

void DelegatedFrameNode::commit(const viz::CompositorFrame &pendingFrame,
                                const viz::CompositorFrame &committedFrame,
                                const CompositorResourceTracker *resourceTracker,
                                RenderWidgetHostViewQtDelegate *apiDelegate)
{
    const viz::CompositorFrame* frameData = &pendingFrame;
    if (frameData->render_pass_list.empty())
        return;

    // DelegatedFrameNode is a transform node only for the purpose of
    // countering the scale of devicePixel-scaled tiles when rendering them
    // to the final surface.
    QMatrix4x4 matrix;
    const float devicePixelRatio = frameData->metadata.device_scale_factor;
    matrix.scale(1 / devicePixelRatio, 1 / devicePixelRatio);
    if (QSGTransformNode::matrix() != matrix)
        setMatrix(matrix);

    QScopedPointer<DelegatedNodeTreeHandler> nodeHandler;

    const QSizeF viewportSizeInPt = apiDelegate->viewGeometry().size();
    const QSizeF viewportSizeF = viewportSizeInPt * devicePixelRatio;
    const QSize viewportSize(std::ceil(viewportSizeF.width()), std::ceil(viewportSizeF.height()));

    // We first compare if the render passes from the previous frame data are structurally
    // equivalent to the render passes in the current frame data. If they are, we are going
    // to reuse the old nodes. Otherwise, we will delete the old nodes and build a new tree.
    //
    // Additionally, because we clip (i.e. don't build scene graph nodes for) quads outside
    // of the visible area, we also have to rebuild the tree whenever the window is resized.
    const bool buildNewTree =
        !areRenderPassStructuresEqual(frameData, &committedFrame) ||
        m_sceneGraphNodes.empty() ||
        viewportSize != m_previousViewportSize;

    if (buildNewTree) {
        // Keep the old objects in scope to hold a ref on layers, resources and textures
        // that we can re-use. Destroy the remaining objects before returning.
        qSwap(m_sgObjects, m_previousSGObjects);
        // Discard the scene graph nodes from the previous frame.
        while (QSGNode *oldChain = firstChild())
            delete oldChain;
        m_sceneGraphNodes.clear();
        nodeHandler.reset(new DelegatedNodeTreeCreator(&m_sceneGraphNodes, apiDelegate));
    } else {
        qSwap(m_sgObjects.bitmapTextures, m_previousSGObjects.bitmapTextures);
        qSwap(m_sgObjects.mailboxTextures, m_previousSGObjects.mailboxTextures);
        nodeHandler.reset(new DelegatedNodeTreeUpdater(&m_sceneGraphNodes));
    }
    // The RenderPasses list is actually a tree where a parent RenderPass is connected
    // to its dependencies through a RenderPassId reference in one or more RenderPassQuads.
    // The list is already ordered with intermediate RenderPasses placed before their
    // parent, with the last one in the list being the root RenderPass, the one
    // that we displayed to the user.
    // All RenderPasses except the last one are rendered to an FBO.
    viz::RenderPass *rootRenderPass = frameData->render_pass_list.back().get();

    gfx::Rect viewportRect(toGfx(viewportSize));
    for (unsigned i = 0; i < frameData->render_pass_list.size(); ++i) {
        viz::RenderPass *pass = frameData->render_pass_list.at(i).get();

        QSGNode *renderPassParent = 0;
        gfx::Rect scissorRect;
        if (pass != rootRenderPass) {
            QSharedPointer<QSGLayer> rpLayer;
            if (buildNewTree) {
                rpLayer = findRenderPassLayer(pass->id, m_previousSGObjects.renderPassLayers);
                if (!rpLayer) {
                    rpLayer = QSharedPointer<QSGLayer>(apiDelegate->createLayer());
                    // Avoid any premature texture update since we need to wait
                    // for the GPU thread to produce the dependent resources first.
                    rpLayer->setLive(false);
                }
                QSharedPointer<QSGRootNode> rootNode(new QSGRootNode);
                rpLayer->setItem(rootNode.data());
                m_sgObjects.renderPassLayers.append(QPair<int,
                                                    QSharedPointer<QSGLayer> >(pass->id, rpLayer));
                m_sgObjects.renderPassRootNodes.append(rootNode);
                renderPassParent = rootNode.data();
            } else
                rpLayer = findRenderPassLayer(pass->id, m_sgObjects.renderPassLayers);

            rpLayer->setRect(toQt(pass->output_rect));
            rpLayer->setSize(toQt(pass->output_rect.size()));
            rpLayer->setFormat(pass->has_transparent_background ? GL_RGBA : GL_RGB);
            rpLayer->setHasMipmaps(pass->generate_mipmap);
            rpLayer->setMirrorVertical(true);
            scissorRect = pass->output_rect;
        } else {
            renderPassParent = this;
            scissorRect = viewportRect;
            scissorRect += rootRenderPass->output_rect.OffsetFromOrigin();
        }

        if (scissorRect.IsEmpty()) {
            holdResources(pass, resourceTracker);
            continue;
        }

        QSGNode *renderPassChain = nullptr;
        if (buildNewTree)
            renderPassChain = buildRenderPassChain(renderPassParent);

        base::circular_deque<std::unique_ptr<viz::DrawPolygon>> polygonQueue;
        int nextPolygonId = 0;
        int currentSortingContextId = 0;
        const viz::SharedQuadState *currentLayerState = nullptr;
        QSGNode *currentLayerChain = nullptr;
        const auto quadListBegin = pass->quad_list.BackToFrontBegin();
        const auto quadListEnd = pass->quad_list.BackToFrontEnd();
        for (auto it = quadListBegin; it != quadListEnd; ++it) {
            const viz::DrawQuad *quad = *it;
            const viz::SharedQuadState *quadState = quad->shared_quad_state;

            gfx::Rect targetRect =
                cc::MathUtil::MapEnclosingClippedRect(quadState->quad_to_target_transform,
                                                      quad->visible_rect);
            if (quadState->is_clipped)
                targetRect.Intersect(quadState->clip_rect);
            targetRect.Intersect(scissorRect);
            if (targetRect.IsEmpty()) {
                holdResources(quad, resourceTracker);
                continue;
            }

            if (quadState->sorting_context_id != currentSortingContextId) {
                flushPolygons(&polygonQueue, renderPassChain,
                              nodeHandler.data(), resourceTracker, apiDelegate);
                currentSortingContextId = quadState->sorting_context_id;
            }

            if (currentSortingContextId != 0) {
                std::unique_ptr<viz::DrawPolygon> polygon(
                    new viz::DrawPolygon(
                        quad,
                        gfx::RectF(quad->visible_rect),
                        quadState->quad_to_target_transform,
                        nextPolygonId++));
                if (polygon->points().size() > 2u)
                    polygonQueue.push_back(std::move(polygon));
                continue;
            }

            if (renderPassChain && currentLayerState != quadState) {
                currentLayerState = quadState;
                currentLayerChain = buildLayerChain(renderPassChain, quadState);
            }

            handleQuad(quad, currentLayerChain,
                       nodeHandler.data(), resourceTracker, apiDelegate);
        }
        flushPolygons(&polygonQueue, renderPassChain,
                      nodeHandler.data(), resourceTracker, apiDelegate);
    }

    copyMailboxTextures();

    m_previousViewportSize = viewportSize;
    m_previousSGObjects = SGObjects();
}

void DelegatedFrameNode::flushPolygons(
    base::circular_deque<std::unique_ptr<viz::DrawPolygon>> *polygonQueue,
    QSGNode *renderPassChain,
    DelegatedNodeTreeHandler *nodeHandler,
    const CompositorResourceTracker *resourceTracker,
    RenderWidgetHostViewQtDelegate *apiDelegate)
{
    if (polygonQueue->empty())
        return;

    const auto actionHandler = [&](viz::DrawPolygon *polygon) {
        const viz::DrawQuad *quad = polygon->original_ref();
        const viz::SharedQuadState *quadState = quad->shared_quad_state;

        QSGNode *currentLayerChain = nullptr;
        if (renderPassChain)
            currentLayerChain = buildLayerChain(renderPassChain, quad->shared_quad_state);

        gfx::Transform inverseTransform;
        bool invertible = quadState->quad_to_target_transform.GetInverse(&inverseTransform);
        DCHECK(invertible);
        polygon->TransformToLayerSpace(inverseTransform);

        handlePolygon(polygon, currentLayerChain,
                      nodeHandler, resourceTracker, apiDelegate);
    };

    viz::BspTree(polygonQueue).TraverseWithActionHandler(&actionHandler);
}

void DelegatedFrameNode::handlePolygon(
    const viz::DrawPolygon *polygon,
    QSGNode *currentLayerChain,
    DelegatedNodeTreeHandler *nodeHandler,
    const CompositorResourceTracker *resourceTracker,
    RenderWidgetHostViewQtDelegate *apiDelegate)
{
    const viz::DrawQuad *quad = polygon->original_ref();

    if (!polygon->is_split()) {
        handleQuad(quad, currentLayerChain,
                   nodeHandler, resourceTracker, apiDelegate);
    } else {
        std::vector<gfx::QuadF> clipRegionList;
        polygon->ToQuads2D(&clipRegionList);
        for (const auto & clipRegion : clipRegionList)
            handleClippedQuad(quad, clipRegion, currentLayerChain,
                              nodeHandler, resourceTracker, apiDelegate);
    }
}

void DelegatedFrameNode::handleClippedQuad(
    const viz::DrawQuad *quad,
    const gfx::QuadF &clipRegion,
    QSGNode *currentLayerChain,
    DelegatedNodeTreeHandler *nodeHandler,
    const CompositorResourceTracker *resourceTracker,
    RenderWidgetHostViewQtDelegate *apiDelegate)
{
    if (currentLayerChain) {
        auto clipGeometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 4);
        auto clipGeometryVertices = clipGeometry->vertexDataAsPoint2D();
        clipGeometryVertices[0].set(clipRegion.p1().x(), clipRegion.p1().y());
        clipGeometryVertices[1].set(clipRegion.p2().x(), clipRegion.p2().y());
        clipGeometryVertices[2].set(clipRegion.p4().x(), clipRegion.p4().y());
        clipGeometryVertices[3].set(clipRegion.p3().x(), clipRegion.p3().y());
        auto clipNode = new QSGClipNode;
        clipNode->setGeometry(clipGeometry);
        clipNode->setIsRectangular(false);
        clipNode->setFlag(QSGNode::OwnsGeometry);
        currentLayerChain->appendChildNode(clipNode);
        currentLayerChain = clipNode;
    }
    handleQuad(quad, currentLayerChain,
               nodeHandler, resourceTracker, apiDelegate);
}

void DelegatedFrameNode::handleQuad(
    const viz::DrawQuad *quad,
    QSGNode *currentLayerChain,
    DelegatedNodeTreeHandler *nodeHandler,
    const CompositorResourceTracker *resourceTracker,
    RenderWidgetHostViewQtDelegate *apiDelegate)
{
    switch (quad->material) {
    case viz::DrawQuad::RENDER_PASS: {
        const viz::RenderPassDrawQuad *renderPassQuad = viz::RenderPassDrawQuad::MaterialCast(quad);
        if (!renderPassQuad->mask_texture_size.IsEmpty()) {
            const CompositorResource *resource = findAndHoldResource(renderPassQuad->mask_resource_id(), resourceTracker);
            Q_UNUSED(resource); // FIXME: QTBUG-67652
        }
        QSGLayer *layer =
            findRenderPassLayer(renderPassQuad->render_pass_id, m_sgObjects.renderPassLayers).data();

        if (layer)
            nodeHandler->setupRenderPassNode(layer, toQt(quad->rect), toQt(renderPassQuad->tex_coord_rect), currentLayerChain);

        break;
    }
    case viz::DrawQuad::TEXTURE_CONTENT: {
        const viz::TextureDrawQuad *tquad = viz::TextureDrawQuad::MaterialCast(quad);
        const CompositorResource *resource = findAndHoldResource(tquad->resource_id(), resourceTracker);
        QSGTexture *texture =
            initAndHoldTexture(resource, quad->ShouldDrawWithBlending(), apiDelegate);
        QSizeF textureSize;
        if (texture)
            textureSize = texture->textureSize();
        gfx::RectF uv_rect =
            gfx::ScaleRect(gfx::BoundingRect(tquad->uv_top_left, tquad->uv_bottom_right),
                           textureSize.width(), textureSize.height());

        nodeHandler->setupTextureContentNode(
            texture, toQt(quad->rect), toQt(uv_rect),
            tquad->y_flipped ? QSGImageNode::MirrorVertically : QSGImageNode::NoTransform,
            currentLayerChain);
        break;
    }
    case viz::DrawQuad::SOLID_COLOR: {
        const viz::SolidColorDrawQuad *scquad = viz::SolidColorDrawQuad::MaterialCast(quad);
        // Qt only supports MSAA and this flag shouldn't be needed.
        // If we ever want to use QSGRectangleNode::setAntialiasing for this we should
        // try to see if we can do something similar for tile quads first.
        Q_UNUSED(scquad->force_anti_aliasing_off);
        nodeHandler->setupSolidColorNode(toQt(quad->rect), toQt(scquad->color), currentLayerChain);
        break;
#ifndef QT_NO_OPENGL
    }
    case viz::DrawQuad::DEBUG_BORDER: {
        const viz::DebugBorderDrawQuad *dbquad = viz::DebugBorderDrawQuad::MaterialCast(quad);

        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 4);
        geometry->setDrawingMode(GL_LINE_LOOP);
        geometry->setLineWidth(dbquad->width);
        // QSGGeometry::updateRectGeometry would actually set the
        // corners in the following order:
        // top-left, bottom-left, top-right, bottom-right, leading to a nice criss cross,
        // instead of having a closed loop.
        const gfx::Rect &r(dbquad->rect);
        geometry->vertexDataAsPoint2D()[0].set(r.x(), r.y());
        geometry->vertexDataAsPoint2D()[1].set(r.x() + r.width(), r.y());
        geometry->vertexDataAsPoint2D()[2].set(r.x() + r.width(), r.y() + r.height());
        geometry->vertexDataAsPoint2D()[3].set(r.x(), r.y() + r.height());

        QSGFlatColorMaterial *material = new QSGFlatColorMaterial;
        material->setColor(toQt(dbquad->color));

        nodeHandler->setupDebugBorderNode(geometry, material, currentLayerChain);
        break;
#endif
    }
    case viz::DrawQuad::TILED_CONTENT: {
        const viz::TileDrawQuad *tquad = viz::TileDrawQuad::MaterialCast(quad);
        const CompositorResource *resource = findAndHoldResource(tquad->resource_id(), resourceTracker);
        nodeHandler->setupTextureContentNode(
            initAndHoldTexture(resource, quad->ShouldDrawWithBlending(), apiDelegate),
            toQt(quad->rect), toQt(tquad->tex_coord_rect),
            QSGImageNode::NoTransform, currentLayerChain);
        break;
#ifndef QT_NO_OPENGL
    }
    case viz::DrawQuad::YUV_VIDEO_CONTENT: {
        const viz::YUVVideoDrawQuad *vquad = viz::YUVVideoDrawQuad::MaterialCast(quad);
        const CompositorResource *yResource =
            findAndHoldResource(vquad->y_plane_resource_id(), resourceTracker);
        const CompositorResource *uResource =
            findAndHoldResource(vquad->u_plane_resource_id(), resourceTracker);
        const CompositorResource *vResource =
            findAndHoldResource(vquad->v_plane_resource_id(), resourceTracker);
        const CompositorResource *aResource = nullptr;
        // This currently requires --enable-vp8-alpha-playback and
        // needs a video with alpha data to be triggered.
        if (vquad->a_plane_resource_id())
            aResource = findAndHoldResource(vquad->a_plane_resource_id(), resourceTracker);

        nodeHandler->setupYUVVideoNode(
            initAndHoldTexture(yResource, quad->ShouldDrawWithBlending()),
            initAndHoldTexture(uResource, quad->ShouldDrawWithBlending()),
            initAndHoldTexture(vResource, quad->ShouldDrawWithBlending()),
            aResource ? initAndHoldTexture(aResource, quad->ShouldDrawWithBlending()) : 0,
            toQt(vquad->ya_tex_coord_rect), toQt(vquad->uv_tex_coord_rect),
            toQt(vquad->ya_tex_size), toQt(vquad->uv_tex_size), vquad->video_color_space,
            vquad->resource_multiplier, vquad->resource_offset, toQt(quad->rect),
            currentLayerChain);
        break;
#ifdef GL_OES_EGL_image_external
    }
    case viz::DrawQuad::STREAM_VIDEO_CONTENT: {
        const viz::StreamVideoDrawQuad *squad = viz::StreamVideoDrawQuad::MaterialCast(quad);
        const CompositorResource *resource = findAndHoldResource(squad->resource_id(), resourceTracker);
        MailboxTexture *texture = static_cast<MailboxTexture *>(
            initAndHoldTexture(resource, quad->ShouldDrawWithBlending(), apiDelegate, GL_TEXTURE_EXTERNAL_OES));

        QMatrix4x4 qMatrix;
        convertToQt(squad->matrix.matrix(), qMatrix);
        nodeHandler->setupStreamVideoNode(texture, toQt(squad->rect), qMatrix, currentLayerChain);
        break;
#endif // GL_OES_EGL_image_external
#endif // QT_NO_OPENGL
    }
    case viz::DrawQuad::SURFACE_CONTENT:
        Q_UNREACHABLE();
    default:
        qWarning("Unimplemented quad material: %d", quad->material);
    }
}

const CompositorResource *DelegatedFrameNode::findAndHoldResource(unsigned resourceId, const CompositorResourceTracker *resourceTracker)
{
    return resourceTracker->findResource(resourceId);
}

void DelegatedFrameNode::holdResources(const viz::DrawQuad *quad, const CompositorResourceTracker *resourceTracker)
{
    for (auto resource : quad->resources)
        findAndHoldResource(resource, resourceTracker);
}

void DelegatedFrameNode::holdResources(const viz::RenderPass *pass, const CompositorResourceTracker *resourceTracker)
{
    for (const auto &quad : pass->quad_list)
        holdResources(quad, resourceTracker);
}

template<class Container, class Key>
inline auto &findTexture(Container &map, Container &previousMap, const Key &key)
{
    auto &value = map[key];
    if (value)
        return value;
    value = previousMap[key];
    return value;
}

QSGTexture *DelegatedFrameNode::initAndHoldTexture(const CompositorResource *resource, bool hasAlphaChannel, RenderWidgetHostViewQtDelegate *apiDelegate, int target)
{
    QSGTexture::Filtering filtering;

    if (resource->filter == GL_NEAREST)
        filtering = QSGTexture::Nearest;
    else if (resource->filter == GL_LINEAR)
        filtering = QSGTexture::Linear;
    else {
        // Depends on qtdeclarative fix, see QTBUG-71322
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 1)
        filtering = QSGTexture::Linear;
#else
        filtering = QSGTexture::Nearest;
#endif
    }

    if (resource->is_software) {
        QSharedPointer<QSGTexture> &texture =
            findTexture(m_sgObjects.bitmapTextures, m_previousSGObjects.bitmapTextures, resource->id);
        if (texture)
            return texture.data();
        texture = createBitmapTexture(resource, hasAlphaChannel, apiDelegate);
        texture->setFiltering(filtering);
        return texture.data();
    } else {
#if QT_CONFIG(opengl)
        QSharedPointer<MailboxTexture> &texture =
            findTexture(m_sgObjects.mailboxTextures, m_previousSGObjects.mailboxTextures, resource->id);
        if (texture)
            return texture.data();
        texture = createMailboxTexture(resource, hasAlphaChannel, target);
        texture->setFiltering(filtering);
        return texture.data();
#else
        Q_UNREACHABLE();
        return nullptr;
#endif
    }
}

QSharedPointer<QSGTexture> DelegatedFrameNode::createBitmapTexture(const CompositorResource *resource, bool hasAlphaChannel, RenderWidgetHostViewQtDelegate *apiDelegate)
{
    Q_ASSERT(apiDelegate);
    viz::SharedBitmap *sharedBitmap = resource->bitmap.get();
    gfx::Size size = resource->size;

    // QSG interprets QImage::hasAlphaChannel meaning that a node should enable blending
    // to draw it but Chromium keeps this information in the quads.
    // The input format is currently always Format_ARGB32_Premultiplied, so assume that all
    // alpha bytes are 0xff if quads aren't requesting blending and avoid the conversion
    // from Format_ARGB32_Premultiplied to Format_RGB32 just to get hasAlphaChannel to
    // return false.
    QImage::Format format = hasAlphaChannel ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32;
    QImage image = sharedBitmap
        ? QImage(sharedBitmap->pixels(), size.width(), size.height(), format)
        : QImage(size.width(), size.height(), format);
    return QSharedPointer<QSGTexture>(apiDelegate->createTextureFromImage(image.copy()));
}

QSharedPointer<MailboxTexture> DelegatedFrameNode::createMailboxTexture(const CompositorResource *resource, bool hasAlphaChannel, int target)
{
#ifndef QT_NO_OPENGL
    return QSharedPointer<MailboxTexture>::create(resource, hasAlphaChannel, target);
#else
    Q_UNREACHABLE();
#endif
}

void DelegatedFrameNode::copyMailboxTextures()
{
#if !defined(QT_NO_OPENGL) && defined(USE_OZONE)
    // Workaround when context is not shared QTBUG-48969
    // Make slow copy between two contexts.
    if (!m_contextShared) {
        QOpenGLContext *currentContext = QOpenGLContext::currentContext() ;
        QOpenGLContext *sharedContext = qt_gl_global_share_context();

        QSurface *surface = currentContext->surface();
        Q_ASSERT(m_offsurface);
        sharedContext->makeCurrent(m_offsurface.data());
        QOpenGLFunctions *funcs = sharedContext->functions();

        GLuint fbo = 0;
        funcs->glGenFramebuffers(1, &fbo);

        for (const QSharedPointer<MailboxTexture> &mailboxTexture : qAsConst(m_sgObjects.mailboxTextures)) {
            if (mailboxTexture->m_ownsTexture)
                continue;

            // Read texture into QImage from shared context.
            // Switch to shared context.
            sharedContext->makeCurrent(m_offsurface.data());
            funcs = sharedContext->functions();
            QImage img(mailboxTexture->textureSize(), QImage::Format_RGBA8888_Premultiplied);
            funcs->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            mailboxTexture->m_fence->wait();
            funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mailboxTexture->m_textureId, 0);
            GLenum status = funcs->glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE) {
                qWarning("fbo error, skipping slow copy...");
                continue;
            }
            funcs->glReadPixels(0, 0, mailboxTexture->textureSize().width(), mailboxTexture->textureSize().height(),
                                GL_RGBA, GL_UNSIGNED_BYTE, img.bits());

            // Restore current context.
            // Create texture from QImage in current context.
            currentContext->makeCurrent(surface);
            GLuint texture = 0;
            funcs = currentContext->functions();
            funcs->glGenTextures(1, &texture);
            funcs->glBindTexture(GL_TEXTURE_2D, texture);
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            funcs->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mailboxTexture->textureSize().width(), mailboxTexture->textureSize().height(), 0,
                                GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
            mailboxTexture->m_textureId = texture;
            mailboxTexture->m_ownsTexture = true;
        }
        // Cleanup allocated resources
        sharedContext->makeCurrent(m_offsurface.data());
        funcs = sharedContext->functions();
        funcs->glBindFramebuffer(GL_FRAMEBUFFER, 0);
        funcs->glDeleteFramebuffers(1, &fbo);
        currentContext->makeCurrent(surface);
    }
#endif
}

} // namespace QtWebEngineCore
