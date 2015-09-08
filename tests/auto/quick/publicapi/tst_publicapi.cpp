/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
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
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QMetaEnum>
#include <QMetaMethod>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaType>
#include <QQmlListProperty>
#include <QtTest/QtTest>
#include <private/qquickwebengineview_p.h>
#include <private/qquickwebenginecertificateerror_p.h>
#include <private/qquickwebenginedownloaditem_p.h>
#include <private/qquickwebenginehistory_p.h>
#include <private/qquickwebengineloadrequest_p.h>
#include <private/qquickwebenginenavigationrequest_p.h>
#include <private/qquickwebenginenewviewrequest_p.h>
#include <private/qquickwebengineprofile_p.h>
#include <private/qquickwebenginescript_p.h>
#include <private/qquickwebenginesettings_p.h>

class tst_publicapi : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void publicAPI();
};

static QList<const QMetaObject *> typesToCheck = QList<const QMetaObject *>()
    << &QQuickWebEngineView::staticMetaObject
    << &QQuickWebEngineCertificateError::staticMetaObject
    << &QQuickWebEngineDownloadItem::staticMetaObject
    << &QQuickWebEngineHistory::staticMetaObject
    << &QQuickWebEngineHistoryListModel::staticMetaObject
    << &QQuickWebEngineLoadRequest::staticMetaObject
    << &QQuickWebEngineNavigationRequest::staticMetaObject
    << &QQuickWebEngineNewViewRequest::staticMetaObject
    << &QQuickWebEngineProfile::staticMetaObject
    << &QQuickWebEngineScript::staticMetaObject
    << &QQuickWebEngineSettings::staticMetaObject
    << &QQuickWebEngineFullScreenRequest::staticMetaObject
    ;

static QList<const char *> knownEnumNames = QList<const char *>();

static QStringList hardcodedTypes = QStringList()
    << "QJSValue"
    << "QQmlListProperty<QQuickWebEngineScript>"
    << "QQmlWebChannel*"
    // Ignore the testSupport types without making a fuss.
    << "QQuickWebEngineTestSupport*"
    << "QQuickWebEngineErrorPage*"
    ;

