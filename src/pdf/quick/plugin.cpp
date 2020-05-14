/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtQml/qqml.h>
#include <QtQml/qqmlcomponent.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlextensionplugin.h>
#include "qquickpdfdocument_p.h"
#include "qquickpdflinkmodel_p.h"
#include "qquickpdfnavigationstack_p.h"
#include "qquickpdfsearchmodel_p.h"
#include "qquickpdfselection_p.h"
#include "qquicktableviewextra_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmlmodule QtQuick.Pdf 5.15
    \title Qt Quick PDF QML Types
    \ingroup qmlmodules
    \brief Provides QML types for handling PDF documents.

    This QML module contains types for handling PDF documents.

    To use the types in this module, import the module with the following line:

    \code
    import QtQuick.Pdf 5.15
    \endcode
*/

class QtQuick2PdfPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    QtQuick2PdfPlugin() : QQmlExtensionPlugin() { }

    void initializeEngine(QQmlEngine *engine, const char *uri) override {
        Q_UNUSED(uri);
#ifdef QT_STATIC
        Q_UNUSED(engine);
#else
        engine->addImportPath(QStringLiteral("qrc:/"));
#endif
    }

    void registerTypes(const char *uri) override {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtQuick.Pdf"));

        // Register the latest version, even if there are no new types or new revisions for existing types yet.
        qmlRegisterModule(uri, 2, QT_VERSION_MINOR);

        qmlRegisterType<QQuickPdfDocument>(uri, 5, 15, "PdfDocument");
        qmlRegisterType<QQuickPdfLinkModel>(uri, 5, 15, "PdfLinkModel");
        qmlRegisterType<QQuickPdfNavigationStack>(uri, 5, 15, "PdfNavigationStack");
        qmlRegisterType<QQuickPdfSearchModel>(uri, 5, 15, "PdfSearchModel");
        qmlRegisterType<QQuickPdfSelection>(uri, 5, 15, "PdfSelection");
        qmlRegisterType<QQuickTableViewExtra>(uri, 5, 15, "TableViewExtra");

        qmlRegisterType(QUrl("qrc:/qt-project.org/qtpdf/qml/PdfPageView.qml"), uri, 5, 15, "PdfPageView");
        qmlRegisterType(QUrl("qrc:/qt-project.org/qtpdf/qml/PdfMultiPageView.qml"), uri, 5, 15, "PdfMultiPageView");
        qmlRegisterType(QUrl("qrc:/qt-project.org/qtpdf/qml/PdfScrollablePageView.qml"), uri, 5, 15, "PdfScrollablePageView");
    }
};

QT_END_NAMESPACE

#include "plugin.moc"
