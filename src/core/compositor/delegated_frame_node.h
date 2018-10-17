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

#ifndef DELEGATED_FRAME_NODE_H
#define DELEGATED_FRAME_NODE_H

#include "base/containers/circular_deque.h"
#include "components/viz/common/quads/compositor_frame.h"
#include "components/viz/common/quads/render_pass.h"

#include <QtCore/QSharedPointer>
#include <QtGui/QOffscreenSurface>
#include <QtQuick/QSGTransformNode>

#include "chromium_gpu_helper.h"
#include "render_widget_host_view_qt_delegate.h"

QT_BEGIN_NAMESPACE
class QSGLayer;
QT_END_NAMESPACE

namespace gfx {
class QuadF;
}

namespace viz {
class DelegatedFrameData;
class DrawQuad;
class DrawPolygon;
}

namespace QtWebEngineCore {

class CompositorResource;
class CompositorResourceTracker;
class DelegatedNodeTreeHandler;
class MailboxTexture;

class DelegatedFrameNode : public QSGTransformNode {
public:
    DelegatedFrameNode();
    ~DelegatedFrameNode();
    void preprocess() override;
    void commit(const viz::CompositorFrame &pendingFrame, const viz::CompositorFrame &committedFrame, const CompositorResourceTracker *resourceTracker, RenderWidgetHostViewQtDelegate *apiDelegate);

private:
    void flushPolygons(base::circular_deque<std::unique_ptr<viz::DrawPolygon> > *polygonQueue,
        QSGNode *renderPassChain,
        DelegatedNodeTreeHandler *nodeHandler,
        const CompositorResourceTracker *resourceTracker,
        RenderWidgetHostViewQtDelegate *apiDelegate);
    void handlePolygon(
        const viz::DrawPolygon *polygon,
        QSGNode *currentLayerChain,
        DelegatedNodeTreeHandler *nodeHandler,
        const CompositorResourceTracker *resourceTracker,
        RenderWidgetHostViewQtDelegate *apiDelegate);
    void handleClippedQuad(
        const viz::DrawQuad *quad,
        const gfx::QuadF &clipRegion,
        QSGNode *currentLayerChain,
        DelegatedNodeTreeHandler *nodeHandler,
        const CompositorResourceTracker *resourceTracker,
        RenderWidgetHostViewQtDelegate *apiDelegate);
    void handleQuad(
        const viz::DrawQuad *quad,
        QSGNode *currentLayerChain,
        DelegatedNodeTreeHandler *nodeHandler,
        const CompositorResourceTracker *resourceTracker,
        RenderWidgetHostViewQtDelegate *apiDelegate);

    const CompositorResource *findAndHoldResource(unsigned resourceId, const CompositorResourceTracker *resourceTracker);
    void holdResources(const viz::DrawQuad *quad, const CompositorResourceTracker *resourceTracker);
    void holdResources(const viz::RenderPass *pass, const CompositorResourceTracker *resourceTracker);
    QSGTexture *initAndHoldTexture(const CompositorResource *resource, bool hasAlphaChannel, RenderWidgetHostViewQtDelegate *apiDelegate = 0, int target = -1);
    QSharedPointer<QSGTexture> createBitmapTexture(const CompositorResource *resource, bool hasAlphaChannel, RenderWidgetHostViewQtDelegate *apiDelegate);
    QSharedPointer<MailboxTexture> createMailboxTexture(const CompositorResource *resource, bool hasAlphaChannel, int target);

    void copyMailboxTextures();

    struct SGObjects {
        QVector<QPair<int, QSharedPointer<QSGLayer> > > renderPassLayers;
        QVector<QSharedPointer<QSGRootNode> > renderPassRootNodes;
        QHash<unsigned, QSharedPointer<QSGTexture> > bitmapTextures;
        QHash<unsigned, QSharedPointer<MailboxTexture> > mailboxTextures;
    } m_sgObjects, m_previousSGObjects;
    QVector<QSGNode*> m_sceneGraphNodes;
#if defined(USE_OZONE)
    bool m_contextShared;
    QScopedPointer<QOffscreenSurface> m_offsurface;
#endif
    QSize m_previousViewportSize;
};

} // namespace QtWebEngineCore

#endif // DELEGATED_FRAME_NODE_H
