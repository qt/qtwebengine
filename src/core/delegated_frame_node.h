/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef DELEGATED_FRAME_NODE_H
#define DELEGATED_FRAME_NODE_H

#include "base/memory/scoped_ptr.h"
#include "cc/quads/render_pass.h"
#include "cc/resources/transferable_resource.h"
#include <QMutex>
#include <QSGNode>
#include <QSharedData>
#include <QSharedPointer>
#include <QWaitCondition>

#include "chromium_gpu_helper.h"
#include "render_widget_host_view_qt_delegate.h"

QT_BEGIN_NAMESPACE
class QSGLayer;
QT_END_NAMESPACE

namespace cc {
class DelegatedFrameData;
}

namespace QtWebEngineCore {

class MailboxTexture;
class ResourceHolder;

// Separating this data allows another DelegatedFrameNode to reconstruct the QSGNode tree from the mailbox textures
// and render pass information.
class ChromiumCompositorData : public QSharedData {
public:
    ChromiumCompositorData() : frameDevicePixelRatio(1) { }
    QHash<unsigned, QSharedPointer<ResourceHolder> > resourceHolders;
    scoped_ptr<cc::DelegatedFrameData> frameData;
    qreal frameDevicePixelRatio;
};

class DelegatedFrameNode : public QSGTransformNode {
public:
    DelegatedFrameNode();
    ~DelegatedFrameNode();
    void preprocess();
    void commit(ChromiumCompositorData *chromiumCompositorData, cc::ReturnedResourceArray *resourcesToRelease, RenderWidgetHostViewQtDelegate *apiDelegate);

private:
    // Making those callbacks static bypasses base::Bind's ref-counting requirement
    // of the this pointer when the callback is a method.
    static void fetchTexturesAndUnlockQt(DelegatedFrameNode *frameNode, QList<MailboxTexture *> *mailboxesToFetch);
    static void syncPointRetired(DelegatedFrameNode *frameNode, QList<MailboxTexture *> *mailboxesToFetch);

    ResourceHolder *findAndHoldResource(unsigned resourceId, QHash<unsigned, QSharedPointer<ResourceHolder> > &candidates);
    QSGTexture *initAndHoldTexture(ResourceHolder *resource, bool quadIsAllOpaque, RenderWidgetHostViewQtDelegate *apiDelegate = 0);

    QExplicitlySharedDataPointer<ChromiumCompositorData> m_chromiumCompositorData;
    struct SGObjects {
        QList<QPair<cc::RenderPassId, QSharedPointer<QSGLayer> > > renderPassLayers;
        QList<QSharedPointer<QSGRootNode> > renderPassRootNodes;
        QList<QSharedPointer<QSGTexture> > textureStrongRefs;
    } m_sgObjects;
    int m_numPendingSyncPoints;
    QMap<uint32, gfx::TransferableFence> m_mailboxGLFences;
    QWaitCondition m_mailboxesFetchedWaitCond;
    QMutex m_mutex;
};

} // namespace QtWebEngineCore

#endif // DELEGATED_FRAME_NODE_H
