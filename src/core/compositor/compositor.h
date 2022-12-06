// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <QtGui/qtguiglobal.h>
#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>

#if QT_CONFIG(webengine_vulkan)
#include <QVulkanInstance>
#endif

QT_BEGIN_NAMESPACE
class QImage;
class QQuickWindow;
class QSize;
QT_END_NAMESPACE

namespace viz {
class FrameSinkId;
} // namespace viz

namespace QtWebEngineCore {

// Produces composited frames for display.
//
// Used by quick/widgets libraries for accessing the frame and
// controlling frame swapping. Must be cast to a subclass to access
// the frame as QImage or OpenGL texture, etc.
class Q_WEBENGINECORE_PRIVATE_EXPORT Compositor
{
    struct Binding;

public:
    // Identifies the implementation type.
    enum class Type {
        Software,
        OpenGL,
        Vulkan,
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
    //
    // In software mode, the image format can be either
    //   QImage::Format_ARGB32_Premultiplied or
    //   QImage::Format_RGBA8888_Premultiplied
    //
    // In OpenGL mode, the texture format is either GL_RGBA or GL_RGB.
    virtual bool hasAlphaChannel() = 0;

    // (Software) QImage of the frame.
    //
    // This is a big image so we should try not to make copies of it.
    // In particular, the client should drop its QImage reference
    // before calling swapFrame(), otherwise each swap will cause a
    // detach.
    virtual QImage image();

    // (OpenGL) Wait on texture fence in Qt's current OpenGL context.
    virtual void waitForTexture();

    // (OpenGL) Texture of the frame.
    virtual int textureId();

#if QT_CONFIG(webengine_vulkan)
    // (Vulkan) VkImage of the frame.
    virtual VkImage vkImage(QQuickWindow *win);

    // (Vulkan) Layout for vkImage().
    virtual VkImageLayout vkImageLayout();

    // (Vulkan) Release Vulkan resources created by Qt's Vulkan instance.
    virtual void releaseVulkanResources(QQuickWindow *win);
#endif

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