static QStringList expectedAPI = QStringList()
    << "QQuickWebEngineView.AcceptRequest --> NavigationRequestAction"
    << "QQuickWebEngineView.IgnoreRequest --> NavigationRequestAction"
    << "QQuickWebEngineView.LoadStartedStatus --> LoadStatus"
    << "QQuickWebEngineView.LoadStoppedStatus --> LoadStatus"
    << "QQuickWebEngineView.LoadSucceededStatus --> LoadStatus"
    << "QQuickWebEngineView.LoadFailedStatus --> LoadStatus"
    << "QQuickWebEngineCertificateError.SslPinnedKeyNotInCertificateChain --> Error"
    << "QQuickWebEngineCertificateError.CertificateCommonNameInvalid --> Error"
    << "QQuickWebEngineCertificateError.CertificateDateInvalid --> Error"
    << "QQuickWebEngineCertificateError.CertificateAuthorityInvalid --> Error"
    << "QQuickWebEngineCertificateError.CertificateContainsErrors --> Error"
    << "QQuickWebEngineCertificateError.CertificateNoRevocationMechanism --> Error"
    << "QQuickWebEngineCertificateError.CertificateUnableToCheckRevocation --> Error"
    << "QQuickWebEngineCertificateError.CertificateRevoked --> Error"
    << "QQuickWebEngineCertificateError.CertificateInvalid --> Error"
    << "QQuickWebEngineCertificateError.CertificateWeakSignatureAlgorithm --> Error"
    << "QQuickWebEngineCertificateError.CertificateNonUniqueName --> Error"
    << "QQuickWebEngineCertificateError.CertificateWeakKey --> Error"
    << "QQuickWebEngineCertificateError.CertificateNameConstraintViolation --> Error"
    << "QQuickWebEngineView.NoErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.InternalErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.ConnectionErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.CertificateErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.HttpErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.FtpErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.DnsErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.FindBackward --> FindFlags"
    << "QQuickWebEngineView.FindCaseSensitively --> FindFlags"
    << "QQuickWebEngineView.LinkClickedNavigation --> NavigationType"
    << "QQuickWebEngineView.TypedNavigation --> NavigationType"
    << "QQuickWebEngineView.FormSubmittedNavigation --> NavigationType"
    << "QQuickWebEngineView.BackForwardNavigation --> NavigationType"
    << "QQuickWebEngineView.ReloadNavigation --> NavigationType"
    << "QQuickWebEngineView.OtherNavigation --> NavigationType"
    << "QQuickWebEngineView.NewViewInWindow --> NewViewDestination"
    << "QQuickWebEngineView.NewViewInTab --> NewViewDestination"
    << "QQuickWebEngineView.NewViewInDialog --> NewViewDestination"
    << "QQuickWebEngineView.NewViewInBackgroundTab --> NewViewDestination"
    << "QQuickWebEngineView.MediaAudioCapture --> Feature"
    << "QQuickWebEngineView.MediaVideoCapture --> Feature"
    << "QQuickWebEngineView.MediaAudioVideoCapture --> Feature"
    << "QQuickWebEngineView.Geolocation --> Feature"
    << "QQuickWebEngineView.InfoMessageLevel --> JavaScriptConsoleMessageLevel"
    << "QQuickWebEngineView.WarningMessageLevel --> JavaScriptConsoleMessageLevel"
    << "QQuickWebEngineView.ErrorMessageLevel --> JavaScriptConsoleMessageLevel"
    << "QQuickWebEngineView.title --> QString"
    << "QQuickWebEngineView.url --> QUrl"
    << "QQuickWebEngineView.icon --> QUrl"
    << "QQuickWebEngineView.canGoBack --> bool"
    << "QQuickWebEngineView.canGoForward --> bool"
    << "QQuickWebEngineView.isFullScreen --> bool"
    << "QQuickWebEngineView.loading --> bool"
    << "QQuickWebEngineView.loadProgress --> int"
    << "QQuickWebEngineView.titleChanged() --> void"
    << "QQuickWebEngineView.loadingChanged(QQuickWebEngineLoadRequest*) --> void"
    << "QQuickWebEngineView.certificateError(QQuickWebEngineCertificateError*) --> void"
    << "QQuickWebEngineView.loadProgressChanged() --> void"
    << "QQuickWebEngineView.javaScriptConsoleMessage(JavaScriptConsoleMessageLevel,QString,int,QString) --> void"
    << "QQuickWebEngineView.urlChanged() --> void"
    << "QQuickWebEngineView.iconChanged() --> void"
    << "QQuickWebEngineView.linkHovered(QUrl) --> void"
    << "QQuickWebEngineView.navigationRequested(QQuickWebEngineNavigationRequest*) --> void"
    << "QQuickWebEngineView.fullScreenRequested(QQuickWebEngineFullScreenRequest) --> void"
    << "QQuickWebEngineView.isFullScreenChanged() --> void"
    << "QQuickWebEngineView.fullScreenCancelled() --> void"
    << "QQuickWebEngineView.featurePermissionRequested(QUrl,Feature) --> void"
    << "QQuickWebEngineView.grantFeaturePermission(QUrl,Feature,bool) --> void"
    << "QQuickWebEngineView.runJavaScript(QString,QJSValue) --> void"
    << "QQuickWebEngineView.runJavaScript(QString) --> void"
    << "QQuickWebEngineView.loadHtml(QString,QUrl) --> void"
    << "QQuickWebEngineView.loadHtml(QString) --> void"
    << "QQuickWebEngineView.goBack() --> void"
    << "QQuickWebEngineView.goForward() --> void"
    << "QQuickWebEngineView.goBackOrForward(int) --> void"
    << "QQuickWebEngineView.stop() --> void"
    << "QQuickWebEngineView.reload() --> void"
    << "QQuickWebEngineView.zoomFactor --> double"
    << "QQuickWebEngineView.zoomFactorChanged(double) --> void"
    << "QQuickWebEngineView.profile --> QQuickWebEngineProfile*"
    << "QQuickWebEngineView.profileChanged() --> void"
    << "QQuickWebEngineView.navigationHistory --> QQuickWebEngineHistory*"
    << "QQuickWebEngineView.newViewRequested(QQuickWebEngineNewViewRequest*) --> void"
    << "QQuickWebEngineView.userScripts --> QQmlListProperty<QQuickWebEngineScript>"
    << "QQuickWebEngineView.settings --> QQuickWebEngineSettings*"
    << "QQuickWebEngineView.testSupport --> QQuickWebEngineTestSupport*"
    << "QQuickWebEngineView.webChannel --> QQmlWebChannel*"
    << "QQuickWebEngineView.webChannelChanged() --> void"
    << "QQuickWebEngineView.reloadAndBypassCache() --> void"
    << "QQuickWebEngineView.findText(QString,FindFlags,QJSValue) --> void"
    << "QQuickWebEngineView.findText(QString,FindFlags) --> void"
    << "QQuickWebEngineView.findText(QString) --> void"
    << "QQuickWebEngineDownloadItem.id --> uint"
    << "QQuickWebEngineDownloadItem.state --> DownloadState"
    << "QQuickWebEngineDownloadItem.path --> QString"
    << "QQuickWebEngineDownloadItem.totalBytes --> qlonglong"
    << "QQuickWebEngineDownloadItem.receivedBytes --> qlonglong"
    << "QQuickWebEngineDownloadItem.DownloadRequested --> DownloadState"
    << "QQuickWebEngineDownloadItem.DownloadInProgress --> DownloadState"
    << "QQuickWebEngineDownloadItem.DownloadCompleted --> DownloadState"
    << "QQuickWebEngineDownloadItem.DownloadCancelled --> DownloadState"
    << "QQuickWebEngineDownloadItem.DownloadInterrupted --> DownloadState"
    << "QQuickWebEngineDownloadItem.stateChanged() --> void"
    << "QQuickWebEngineDownloadItem.pathChanged() --> void"
    << "QQuickWebEngineDownloadItem.receivedBytesChanged() --> void"
    << "QQuickWebEngineDownloadItem.totalBytesChanged() --> void"
    << "QQuickWebEngineDownloadItem.accept() --> void"
    << "QQuickWebEngineDownloadItem.cancel() --> void"
    << "QQuickWebEngineHistory.items --> QQuickWebEngineHistoryListModel*"
    << "QQuickWebEngineHistory.backItems --> QQuickWebEngineHistoryListModel*"
    << "QQuickWebEngineHistory.forwardItems --> QQuickWebEngineHistoryListModel*"
    << "QQuickWebEngineLoadRequest.url --> QUrl"
    << "QQuickWebEngineLoadRequest.status --> QQuickWebEngineView::LoadStatus"
    << "QQuickWebEngineLoadRequest.errorString --> QString"
    << "QQuickWebEngineLoadRequest.errorDomain --> QQuickWebEngineView::ErrorDomain"
    << "QQuickWebEngineLoadRequest.errorCode --> int"
    << "QQuickWebEngineNavigationRequest.url --> QUrl"
    << "QQuickWebEngineNavigationRequest.isMainFrame --> bool"
    << "QQuickWebEngineNavigationRequest.action --> QQuickWebEngineView::NavigationRequestAction"
    << "QQuickWebEngineNavigationRequest.navigationType --> QQuickWebEngineView::NavigationType"
    << "QQuickWebEngineNavigationRequest.actionChanged() --> void"
    << "QQuickWebEngineNewViewRequest.destination --> QQuickWebEngineView::NewViewDestination"
    << "QQuickWebEngineNewViewRequest.userInitiated --> bool"
    << "QQuickWebEngineNewViewRequest.openIn(QQuickWebEngineView*) --> void"
    << "QQuickWebEngineProfile.MemoryHttpCache --> HttpCacheType"
    << "QQuickWebEngineProfile.DiskHttpCache --> HttpCacheType"
    << "QQuickWebEngineProfile.NoPersistentCookies --> PersistentCookiesPolicy"
    << "QQuickWebEngineProfile.AllowPersistentCookies --> PersistentCookiesPolicy"
    << "QQuickWebEngineProfile.ForcePersistentCookies --> PersistentCookiesPolicy"
    << "QQuickWebEngineProfile.storageName --> QString"
    << "QQuickWebEngineProfile.offTheRecord --> bool"
    << "QQuickWebEngineProfile.persistentStoragePath --> QString"
    << "QQuickWebEngineProfile.cachePath --> QString"
    << "QQuickWebEngineProfile.httpUserAgent --> QString"
    << "QQuickWebEngineProfile.httpCacheType --> HttpCacheType"
    << "QQuickWebEngineProfile.persistentCookiesPolicy --> PersistentCookiesPolicy"
    << "QQuickWebEngineProfile.httpCacheMaximumSize --> int"
    << "QQuickWebEngineProfile.storageNameChanged() --> void"
    << "QQuickWebEngineProfile.offTheRecordChanged() --> void"
    << "QQuickWebEngineProfile.persistentStoragePathChanged() --> void"
    << "QQuickWebEngineProfile.cachePathChanged() --> void"
    << "QQuickWebEngineProfile.httpUserAgentChanged() --> void"
    << "QQuickWebEngineProfile.httpCacheTypeChanged() --> void"
    << "QQuickWebEngineProfile.persistentCookiesPolicyChanged() --> void"
    << "QQuickWebEngineProfile.httpCacheMaximumSizeChanged() --> void"
    << "QQuickWebEngineProfile.downloadRequested(QQuickWebEngineDownloadItem*) --> void"
    << "QQuickWebEngineProfile.downloadFinished(QQuickWebEngineDownloadItem*) --> void"
    << "QQuickWebEngineSettings.autoLoadImages --> bool"
    << "QQuickWebEngineSettings.javascriptEnabled --> bool"
    << "QQuickWebEngineSettings.javascriptCanOpenWindows --> bool"
    << "QQuickWebEngineSettings.javascriptCanAccessClipboard --> bool"
    << "QQuickWebEngineSettings.linksIncludedInFocusChain --> bool"
    << "QQuickWebEngineSettings.localStorageEnabled --> bool"
    << "QQuickWebEngineSettings.localContentCanAccessRemoteUrls --> bool"
    << "QQuickWebEngineSettings.spatialNavigationEnabled --> bool"
    << "QQuickWebEngineSettings.localContentCanAccessFileUrls --> bool"
    << "QQuickWebEngineSettings.hyperlinkAuditingEnabled --> bool"
    << "QQuickWebEngineSettings.errorPageEnabled --> bool"
    << "QQuickWebEngineSettings.defaultTextEncoding --> QString"
    << "QQuickWebEngineSettings.autoLoadImagesChanged() --> void"
    << "QQuickWebEngineSettings.javascriptEnabledChanged() --> void"
    << "QQuickWebEngineSettings.javascriptCanOpenWindowsChanged() --> void"
    << "QQuickWebEngineSettings.javascriptCanAccessClipboardChanged() --> void"
    << "QQuickWebEngineSettings.linksIncludedInFocusChainChanged() --> void"
    << "QQuickWebEngineSettings.localStorageEnabledChanged() --> void"
    << "QQuickWebEngineSettings.localContentCanAccessRemoteUrlsChanged() --> void"
    << "QQuickWebEngineSettings.spatialNavigationEnabledChanged() --> void"
    << "QQuickWebEngineSettings.localContentCanAccessFileUrlsChanged() --> void"
    << "QQuickWebEngineSettings.hyperlinkAuditingEnabledChanged() --> void"
    << "QQuickWebEngineSettings.errorPageEnabledChanged() --> void"
    << "QQuickWebEngineSettings.defaultTextEncodingChanged() --> void"
    << "QQuickWebEngineCertificateError.ignoreCertificateError() --> void"
    << "QQuickWebEngineCertificateError.rejectCertificate() --> void"
    << "QQuickWebEngineCertificateError.defer() --> void"
    << "QQuickWebEngineCertificateError.url --> QUrl"
    << "QQuickWebEngineCertificateError.error --> Error"
    << "QQuickWebEngineCertificateError.description --> QString"
    << "QQuickWebEngineCertificateError.overridable --> bool"
    << "QQuickWebEngineScript.Deferred --> InjectionPoint"
    << "QQuickWebEngineScript.DocumentReady --> InjectionPoint"
    << "QQuickWebEngineScript.DocumentCreation --> InjectionPoint"
    << "QQuickWebEngineScript.MainWorld --> ScriptWorldId"
    << "QQuickWebEngineScript.ApplicationWorld --> ScriptWorldId"
    << "QQuickWebEngineScript.UserWorld --> ScriptWorldId"
    << "QQuickWebEngineScript.name --> QString"
    << "QQuickWebEngineScript.sourceUrl --> QUrl"
    << "QQuickWebEngineScript.sourceCode --> QString"
    << "QQuickWebEngineScript.injectionPoint --> InjectionPoint"
    << "QQuickWebEngineScript.worldId --> ScriptWorldId"
    << "QQuickWebEngineScript.runOnSubframes --> bool"
    << "QQuickWebEngineScript.nameChanged(QString) --> void"
    << "QQuickWebEngineScript.sourceUrlChanged(QUrl) --> void"
    << "QQuickWebEngineScript.sourceCodeChanged(QString) --> void"
    << "QQuickWebEngineScript.injectionPointChanged(InjectionPoint) --> void"
    << "QQuickWebEngineScript.worldIdChanged(ScriptWorldId) --> void"
    << "QQuickWebEngineScript.runOnSubframesChanged(bool) --> void"
    << "QQuickWebEngineScript.setName(QString) --> void"
    << "QQuickWebEngineScript.setSourceUrl(QUrl) --> void"
    << "QQuickWebEngineScript.setSourceCode(QString) --> void"
    << "QQuickWebEngineScript.setInjectionPoint(InjectionPoint) --> void"
    << "QQuickWebEngineScript.setWorldId(ScriptWorldId) --> void"
    << "QQuickWebEngineScript.setRunOnSubframes(bool) --> void"
    << "QQuickWebEngineScript.toString() --> QString"
    << "QQuickWebEngineFullScreenRequest.toggleOn --> bool"
    << "QQuickWebEngineFullScreenRequest.accept() --> void"
    ;

