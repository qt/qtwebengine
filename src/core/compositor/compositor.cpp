// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "compositor.h"

#include "base/memory/ref_counted.h"
#include "components/viz/common/surfaces/frame_sink_id.h"

#include <QHash>
#include <QMutex>
#include <QQuickWindow>

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
    g_bindings.lock();
    if (m_binding && m_binding->compositor)
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
    g_bindings.lock();
    if (m_binding && m_binding->observer)
        return m_binding->observer; // delay unlock
    g_bindings.unlock();
    return nullptr;
}

void Compositor::waitForTexture()
{
}

void Compositor::releaseTexture()
{
}

QSGTexture *Compositor::texture(QQuickWindow *, uint32_t textureOptions)
{
    Q_UNREACHABLE();
    return nullptr;
}

bool Compositor::textureIsFlipped()
{
    Q_UNREACHABLE();
    return false;
}

void Compositor::releaseResources(QQuickWindow *)
{
    Q_UNREACHABLE();
}

// static
void Compositor::unlockBindings()
{
    g_bindings.unlock();
}
} // namespace QtWebEngineCore
