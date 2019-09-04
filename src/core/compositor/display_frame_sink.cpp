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

#include "display_frame_sink.h"

#include <QMap>

namespace QtWebEngineCore {

namespace {

class DisplayFrameSinkMap
{
public:
    static DisplayFrameSinkMap *instance()
    {
        static DisplayFrameSinkMap map;
        return &map;
    }

    scoped_refptr<DisplayFrameSink> findOrCreate(viz::FrameSinkId frameSinkId)
    {
        QMutexLocker locker(&m_mutex);
        auto it = m_map.find(frameSinkId);
        if (it == m_map.end())
            it = m_map.insert(frameSinkId, new DisplayFrameSink(frameSinkId));
        return *it;
    }

    void remove(viz::FrameSinkId frameSinkId)
    {
        QMutexLocker locker(&m_mutex);
        m_map.remove(frameSinkId);
    }

private:
    mutable QMutex m_mutex;
    QMap<viz::FrameSinkId, DisplayFrameSink *> m_map;
};

} // namespace

// static
scoped_refptr<DisplayFrameSink> DisplayFrameSink::findOrCreate(viz::FrameSinkId frameSinkId)
{
    return DisplayFrameSinkMap::instance()->findOrCreate(frameSinkId);
}

DisplayFrameSink::DisplayFrameSink(viz::FrameSinkId frameSinkId)
    : m_frameSinkId(frameSinkId)
{
    DCHECK(m_frameSinkId.is_valid());
}

DisplayFrameSink::~DisplayFrameSink()
{
    DisplayFrameSinkMap::instance()->remove(m_frameSinkId);
}

void DisplayFrameSink::connect(DisplayConsumer *consumer)
{
    QMutexLocker locker(&m_mutex);
    DCHECK(m_consumer == nullptr);
    m_consumer = consumer;
}

void DisplayFrameSink::connect(DisplayProducer *producer)
{
    QMutexLocker locker(&m_mutex);
    DCHECK(m_producer == nullptr);
    m_producer = producer;
}

void DisplayFrameSink::disconnect(DisplayConsumer *consumer)
{
    QMutexLocker locker(&m_mutex);
    DCHECK(m_consumer == consumer);
    m_consumer = nullptr;
}

void DisplayFrameSink::disconnect(DisplayProducer *producer)
{
    QMutexLocker locker(&m_mutex);
    DCHECK(m_producer == producer);
    m_producer = nullptr;
}

void DisplayFrameSink::scheduleUpdate()
{
    QMutexLocker locker(&m_mutex);
    if (m_consumer)
        m_consumer->scheduleUpdate();
}

QSGNode *DisplayFrameSink::updatePaintNode(QSGNode *oldNode, RenderWidgetHostViewQtDelegate *delegate)
{
    QMutexLocker locker(&m_mutex);
    QSGNode *newNode = oldNode;
    if (m_producer)
        newNode = m_producer->updatePaintNode(oldNode, delegate);
    return newNode;
}

} // namespace QtWebEngineCore
