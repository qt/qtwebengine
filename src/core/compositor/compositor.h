// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>

QT_BEGIN_NAMESPACE
class QQuickWindow;
class QSize;
class QSGTexture;
QT_END_NAMESPACE

namespace viz {
class FrameSinkId;
} // namespace viz

namespace QtWebEngineCore {

// Produces composited frames for display.
//
// Used by quick/widgets libraries for accessing the frames and
// controlling frame swapping.
class Q_WEBENGINECORE_PRIVATE_EXPORT Compositor
{
    struct Binding;

public:
    // Identifies the implementation type.
    enum class Type {
        Software,
        OpenGL,
        Vulkan,
        NativeBuffer
    };

    // Identifies a compositor.
    //
    // The purpose of assigning ids to compositors is to allow the
    // corresponding observer to be registered before the compositor
    // itself is created, which is necessary since the creation
    // happens on a different thread in the depths of viz.
    //
    // (Maps to viz::FrameSinkId internally).
    struct Id
    {
        quint32 client_id;
        quint32 sink_id;

        Id(viz::FrameSinkId);
    };

    // Pointer to Compositor or Observer that holds a lock to prevent
    // either from being unbound and destroyed.
    template<typename T>
    class Handle
    {
    public:
        Handle(std::nullptr_t) : m_data(nullptr) { }
        Handle(T *data) : m_data(data) { }
        Handle(Handle &&that) : m_data(that.m_data) { that.m_data = nullptr; }
        ~Handle()
        {
            if (m_data)
                Compositor::unlockBindings();
        }
        T *operator->() const { return m_data; }
        T &operator*() const { return *m_data; }
        explicit operator bool() const { return m_data; }

    private:
        T *m_data;
    };

    // Observes the compositor corresponding to the given id.
    //
    // Only one observer can exist per compositor.
    class Q_WEBENGINECORE_PRIVATE_EXPORT Observer
    {
    public:
        // Binding to compositor
        void bind(Id id);
        void unbind();

        // Compositor if bound
        Handle<Compositor> compositor();

        // There's a new frame ready, time to swapFrame().
        virtual void readyToSwap() = 0;

    protected:
        Observer() = default;
        ~Observer() { if (m_binding) unbind(); }

    private:
        Binding *m_binding = nullptr;
    };

    // Type determines which methods can be called.
    Type type() const { return m_type; }

    // Binding to observer.
    void bind(Id id);
    void unbind();

    // Observer if bound.
    Handle<Observer> observer();

    // Update to next frame if possible.
    virtual void swapFrame() = 0;

    // Ratio of pixels to DIPs.
    //
    // Don't use the devicePixelRatio of QImage, it's always 1.
    virtual float devicePixelRatio() = 0;

    // Size of frame in pixels.
    virtual QSize size() = 0;

    // Whether frame needs an alpha channel.
    virtual bool requiresAlphaChannel() = 0;

    // Wait on texture to be ready aka. sync.
    virtual void waitForTexture();

    // Release any held texture resources
    virtual void releaseTexture();

    // QSGTexture of the frame.
    virtual QSGTexture *texture(QQuickWindow *win, uint32_t textureOptions);

    // Is the texture produced upside down?
    virtual bool textureIsFlipped();

    // Release resources created in texture()
    virtual void releaseResources(QQuickWindow *win);

protected:
    Compositor(Type type) : m_type(type) { }
    virtual ~Compositor() { if (m_binding) unbind(); }

private:
    template<typename T>
    friend class Handle;

    class BindingMap;
    static void unlockBindings();

    const Type m_type;
    Binding *m_binding = nullptr;
};

} // namespace QtWebEngineCore

#endif // !COMPOSITOR_H
