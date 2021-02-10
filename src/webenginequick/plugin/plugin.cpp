/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtQml/qqmlextensionplugin.h>
#include <QtWebEngineQuick/QQuickWebEngineProfile>

#include <QtWebEngineQuick/private/qquickwebengineclientcertificateselection_p.h>
#include <QtWebEngineQuick/private/qquickwebenginedialogrequests_p.h>
#include <QtWebEngineQuick/private/qquickwebenginehistory_p.h>
#include <QtWebEngineQuick/private/qquickwebenginefaviconprovider_p_p.h>
#include <QtWebEngineQuick/private/qquickwebenginenavigationrequest_p.h>
#include <QtWebEngineQuick/private/qquickwebenginenewviewrequest_p.h>
#include <QtWebEngineQuick/private/qquickwebenginesettings_p.h>
#include <QtWebEngineQuick/private/qquickwebenginesingleton_p.h>
#include <QtWebEngineQuick/private/qquickwebenginetouchhandleprovider_p_p.h>
#include <QtWebEngineQuick/private/qquickwebengineview_p.h>
#include <QtWebEngineQuick/private/qquickwebengineaction_p.h>
#include <QtWebEngineCore/qwebenginecertificateerror.h>
#include <QtWebEngineCore/qwebenginefindtextresult.h>
#include <QtWebEngineCore/qwebenginefullscreenrequest.h>
#include <QtWebEngineCore/qwebengineloadrequest.h>
#include <QtWebEngineCore/qwebenginenotification.h>
#include <QtWebEngineCore/qwebenginequotarequest.h>
#include <QtWebEngineCore/qwebengineregisterprotocolhandlerrequest.h>
#include <QtWebEngineCore/qwebenginecontextmenurequest.h>
#include <QtWebEngineCore/qwebenginedownloadrequest.h>
#include <QtWebEngineCore/qwebenginescript.h>

QT_BEGIN_NAMESPACE

