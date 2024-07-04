// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEFRAME_H
#define QWEBENGINEFRAME_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtQml/qqmlregistration.h>
#include <QtQml/qjsvalue.h>
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

class QWebEngineFrame
{
    Q_GADGET_EXPORT(Q_WEBENGINECORE_EXPORT)

    Q_PROPERTY(bool isValid READ isValid FINAL)
    Q_PROPERTY(QString name READ name FINAL)
    Q_PROPERTY(QString htmlName READ htmlName FINAL)
    Q_PROPERTY(QUrl url READ url FINAL)
    Q_PROPERTY(QSizeF size READ size FINAL)
    Q_PROPERTY(bool isMainFrame READ isMainFrame FINAL)

public:
    QML_VALUE_TYPE(webEngineFrame)
    QML_ADDED_IN_VERSION(6, 8)

    Q_WEBENGINECORE_EXPORT bool isValid() const;
    Q_WEBENGINECORE_EXPORT QString name() const;
    Q_WEBENGINECORE_EXPORT QString htmlName() const;
    Q_WEBENGINECORE_EXPORT QList<QWebEngineFrame> children() const;
    Q_WEBENGINECORE_EXPORT QUrl url() const;
    Q_WEBENGINECORE_EXPORT QSizeF size() const;
    Q_WEBENGINECORE_EXPORT bool isMainFrame() const;

    Q_WEBENGINECORE_EXPORT void
    runJavaScript(const QString &script, const std::function<void(const QVariant &)> &callback);
    Q_WEBENGINECORE_EXPORT void
    runJavaScript(const QString &script, quint32 worldId,
                  const std::function<void(const QVariant &)> &callback);
    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void runJavaScript(const QString &script,
                                                          quint32 worldId = 0);
    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void runJavaScript(const QString &script,
                                                          const QJSValue &callback);
    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void runJavaScript(const QString &script, quint32 worldId,
                                                          const QJSValue &callback);

    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void printToPdf(const QString &filePath);
    Q_WEBENGINECORE_EXPORT void printToPdf(const std::function<void(const QByteArray &)> &callback);
    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void printToPdf(const QJSValue &callback);

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
    friend class QQuickWebEngineViewPrivate;

    Q_WEBENGINECORE_EXPORT
    QWebEngineFrame(QWeakPointer<QtWebEngineCore::WebContentsAdapter> adapter, quint64 id);

    QWeakPointer<QtWebEngineCore::WebContentsAdapter> m_adapter;
    quint64 m_id;
};

QT_END_NAMESPACE

#endif // QWEBENGINEFRAME_H
