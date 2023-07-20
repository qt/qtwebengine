// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineview_plugin.h"

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include <QtQuick/QQuickWindow>
#include <QWebEngineView>

QT_BEGIN_NAMESPACE

static const char toolTipC[] =
    QT_TRANSLATE_NOOP(QWebEngineViewPlugin,
                      "A widget for displaying a web page, from the Qt WebEngineWidgets module");

QWebEngineViewPlugin::QWebEngineViewPlugin(QObject *parent) :
    QObject(parent),
    m_initialized(false)
{
}

QString QWebEngineViewPlugin::name() const
{
    return QStringLiteral("QWebEngineView");
}

QString QWebEngineViewPlugin::group() const
{
    return QStringLiteral("Display Widgets");
}

QString QWebEngineViewPlugin::toolTip() const
{
    return tr(toolTipC);
}

QString QWebEngineViewPlugin::whatsThis() const
{
    return tr(toolTipC);
}

QString QWebEngineViewPlugin::includeFile() const
{
    return QStringLiteral("<QtWebEngineWidgets/QWebEngineView>");
}

QIcon QWebEngineViewPlugin::icon() const
{
    return QIcon(QStringLiteral(":/qt-project.org/qwebengineview/images/qwebengineview.png"));
}

bool QWebEngineViewPlugin::isContainer() const
{
    return false;
}

QWidget *QWebEngineViewPlugin::createWidget(QWidget *parent)
{
    if (parent)
        return new QWebEngineView(parent);
    return new fake::QWebEngineView;
}

bool QWebEngineViewPlugin::isInitialized() const
{
    return m_initialized;
}

void QWebEngineViewPlugin::initialize(QDesignerFormEditorInterface * /*core*/)
{
    if (m_initialized)
        return;

    m_initialized = true;
}

QString QWebEngineViewPlugin::domXml() const
{
    return QStringLiteral("\
    <ui language=\"c++\">\
        <widget class=\"QWebEngineView\" name=\"webEngineView\">\
            <property name=\"url\">\
                <url>\
                    <string>about:blank</string>\
                </url>\
            </property>\
            <property name=\"geometry\">\
                <rect>\
                    <x>0</x>\
                    <y>0</y>\
                    <width>300</width>\
                    <height>200</height>\
                </rect>\
            </property>\
        </widget>\
    </ui>");
}

QT_END_NAMESPACE
