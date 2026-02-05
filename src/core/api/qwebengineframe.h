// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QWEBENGINEFRAME_H
#define QWEBENGINEFRAME_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtCore/qcompare.h>
#include <QtCore/QList>
#include <QtCore/QSizeF>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QWeakPointer>

namespace QtWebEngineCore {
class WebContentsAdapter;
}

QT_BEGIN_NAMESPACE
class QJSValue;

namespace QtWebEngine {
template<typename Functor, typename Arg>
using if_callback_taking_t = std::enable_if_t<QtPrivate::AreFunctionsCompatible<void(*)(Arg), Functor>::value, bool>;
} // namespace QtWebEngine

class QWebEngineFrame
{
    Q_GADGET_EXPORT(Q_WEBENGINECORE_EXPORT)

    Q_PROPERTY(bool isValid READ isValid FINAL)
    Q_PROPERTY(QString name READ name FINAL)
    Q_PROPERTY(QString htmlName READ htmlName FINAL)
    Q_PROPERTY(QUrl url READ url FINAL)
    Q_PROPERTY(QSizeF size READ size FINAL)
    Q_PROPERTY(bool isMainFrame READ isMainFrame FINAL)
    Q_PROPERTY(QList<QWebEngineFrame> children READ children)

public:
    QWebEngineFrame() = default;
    QWebEngineFrame(const QWebEngineFrame &other) = default;
    QWebEngineFrame &operator=(const QWebEngineFrame &other) = default;
    QWebEngineFrame(QWebEngineFrame &&other) = default;
    QWebEngineFrame &operator=(QWebEngineFrame &&other) = default;
    ~QWebEngineFrame() = default;

    Q_WEBENGINECORE_EXPORT bool isValid() const;
    Q_WEBENGINECORE_EXPORT QString name() const;
    Q_WEBENGINECORE_EXPORT QString htmlName() const;
    Q_WEBENGINECORE_EXPORT QList<QWebEngineFrame> children() const;
    Q_WEBENGINECORE_EXPORT QUrl url() const;
    Q_WEBENGINECORE_EXPORT QSizeF size() const;
    Q_WEBENGINECORE_EXPORT bool isMainFrame() const;

    template<typename Functor, QtWebEngine::if_callback_taking_t<Functor, const QVariant &> = true>
    void runJavaScript(const QString &script, Functor &&callback)
    {
        runJavaScript(script, 0, std::forward<Functor>(callback));
    }
    template<typename Functor, QtWebEngine::if_callback_taking_t<Functor, const QVariant &> = true>
    void runJavaScript(const QString &script, quint32 worldId, Functor &&callback)
    {
        if constexpr (std::is_constructible_v<bool, Functor>) {
            if (!callback)
                return runJavaScriptImpl(script, worldId, nullptr);
        }
        runJavaScriptImpl(script, worldId,
                          QtPrivate::makeCallableObject<void(*)(const QVariant &)>(std::forward<Functor>(callback)));
    }

#if QT_WEBENGINECORE_REMOVED_SINCE(6, 12)
    Q_WEBENGINECORE_EXPORT void
    runJavaScript(const QString &script, const std::function<void(const QVariant &)> &callback);
    Q_WEBENGINECORE_EXPORT void
    runJavaScript(const QString &script, quint32 worldId,
                  const std::function<void(const QVariant &)> &callback);
#endif
    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void runJavaScript(const QString &script,
                                                          quint32 worldId = 0);

#if QT_DEPRECATED_SINCE(6, 10)
    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void runJavaScript(const QString &script,
                                                          const QJSValue &callback);
    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void runJavaScript(const QString &script, quint32 worldId,
                                                          const QJSValue &callback);
#endif

    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void printToPdf(const QString &filePath);
    Q_WEBENGINECORE_EXPORT void printToPdf(const std::function<void(const QByteArray &)> &callback);
#if QT_DEPRECATED_SINCE(6, 10)
    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void printToPdf(const QJSValue &callback);
#endif

    friend inline bool comparesEqual(const QWebEngineFrame &lhs,
                                     const QWebEngineFrame &rhs) noexcept
    {
        return lhs.m_adapter == rhs.m_adapter && lhs.m_id == rhs.m_id;
    }

    Q_DECLARE_EQUALITY_COMPARABLE(QWebEngineFrame);

private:
    friend class QWebEnginePage;
    friend class QWebEnginePagePrivate;
    friend class QQuickWebEngineView;
    friend class QQuickWebEngineFrame;

    Q_WEBENGINECORE_EXPORT void runJavaScriptImpl(const QString &scriptSource, quint32 worldId, QtPrivate::QSlotObjectBase *callback);

    Q_WEBENGINECORE_EXPORT
    QWebEngineFrame(QWeakPointer<QtWebEngineCore::WebContentsAdapter> adapter, quint64 id);

    QWeakPointer<QtWebEngineCore::WebContentsAdapter> m_adapter;
    quint64 m_id;
};

QT_END_NAMESPACE

#endif // QWEBENGINEFRAME_H