static QObject *webEngineSingletonProvider(QQmlEngine *, QJSEngine *)
{
    return new QQuickWebEngineSingleton;
}

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

    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtWebEngine"));

        qmlRegisterType<QQuickWebEngineView>(uri, 1, 0, "WebEngineView");
        qmlRegisterUncreatableType<QWebEngineLoadRequest>(uri, 1, 0, "WebEngineLoadRequest", msgUncreatableType("WebEngineLoadRequest"));
        qmlRegisterUncreatableType<QQuickWebEngineNavigationRequest>(uri, 1, 0, "WebEngineNavigationRequest", msgUncreatableType("WebEngineNavigationRequest"));

        qmlRegisterType<QQuickWebEngineView, 1>(uri, 1, 1, "WebEngineView");
        qmlRegisterType<QQuickWebEngineView, 2>(uri, 1, 2, "WebEngineView");
        qmlRegisterType<QQuickWebEngineView, 3>(uri, 1, 3, "WebEngineView");
        qmlRegisterType<QQuickWebEngineView, 4>(uri, 1, 4, "WebEngineView");
        qmlRegisterType<QQuickWebEngineView, 5>(uri, 1, 5, "WebEngineView");
        qmlRegisterType<QQuickWebEngineView, 6>(uri, 1, 6, "WebEngineView");
        qmlRegisterType<QQuickWebEngineView, 7>(uri, 1, 7, "WebEngineView");
        qmlRegisterType<QQuickWebEngineView, 8>(uri, 1, 8, "WebEngineView");
        qmlRegisterType<QQuickWebEngineView, 9>(uri, 1, 9, "WebEngineView");
        qmlRegisterType<QQuickWebEngineView, 10>(uri, 1, 10, "WebEngineView");
        qmlRegisterType<QQuickWebEngineView, 11>(uri, 1, 11, "WebEngineView");
        qmlRegisterType<QQuickWebEngineProfile>(uri, 1, 1, "WebEngineProfile");
        qmlRegisterType<QQuickWebEngineProfile, 1>(uri, 1, 2, "WebEngineProfile");
        qmlRegisterType<QQuickWebEngineProfile, 2>(uri, 1, 3, "WebEngineProfile");
        qmlRegisterType<QQuickWebEngineProfile, 3>(uri, 1, 4, "WebEngineProfile");
        qmlRegisterType<QQuickWebEngineProfile, 4>(uri, 1, 5, "WebEngineProfile");
        qmlRegisterType<QQuickWebEngineProfile, 5>(uri, 1, 9, "WebEngineProfile");
        qRegisterMetaType<QWebEngineScript>();
        qmlRegisterUncreatableType<QWebEngineScript>(
                uri, 1, 1, "WebEngineScript", msgUncreatableType("WebEngineScript")); // for enums
        qRegisterMetaType<QWebEngineCertificateError>();
        qmlRegisterUncreatableType<QWebEngineCertificateError>(uri, 1, 1, "WebEngineCertificateError", msgUncreatableType("WebEngineCertificateError"));
        qmlRegisterUncreatableType<QWebEngineDownloadRequest>(uri, 1, 1, "WebEngineDownloadRequest",
            msgUncreatableType("WebEngineDownloadRequest"));
        qmlRegisterUncreatableType<QQuickWebEngineNewViewRequest>(uri, 1, 1, "WebEngineNewViewRequest", msgUncreatableType("WebEngineNewViewRequest"));
        qmlRegisterUncreatableType<QQuickWebEngineNewViewRequest, 1>(uri, 1, 5, "WebEngineNewViewRequest", tr("Cannot create separate instance of WebEngineNewViewRequest"));
        qmlRegisterUncreatableType<QQuickWebEngineSettings>(uri, 1, 1, "WebEngineSettings", msgUncreatableType("WebEngineSettings"));
        qmlRegisterUncreatableType<QQuickWebEngineSettings, 1>(uri, 1, 2, "WebEngineSettings", msgUncreatableType("WebEngineSettings"));
        qmlRegisterUncreatableType<QQuickWebEngineSettings, 2>(uri, 1, 3, "WebEngineSettings", msgUncreatableType("WebEngineSettings"));
        qmlRegisterUncreatableType<QQuickWebEngineSettings, 3>(uri, 1, 4, "WebEngineSettings", msgUncreatableType("WebEngineSettings"));
        qmlRegisterUncreatableType<QQuickWebEngineSettings, 4>(uri, 1, 5, "WebEngineSettings", msgUncreatableType("WebEngineSettings"));
        qmlRegisterUncreatableType<QQuickWebEngineSettings, 5>(uri, 1, 6, "WebEngineSettings", msgUncreatableType("WebEngineSettings"));
        qmlRegisterUncreatableType<QQuickWebEngineSettings, 6>(uri, 1, 7, "WebEngineSettings", msgUncreatableType("WebEngineSettings"));
        qmlRegisterUncreatableType<QQuickWebEngineSettings, 7>(uri, 1, 8, "WebEngineSettings", msgUncreatableType("WebEngineSettings"));
        qmlRegisterUncreatableType<QQuickWebEngineSettings, 8>(uri, 1, 9, "WebEngineSettings", msgUncreatableType("WebEngineSettings"));
        qmlRegisterSingletonType<QQuickWebEngineSingleton>(uri, 1, 1, "WebEngine", webEngineSingletonProvider);
        qmlRegisterUncreatableType<QQuickWebEngineHistory>(uri, 1, 1, "NavigationHistory",
            msgUncreatableType("NavigationHistory"));
        qmlRegisterUncreatableType<QQuickWebEngineHistory, 1>(uri, 1, 11, "NavigationHistory", msgUncreatableType("NavigationHistory"));
        qmlRegisterUncreatableType<QQuickWebEngineHistoryListModel>(uri, 1, 1, "NavigationHistoryListModel",
            msgUncreatableType("NavigationHistory"));
        qmlRegisterUncreatableType<QWebEngineFullScreenRequest>(uri, 1, 1, "FullScreenRequest",
            msgUncreatableType("FullScreenRequest"));

        qmlRegisterUncreatableType<QWebEngineContextMenuRequest, 1>(
                uri, 1, 7, "ContextMenuRequest", msgUncreatableType("ContextMenuRequest"));
        qmlRegisterUncreatableType<QQuickWebEngineAuthenticationDialogRequest>(uri, 1, 4, "AuthenticationDialogRequest",
                                                                       msgUncreatableType("AuthenticationDialogRequest"));
        qmlRegisterUncreatableType<QQuickWebEngineJavaScriptDialogRequest>(uri, 1, 4, "JavaScriptDialogRequest",
                                                                         msgUncreatableType("JavaScriptDialogRequest"));
        qmlRegisterUncreatableType<QQuickWebEngineColorDialogRequest>(uri, 1, 4, "ColorDialogRequest",
                                                                         msgUncreatableType("ColorDialogRequest"));
        qmlRegisterUncreatableType<QQuickWebEngineFileDialogRequest>(uri, 1, 4, "FileDialogRequest",
                                                                         msgUncreatableType("FileDialogRequest"));
        qmlRegisterUncreatableType<QQuickWebEngineFormValidationMessageRequest>(uri, 1, 4, "FormValidationMessageRequest",
                                                                         msgUncreatableType("FormValidationMessageRequest"));
        qRegisterMetaType<QWebEngineQuotaRequest>();
        qmlRegisterUncreatableType<QWebEngineQuotaRequest>(uri, 1, 7, "QuotaRequest",
                                                           msgUncreatableType("QuotaRequest"));
        qRegisterMetaType<QWebEngineRegisterProtocolHandlerRequest>();
        qmlRegisterUncreatableType<QWebEngineRegisterProtocolHandlerRequest>(uri, 1, 7, "RegisterProtocolHandlerRequest",
                                                                             msgUncreatableType("RegisterProtocolHandlerRequest"));
        qmlRegisterUncreatableType<QQuickWebEngineAction>(uri, 1, 8, "WebEngineAction", msgUncreatableType("WebEngineAction"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        qmlRegisterUncreatableType<QQuickWebEngineClientCertificateSelection>(uri, 1, 9, "WebEngineClientCertificateSelection",
                                                                              msgUncreatableType("WebEngineClientCertificateSelection"));
        qmlRegisterUncreatableType<QQuickWebEngineClientCertificateOption>(uri, 1, 9, "WebEngineClientCertificateOption",
                                                                           msgUncreatableType("WebEngineClientCertificateOption"));
#endif
        qmlRegisterUncreatableType<QWebEngineNotification>(uri, 1, 9, "WebEngineNotification", msgUncreatableType("WebEngineNotification"));
        qmlRegisterUncreatableType<QQuickWebEngineTooltipRequest>(uri, 1, 10, "TooltipRequest",
                                                                  msgUncreatableType("TooltipRequest"));
        qRegisterMetaType<QWebEngineFindTextResult>();
        qmlRegisterUncreatableType<QWebEngineFindTextResult>(uri, 1, 10, "FindTextResult", msgUncreatableType("FindTextResult"));
    }

private:
    static QString msgUncreatableType(const char *className)
    {
        return tr("Cannot create separate instance of %1").arg(QLatin1String(className));
    }
};

QT_END_NAMESPACE

#include "plugin.moc"