static bool isCheckedEnum(const QByteArray &typeName)
{
    QList<QByteArray> tokens = typeName.split(':');
    if (tokens.size() == 3) {
        QByteArray &enumClass = tokens[0];
        QByteArray &enumName = tokens[2];
        foreach (const QMetaObject *mo, typesToCheck) {
            if (mo->className() != enumClass)
                continue;
            for (int i = mo->enumeratorOffset(); i < mo->enumeratorCount(); ++i)
                if (mo->enumerator(i).name() == enumName)
                    return true;
        }
    } else if (tokens.size() == 1) {
        QByteArray &enumName = tokens[0];
        foreach (const char *knownEnumName, knownEnumNames) {
            if (enumName == knownEnumName)
                return true;
        }
    }
    return false;
}

static bool isCheckedClass(const QByteArray &typeName)
{
    foreach (const QMetaObject *mo, typesToCheck) {
        QByteArray moTypeName(mo->className());
        if (moTypeName == typeName || moTypeName + "*" == typeName)
            return true;
    }
    return false;
}

static void checkKnownType(const QByteArray &typeName)
{
    if ((!hardcodedTypes.contains(typeName) && !QMetaType::type(typeName)) || QMetaType::type(typeName) >= QMetaType::User) {
        bool knownEnum = isCheckedEnum(typeName);
        bool knownClass = isCheckedClass(typeName);
        QVERIFY2(knownEnum || knownClass, qPrintable(QString("The API uses an unknown type [%1], you might have to add it to the typesToCheck list.").arg(typeName.constData())));
    }
}

