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

#ifndef DELEGATED_FRAME_NODE_H
#define DELEGATED_FRAME_NODE_H

#include "base/memory/scoped_ptr.h"
#include "cc/resources/transferable_resource.h"
#include <QMutex>
#include <QSGNode>
#include <QSharedData>
#include <QSharedPointer>
#include <QWaitCondition>

QT_BEGIN_NAMESPACE
class QSGRenderContext;
QT_END_NAMESPACE

namespace cc {
class DelegatedFrameData;
}

class MailboxTexture;
class RenderPassTexture;

// Separating this data allows another DelegatedFrameNode to reconstruct the QSGNode tree from the mailbox textures
// and render pass information.
class DelegatedFrameNodeData : public QSharedData {
public:
    DelegatedFrameNodeData() : frameDevicePixelRatio(1) { }
    QHash<unsigned, QSharedPointer<MailboxTexture> > mailboxTextures;
    scoped_ptr<cc::DelegatedFrameData> frameData;
    qreal frameDevicePixelRatio;
};

class DelegatedFrameNode : public QSGTransformNode {
public:
    DelegatedFrameNode(QSGRenderContext *sgRenderContext);
    ~DelegatedFrameNode();
    void preprocess();
    void commit(DelegatedFrameNodeData* data, cc::ReturnedResourceArray *resourcesToRelease);

private:
    QExplicitlySharedDataPointer<DelegatedFrameNodeData> m_data;
    QSGRenderContext *m_sgRenderContext;
    QList<QSharedPointer<RenderPassTexture> > m_renderPassTextures;
    int m_numPendingSyncPoints;
    QWaitCondition m_mailboxesFetchedWaitCond;
    QMutex m_mutex;

    // Making those callbacks static bypasses base::Bind's ref-counting requirement
    // of the this pointer when the callback is a method.
    static void fetchTexturesAndUnlockQt(DelegatedFrameNode *frameNode, QList<MailboxTexture *> *mailboxesToFetch);
    static void syncPointRetired(DelegatedFrameNode *frameNode, QList<MailboxTexture *> *mailboxesToFetch);
};

#endif // DELEGATED_FRAME_NODE_H
