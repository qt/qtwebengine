// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#define QT_WEBENGINECORE_BUILD_REMOVED_API

#include "qtwebenginecoreglobal.h"

QT_USE_NAMESPACE


#if QT_WEBENGINECORE_REMOVED_SINCE(6, 9)

// #include "qotherheader.h"
// implement removed functions from qotherheader.h
// order sections alphabetically

#endif // QT_WEBENGINECORE_REMOVED_SINCE(6, 9)

#if QT_WEBENGINECORE_REMOVED_SINCE(6, 12)

#include "qtwebenginecoreglobal_p.h"
#include "qwebengineframe.h"
#include "qwebenginescript.h"

void QWebEngineFrame::runJavaScript(const QString &scriptSource, const std::function<void(const QVariant &)> &resultCallback)
{
    runJavaScript(scriptSource, QWebEngineScript::MainWorld, resultCallback);
}

void QWebEngineFrame::runJavaScript(const QString &scriptSource, quint32 worldId, const std::function<void(const QVariant &)> &resultCallback)
{
    if (!resultCallback)
        return runJavaScriptImpl(scriptSource, worldId, nullptr);
    runJavaScriptImpl(scriptSource, worldId,
                      QtPrivate::makeCallableObject<void(*)(const QVariant &)>(resultCallback));
}

void QWebEngineFrame::printToPdf(const std::function<void(const QByteArray&)> &resultCallback)
{
#if QT_CONFIG(webengine_printing_and_pdf)
    std::function wrappedCallback = [resultCallback](std::shared_ptr<QByteArray> result) {
        if (resultCallback)
            resultCallback(result ? *result : QByteArray());
    };
    auto *callback =
            QtPrivate::makeCallableObject<void(*)(std::shared_ptr<QByteArray>)>(std::move(wrappedCallback));

    printToPdfImpl(callback);
#else
    if (resultCallback)
        resultCallback(QByteArray());
#endif
}

#include "qwebenginepage.h"

void QWebEnginePage::runJavaScript(const QString& scriptSource, const std::function<void(const QVariant &)> &resultCallback)
{
    runJavaScript(scriptSource, QWebEngineScript::MainWorld, resultCallback);
}

void QWebEnginePage::runJavaScript(const QString& scriptSource, quint32 worldId, const std::function<void(const QVariant &)> &resultCallback)
{
    if (!resultCallback)
        return runJavaScriptImpl(scriptSource, worldId, nullptr);
    runJavaScriptImpl(scriptSource, worldId,
                      QtPrivate::makeCallableObject<void(*)(const QVariant &)>(resultCallback));
}

void QWebEnginePage::printToPdf(const std::function<void(const QByteArray&)> &resultCallback, const QPageLayout &layout, const QPageRanges &ranges)
{
#if QT_CONFIG(webengine_printing_and_pdf)
    auto wrappedCallback = [resultCallback](std::shared_ptr<QByteArray> result) {
        if (resultCallback)
            resultCallback(result ? *result : QByteArray());
    };
    auto *callback =
            QtPrivate::makeCallableObject<void(*)(std::shared_ptr<QByteArray>)>(std::move(wrappedCallback));

    printToPdfImpl(callback, layout, ranges);
#else
    Q_UNUSED(layout);
    Q_UNUSED(ranges);
    if (resultCallback)
        resultCallback(QByteArray());
#endif
}

#endif // QT_WEBENGINECORE_REMOVED_SINCE(6, 12)
