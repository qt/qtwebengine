// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtQml/qqmlextensionplugin.h>
#include <QtWebEngineQuick/QQuickWebEngineProfile>

#include <QtWebEngineQuick/private/qquickwebenginefaviconprovider_p_p.h>
#include <QtWebEngineQuick/private/qquickwebenginetouchhandleprovider_p_p.h>

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
QT_BEGIN_NAMESPACE
void Q_WEBENGINEQUICK_PRIVATE_EXPORT qml_register_types_QtWebEngine();
QT_END_NAMESPACE
#else
void Q_WEBENGINEQUICK_PRIVATE_EXPORT qml_register_types_QtWebEngine();
#endif

QT_BEGIN_NAMESPACE

class QtWebEnginePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void initializeEngine(QQmlEngine *engine, const char *uri) override
    {
        Q_UNUSED(uri);
        engine->addImageProvider(QQuickWebEngineFaviconProvider::identifier(), new QQuickWebEngineFaviconProvider);
        engine->addImageProvider(QQuickWebEngineTouchHandleProvider::identifier(), new QQuickWebEngineTouchHandleProvider);
    }
    void registerTypes(const char *uri) override {
        volatile auto registration = &qml_register_types_QtWebEngine;
        Q_UNUSED(registration);
        Q_UNUSED(uri);
    };
};

QT_END_NAMESPACE

#include "plugin.moc"
