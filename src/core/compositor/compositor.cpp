// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "compositor.h"

#include "base/memory/ref_counted.h"
#include "components/viz/common/surfaces/frame_sink_id.h"

#include <QGuiApplication>
#include <QHash>
#include <QReadWriteLock>
#include <QQuickWindow>

namespace QtWebEngineCore {

Q_LOGGING_CATEGORY(lcWebEngineCompositor, "qt.webengine.compositor");

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
    void lock() { m_mutex.lockForRead(); }
    void lockForWrite() { m_mutex.lockForWrite(); }

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
    QReadWriteLock m_mutex;
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
    g_bindings.lockForWrite();
    m_binding = g_bindings.findOrCreate(id);
    DCHECK(!m_binding->observer);
    m_binding->observer = this;
    g_bindings.unlock();
}

void Compositor::Observer::unbind()
{
    g_bindings.lockForWrite();
    if (m_binding) {
        m_binding->observer = nullptr;
        if (m_binding->compositor == nullptr)
            delete m_binding;
        m_binding = nullptr;
    }
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

Compositor::Observer::~Observer()
{
    DCHECK(!m_binding); // check that unbind() was called by derived final class
}

// Compositor

void Compositor::bind(Id id)
{
    DCHECK(!m_binding);
    g_bindings.lockForWrite();
    m_binding = g_bindings.findOrCreate(id);
    DCHECK(!m_binding->compositor);
    m_binding->compositor = this;
    g_bindings.unlock();
}

void Compositor::unbind()
{
    g_bindings.lockForWrite();
    if (m_binding) {
        m_binding->compositor = nullptr;
        if (m_binding->observer == nullptr)
            delete m_binding;
        m_binding = nullptr;
    }
    g_bindings.unlock();
}

void Compositor::readyToSwap()
{
    g_bindings.lock();
    if (m_binding && m_binding->observer)
        m_binding->observer->readyToSwap();
    g_bindings.unlock();
}

void Compositor::waitForTexture()
{
}

void Compositor::releaseTexture()
{
}

QSGTexture *Compositor::texture(QQuickWindow *, uint32_t textureOptions)
{
    Q_UNREACHABLE_RETURN(nullptr);
}

bool Compositor::textureIsFlipped()
{
    Q_UNREACHABLE_RETURN(false);
}

void Compositor::releaseResources() { }

Compositor::Compositor(Type type) : m_type(type)
{
    qCDebug(lcWebEngineCompositor, "Compositor Type: %s",
            m_type == Type::Software ? "Software" : "Native");
    qCDebug(lcWebEngineCompositor, "QPA Platform Plugin: %ls",
            qUtf16Printable(QGuiApplication::platformName()));
}

Compositor::~Compositor()
{
    DCHECK(!m_binding); // check that unbind() was called by derived final class
}

// static
void Compositor::unlockBindings()
{
    g_bindings.unlock();
}
} // namespace QtWebEngineCore