static void gatherAPI(const QString &prefix, const QMetaEnum &metaEnum, QStringList *output)
{
    for (int i = 0; i < metaEnum.keyCount(); ++i)
        *output << QString::fromLatin1("%1%2 --> %3").arg(prefix).arg(metaEnum.key(i)).arg(metaEnum.name());
}

static void gatherAPI(const QString &prefix, const QMetaProperty &property, QStringList *output)
{
    *output << QString::fromLatin1("%1%2 --> %3").arg(prefix).arg(property.name()).arg(property.typeName());
    checkKnownType(property.typeName());
}

static void gatherAPI(const QString &prefix, const QMetaMethod &method, QStringList *output)
{
    if (method.access() != QMetaMethod::Private) {
        const char *methodTypeName = !!strlen(method.typeName()) ? method.typeName() : "void";
        *output << QString::fromLatin1("%1%2 --> %3").arg(prefix).arg(QString::fromLatin1(method.methodSignature())).arg(QString::fromLatin1(methodTypeName));

        checkKnownType(methodTypeName);
        foreach (QByteArray paramType, method.parameterTypes())
            checkKnownType(paramType);
    }
}

static void gatherAPI(const QString &prefix, const QMetaObject *meta, QStringList *output)
{
    // *Offset points us only at the leaf class members, we don't have inheritance in our API yet anyway.
    for (int i = meta->enumeratorOffset(); i < meta->enumeratorCount(); ++i) {
        knownEnumNames << meta->enumerator(i).name();
        gatherAPI(prefix, meta->enumerator(i), output);
    }
    for (int i = meta->propertyOffset(); i < meta->propertyCount(); ++i)
        gatherAPI(prefix, meta->property(i), output);
    for (int i = meta->methodOffset(); i < meta->methodCount(); ++i)
        gatherAPI(prefix, meta->method(i), output);
}

void tst_publicapi::publicAPI()
{
    QStringList actualAPI;
    foreach (const QMetaObject *meta, typesToCheck)
        gatherAPI(QString::fromLatin1(meta->className()) + ".", meta, &actualAPI);

    // Uncomment to print the actual API.
    // foreach (QString actual, actualAPI)
    //     printf("    << \"%s\"\n", qPrintable(actual));

    // Make sure that nothing slips in the public API unintentionally.
    foreach (QString actual, actualAPI) {
        if (!expectedAPI.contains(actual))
            QEXPECT_FAIL("", qPrintable("Expected list is not up-to-date: " + actual), Continue);
        QVERIFY2(expectedAPI.contains(actual), qPrintable(actual));
    }
    // Make sure that the expected list is up-to-date with intentionally added APIs.
    foreach (QString expected, expectedAPI) {
        if (!actualAPI.contains(expected))
            QEXPECT_FAIL("", qPrintable("Not implemented: " + expected), Continue);
        QVERIFY2(actualAPI.contains(expected), qPrintable(expected));
    }
}

QTEST_MAIN(tst_publicapi)

#include "tst_publicapi.moc"

