/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#include "compositor.h"

#include "base/memory/ref_counted.h"
#include "components/viz/common/surfaces/frame_sink_id.h"

#include <QHash>
#include <QImage>
#include <QMutex>

namespace QtWebEngineCore {

// Compositor::Id

Compositor::Id::Id(viz::FrameSinkId fid) : client_id(fid.client_id()), sink_id(fid.sink_id()) { }

static size_t qHash(Compositor::Id id, size_t seed = 0)
{
    QtPrivate::QHashCombine hasher;
    seed = hasher(seed, id.client_id);
    seed = hasher(seed, id.sink_id);
    return seed;
}

static bool operator==(Compositor::Id id1, Compositor::Id id2)
{
    return id1.client_id == id2.client_id && id1.sink_id == id2.sink_id;
}

// Compositor::Binding and Compositor::Bindings

struct Compositor::Binding
{
    const Id id;
    Compositor *compositor = nullptr;
    Observer *observer = nullptr;

    Binding(Id id) : id(id) { }
    ~Binding();
};

class Compositor::BindingMap
{
public:
    void lock() { m_mutex.lock(); }

    void unlock() { m_mutex.unlock(); }

    Binding *findOrCreate(Id id)
    {
        auto it = m_map.find(id);
        if (it == m_map.end())
            it = m_map.insert(id, new Binding(id));
        return *it;
    }

    void remove(Id id) { m_map.remove(id); }

private:
    QMutex m_mutex;
    QHash<Id, Binding *> m_map;
} static g_bindings;

Compositor::Binding::~Binding()
{
    g_bindings.remove(id);
}

// Compositor::Observer

void Compositor::Observer::bind(Id id)
{
    DCHECK(!m_binding);
    g_bindings.lock();
    m_binding = g_bindings.findOrCreate(id);
    DCHECK(!m_binding->observer);
    m_binding->observer = this;
    g_bindings.unlock();
}

void Compositor::Observer::unbind()
{
    DCHECK(m_binding);
    g_bindings.lock();
    m_binding->observer = nullptr;
    if (m_binding->compositor == nullptr)
        delete m_binding;
    m_binding = nullptr;
    g_bindings.unlock();
}

Compositor::Handle<Compositor> Compositor::Observer::compositor()
{
    if (!m_binding)
        return nullptr;
    g_bindings.lock();
    if (m_binding->compositor)
        return m_binding->compositor; // delay unlock
    g_bindings.unlock();
    return nullptr;
}

// Compositor

void Compositor::bind(Id id)
{
    DCHECK(!m_binding);
    g_bindings.lock();
    m_binding = g_bindings.findOrCreate(id);
    DCHECK(!m_binding->compositor);
    m_binding->compositor = this;
    g_bindings.unlock();
}

void Compositor::unbind()
{
    DCHECK(m_binding);
    g_bindings.lock();
    m_binding->compositor = nullptr;
    if (m_binding->observer == nullptr)
        delete m_binding;
    m_binding = nullptr;
    g_bindings.unlock();
}

Compositor::Handle<Compositor::Observer> Compositor::observer()
{
    if (!m_binding)
        return nullptr;
    g_bindings.lock();
    if (m_binding->observer)
        return m_binding->observer; // delay unlock
    g_bindings.unlock();
    return nullptr;
}

QImage Compositor::image()
{
    Q_UNREACHABLE();
    return {};
}

void Compositor::waitForTexture()
{
    Q_UNREACHABLE();
}

int Compositor::textureId()
{
    Q_UNREACHABLE();
    return 0;
}

// static
void Compositor::unlockBindings()
{
    g_bindings.unlock();
}
} // namespace QtWebEngineCore
