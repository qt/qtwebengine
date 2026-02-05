// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwebengineframe_p.h"
#include <QtWebEngineCore/qwebenginescript.h>

#include "web_contents_adapter_client.h"
#include "web_contents_adapter.h"

#include <QtQml/qqmlengine.h>
#include <QtGui/qpagelayout.h>
#include <QtGui/qpageranges.h>

QT_BEGIN_NAMESPACE

QQuickWebEngineFrame::QQuickWebEngineFrame()
    : QWebEngineFrame(QWeakPointer<QtWebEngineCore::WebContentsAdapter>(),
                      QtWebEngineCore::WebContentsAdapter::kInvalidFrameId) { };

QQuickWebEngineFrame::QQuickWebEngineFrame(
        QWeakPointer<QtWebEngineCore::WebContentsAdapter> adapter, quint64 id)
    : QWebEngineFrame(std::move(adapter), id)
{
}

QList<QQuickWebEngineFrame> QQuickWebEngineFrame::children() const
{
    auto adapter = m_adapter.lock();
    if (!adapter)
        return QList<QQuickWebEngineFrame>();
    QList<QQuickWebEngineFrame> result;
    for (auto childId : adapter->frameChildren(m_id))
        result.push_back(QQuickWebEngineFrame{ m_adapter, childId });
    return result;
}

void QQuickWebEngineFrame::runJavaScript(const QString &script, const QJSValue &callback)
{
    QWebEngineFrame::runJavaScript(script, QWebEngineScript::MainWorld, callback);
}

void QQuickWebEngineFrame::runJavaScript(const QString &script, quint32 worldId,
                                         const QJSValue &callback)
{
    const auto adapter = m_adapter.lock();
    if (!adapter)
        return;
    if (!callback.isUndefined()) {
        const QObject *holdingObject = adapter->adapterClient()->holdingQObject();
        auto wrappedCallback = [holdingObject, callback](const QVariant &result) {
            if (auto engine = qmlEngine(holdingObject)) {
                QJSValueList args;
                args.append(engine->toScriptValue(result));
                callback.call(args);
            } else {
                qWarning("No QML engine found to execute runJavaScript() callback");
            }
        };
        QWebEngineFrame::runJavaScript(script, worldId, std::move(wrappedCallback));
    } else {
        QWebEngineFrame::runJavaScript(script, worldId);
    }
}

void QQuickWebEngineFrame::printToPdf(const QJSValue &callback)
{
    const auto adapter = m_adapter.lock();
    if (!adapter)
        return;
    std::function<void(QSharedPointer<QByteArray>)> wrappedCallback;
    if (!callback.isUndefined()) {
        const QObject *holdingObject = adapter->adapterClient()->holdingQObject();
        wrappedCallback = [holdingObject, callback](QSharedPointer<QByteArray> result) {
            if (auto engine = qmlEngine(holdingObject)) {
                QJSValueList args;
                args.append(engine->toScriptValue(result ? *result : QByteArray()));
                callback.call(args);
            } else {
                qWarning("No QML engine found to execute printToPdf() callback");
            }
        };
    }
    QPageLayout layout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF());
    adapter->adapterClient()->printToPdf(std::move(wrappedCallback), layout, QPageRanges(), m_id);
}

QT_END_NAMESPACE

#include "moc_qquickwebengineframe_p.cpp"
