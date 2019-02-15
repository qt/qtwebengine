/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#ifndef COMPOSITOR_RESOURCE_H
#define COMPOSITOR_RESOURCE_H

#include <base/memory/ref_counted.h>
#include <components/viz/common/resources/transferable_resource.h>

#include <QtCore/qglobal.h>
#include <QtGui/qtgui-config.h>

#if QT_CONFIG(opengl)
# include "compositor_resource_fence.h"
#endif

namespace viz {
class SharedBitmap;
} // namespace viz

namespace QtWebEngineCore {

using CompositorResourceId = quint32;

// A resource (OpenGL texture or software shared bitmap).
//
// - Created by the CompositorResourceTracker from a newly submitted
//   CompositorFrame's resource_list.
//
// - Until the frame is committed, its resources are in a 'pending' state and
//   are inaccessible from outside the CompositorResourceTracker.
//
// - Once the frame is committed, its resources can be found via
//   CompositorResourceTracker::findResource.
//
// - A committed resource's fields may not be updated and are safe to use from
//   other threads without synchronization (unless noted otherwise).
class CompositorResource : public viz::TransferableResource
{
public:
    CompositorResource(const viz::TransferableResource &tr) : viz::TransferableResource(tr) {}

    // Counts the number of times this resource has been encountered in
    // CompositorFrames' resource lists.
    //
    // Corresponds to viz::ReturnedResource::count.
    //
    // Updated by CompositorResourceTracker on UI thread.
    int import_count = 1;

    // Identifies the last frame that needed this resource. Used by
    // CompositorResourceTracker to return unused resources back to child
    // compositors.
    //
    // Updated by CompositorResourceTracker on UI thread.
    quint32 last_used_for_frame = 0;

    // Bitmap (if is_software).
    std::unique_ptr<viz::SharedBitmap> bitmap;

#if QT_CONFIG(opengl)
    // OpenGL texture id (if !is_software).
    quint32 texture_id = 0;

    // Should be waited on before using the texture (non-null if !is_software).
    scoped_refptr<CompositorResourceFence> texture_fence;
#endif // QT_CONFIG(opengl)
};

inline bool operator<(const CompositorResource &r1, const CompositorResource &r2)
{
    return r1.id < r2.id;
}

inline bool operator<(const CompositorResource &r, CompositorResourceId id)
{
    return r.id < id;
}

inline bool operator<(CompositorResourceId id, const CompositorResource &r)
{
    return id < r.id;
}

} // namespace QtWebEngineCore

#endif // !COMPOSITOR_RESOURCE_H
