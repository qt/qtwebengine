// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QMetaEnum>
#include <QMetaMethod>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaType>
#include <QQmlListProperty>
#include <QtTest/QtTest>
#include <QtWebEngineQuick/QQuickWebEngineProfile>
#include <QtWebEngineCore/QWebEngineCertificateError>
#include <QtWebEngineCore/QWebEngineFileSystemAccessRequest>
#include <QtWebEngineCore/QWebEngineFindTextResult>
#include <QtWebEngineCore/QWebEngineFullScreenRequest>
#include <QtWebEngineCore/QWebEngineHistory>
#include <QtWebEngineCore/QWebEngineNavigationRequest>
#include <QtWebEngineCore/QWebEngineNewWindowRequest>
#include <QtWebEngineCore/QWebEngineNotification>
#include <QtWebEngineCore/QWebEngineQuotaRequest>
#include <QtWebEngineCore/QWebEngineRegisterProtocolHandlerRequest>
#include <QtWebEngineCore/QWebEngineContextMenuRequest>
#include <QtWebEngineCore/QWebEngineDownloadRequest>
#include <QtWebEngineCore/QWebEngineScript>
#include <QtWebEngineCore/QWebEngineLoadingInfo>
#include <private/qquickwebengineview_p.h>
#include <private/qquickwebengineaction_p.h>
#include <private/qquickwebengineclientcertificateselection_p.h>
#include <private/qquickwebenginedialogrequests_p.h>
#include <private/qquickwebenginedownloadrequest_p.h>
#include <private/qquickwebenginenewwindowrequest_p.h>
#include <private/qquickwebenginesettings_p.h>
#include <private/qquickwebenginesingleton_p.h>
#include <private/qquickwebenginetouchselectionmenurequest_p.h>

class tst_publicapi : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void publicAPI();
};

static const QList<const QMetaObject *> typesToCheck = QList<const QMetaObject *>()
    << &QQuickWebEngineView::staticMetaObject
    << &QQuickWebEngineAction::staticMetaObject
    << &QQuickWebEngineClientCertificateOption::staticMetaObject
    << &QQuickWebEngineClientCertificateSelection::staticMetaObject
    << &QQuickWebEngineDownloadRequest::staticMetaObject
    << &QWebEngineDownloadRequest::staticMetaObject
    << &QWebEngineHistory::staticMetaObject
    << &QWebEngineHistoryModel::staticMetaObject
    << &QQuickWebEngineProfile::staticMetaObject
    << &QQuickWebEngineSettings::staticMetaObject
    << &QWebEngineFullScreenRequest::staticMetaObject
    << &QWebEngineScript::staticMetaObject
    << &QQuickWebEngineSingleton::staticMetaObject
    << &QQuickWebEngineAuthenticationDialogRequest::staticMetaObject
    << &QQuickWebEngineJavaScriptDialogRequest::staticMetaObject
    << &QQuickWebEngineColorDialogRequest::staticMetaObject
    << &QQuickWebEngineFileDialogRequest::staticMetaObject
    << &QQuickWebEngineNewWindowRequest::staticMetaObject
    << &QQuickWebEngineTooltipRequest::staticMetaObject
    << &QWebEngineContextMenuRequest::staticMetaObject
    << &QWebEngineCertificateError::staticMetaObject
    << &QWebEngineFileSystemAccessRequest::staticMetaObject
    << &QWebEngineFindTextResult::staticMetaObject
    << &QWebEngineLoadingInfo::staticMetaObject
    << &QWebEngineNavigationRequest::staticMetaObject
    << &QWebEngineNewWindowRequest::staticMetaObject
    << &QWebEngineNotification::staticMetaObject
    << &QWebEngineQuotaRequest::staticMetaObject
    << &QWebEngineRegisterProtocolHandlerRequest::staticMetaObject
    << &QQuickWebEngineTouchSelectionMenuRequest::staticMetaObject
    ;

static QList<QMetaEnum> knownEnumNames = QList<QMetaEnum>()
    << QWebEngineDownloadRequest::staticMetaObject.enumerator(QWebEngineDownloadRequest::staticMetaObject.indexOfEnumerator("SavePageFormat"))
    ;

static const QStringList hardcodedTypes = QStringList()
    << "QJSValue"
    << "QQmlListProperty<QWebEngineScript>"
    << "QQmlListProperty<QQuickWebEngineClientCertificateOption>"
    << "const QQuickWebEngineClientCertificateOption*"
    << "QQmlWebChannel*"
    << "const QQuickWebEngineContextMenuData*"
    << "QWebEngineCookieStore*"
    << "Qt::LayoutDirection"
    << "QQuickWebEngineScriptCollection*"
    << "QQmlComponent*"
    << "QMultiMap<QByteArray,QByteArray>";

static const QStringList expectedAPI = QStringList()
    << "QQuickWebEngineAction.text --> QString"
    << "QQuickWebEngineAction.iconName --> QString"
    << "QQuickWebEngineAction.enabled --> bool"
    << "QQuickWebEngineAction.triggered() --> void"
    << "QQuickWebEngineAction.enabledChanged() --> void"
    << "QQuickWebEngineAction.trigger() --> void"
    << "QQuickWebEngineAuthenticationDialogRequest.AuthenticationTypeHTTP --> AuthenticationType"
    << "QQuickWebEngineAuthenticationDialogRequest.AuthenticationTypeProxy --> AuthenticationType"
    << "QQuickWebEngineAuthenticationDialogRequest.accepted --> bool"
    << "QQuickWebEngineAuthenticationDialogRequest.dialogAccept(QString,QString) --> void"
    << "QQuickWebEngineAuthenticationDialogRequest.dialogReject() --> void"
    << "QQuickWebEngineAuthenticationDialogRequest.proxyHost --> QString"
    << "QQuickWebEngineAuthenticationDialogRequest.realm --> QString"
    << "QQuickWebEngineAuthenticationDialogRequest.type --> QQuickWebEngineAuthenticationDialogRequest::AuthenticationType"
    << "QQuickWebEngineAuthenticationDialogRequest.url --> QUrl"
    << "QWebEngineCertificateError.CertificateAuthorityInvalid --> Type"
    << "QWebEngineCertificateError.CertificateCommonNameInvalid --> Type"
    << "QWebEngineCertificateError.CertificateContainsErrors --> Type"
    << "QWebEngineCertificateError.CertificateDateInvalid --> Type"
    << "QWebEngineCertificateError.CertificateInvalid --> Type"
    << "QWebEngineCertificateError.CertificateKnownInterceptionBlocked --> Type"
    << "QWebEngineCertificateError.CertificateNameConstraintViolation --> Type"
    << "QWebEngineCertificateError.CertificateNoRevocationMechanism --> Type"
    << "QWebEngineCertificateError.CertificateNonUniqueName --> Type"
    << "QWebEngineCertificateError.CertificateRevoked --> Type"
    << "QWebEngineCertificateError.CertificateTransparencyRequired --> Type"
    << "QWebEngineCertificateError.CertificateUnableToCheckRevocation --> Type"
    << "QWebEngineCertificateError.CertificateValidityTooLong --> Type"
    << "QWebEngineCertificateError.CertificateWeakKey --> Type"
    << "QWebEngineCertificateError.CertificateWeakSignatureAlgorithm --> Type"
    << "QWebEngineCertificateError.CertificateSymantecLegacy --> Type"
    << "QWebEngineCertificateError.SslObsoleteVersion --> Type"
    << "QWebEngineCertificateError.SslPinnedKeyNotInCertificateChain --> Type"
    << "QWebEngineCertificateError.defer() --> void"
    << "QWebEngineCertificateError.description --> QString"
    << "QWebEngineCertificateError.type --> QWebEngineCertificateError::Type"
    << "QWebEngineCertificateError.acceptCertificate() --> void"
    << "QWebEngineCertificateError.overridable --> bool"
    << "QWebEngineCertificateError.rejectCertificate() --> void"
    << "QWebEngineCertificateError.url --> QUrl"
    << "QQuickWebEngineClientCertificateOption.issuer --> QString"
    << "QQuickWebEngineClientCertificateOption.subject --> QString"
    << "QQuickWebEngineClientCertificateOption.effectiveDate --> QDateTime"
    << "QQuickWebEngineClientCertificateOption.expiryDate --> QDateTime"
    << "QQuickWebEngineClientCertificateOption.isSelfSigned --> bool"
    << "QQuickWebEngineClientCertificateOption.select() --> void"
    << "QQuickWebEngineClientCertificateSelection.host --> QUrl"
    << "QQuickWebEngineClientCertificateSelection.certificates --> QQmlListProperty<QQuickWebEngineClientCertificateOption>"
    << "QQuickWebEngineClientCertificateSelection.select(int) --> void"
    << "QQuickWebEngineClientCertificateSelection.select(const QQuickWebEngineClientCertificateOption*) --> void"
    << "QQuickWebEngineClientCertificateSelection.selectNone() --> void"
    << "QQuickWebEngineColorDialogRequest.accepted --> bool"
    << "QQuickWebEngineColorDialogRequest.color --> QColor"
    << "QWebEngineContextMenuRequest.CanUndo --> EditFlags"
    << "QWebEngineContextMenuRequest.CanRedo --> EditFlags"
    << "QWebEngineContextMenuRequest.CanCut --> EditFlags"
    << "QWebEngineContextMenuRequest.CanCopy --> EditFlags"
    << "QWebEngineContextMenuRequest.CanPaste --> EditFlags"
    << "QWebEngineContextMenuRequest.CanDelete --> EditFlags"
    << "QWebEngineContextMenuRequest.CanSelectAll --> EditFlags"
    << "QWebEngineContextMenuRequest.CanTranslate --> EditFlags"
    << "QWebEngineContextMenuRequest.CanEditRichly --> EditFlags"
    << "QQuickWebEngineColorDialogRequest.dialogAccept(QColor) --> void"
    << "QQuickWebEngineColorDialogRequest.dialogReject() --> void"
    << "QWebEngineContextMenuRequest.editFlags --> QFlags<QWebEngineContextMenuRequest::EditFlag>"
    << "QWebEngineContextMenuRequest.MediaInError --> MediaFlags"
    << "QWebEngineContextMenuRequest.MediaPaused --> MediaFlags"
    << "QWebEngineContextMenuRequest.MediaMuted --> MediaFlags"
    << "QWebEngineContextMenuRequest.MediaLoop --> MediaFlags"
    << "QWebEngineContextMenuRequest.MediaCanSave --> MediaFlags"
    << "QWebEngineContextMenuRequest.MediaHasAudio --> MediaFlags"
    << "QWebEngineContextMenuRequest.MediaCanToggleControls --> MediaFlags"
    << "QWebEngineContextMenuRequest.MediaControls --> MediaFlags"
    << "QWebEngineContextMenuRequest.MediaCanPrint --> MediaFlags"
    << "QWebEngineContextMenuRequest.MediaCanRotate --> MediaFlags"
    << "QWebEngineContextMenuRequest.MediaTypeAudio --> MediaType"
    << "QWebEngineContextMenuRequest.MediaTypeCanvas --> MediaType"
    << "QWebEngineContextMenuRequest.MediaTypeFile --> MediaType"
    << "QWebEngineContextMenuRequest.MediaTypeImage --> MediaType"
    << "QWebEngineContextMenuRequest.MediaTypeNone --> MediaType"
    << "QWebEngineContextMenuRequest.MediaTypePlugin --> MediaType"
    << "QWebEngineContextMenuRequest.MediaTypeVideo --> MediaType"
    << "QWebEngineContextMenuRequest.accepted --> bool"
    << "QWebEngineContextMenuRequest.isContentEditable --> bool"
    << "QWebEngineContextMenuRequest.linkText --> QString"
    << "QWebEngineContextMenuRequest.linkUrl --> QUrl"
    << "QWebEngineContextMenuRequest.mediaFlags --> QFlags<QWebEngineContextMenuRequest::MediaFlag>"
    << "QWebEngineContextMenuRequest.mediaType --> QWebEngineContextMenuRequest::MediaType"
    << "QWebEngineContextMenuRequest.mediaUrl --> QUrl"
    << "QWebEngineContextMenuRequest.misspelledWord --> QString"
    << "QWebEngineContextMenuRequest.selectedText --> QString"
    << "QWebEngineContextMenuRequest.spellCheckerSuggestions --> QStringList"
    << "QWebEngineContextMenuRequest.position --> QPoint"
    << "QWebEngineDownloadRequest.CompleteHtmlSaveFormat --> SavePageFormat"
    << "QWebEngineDownloadRequest.DownloadCancelled --> DownloadState"
    << "QWebEngineDownloadRequest.DownloadCompleted --> DownloadState"
    << "QWebEngineDownloadRequest.DownloadInProgress --> DownloadState"
    << "QWebEngineDownloadRequest.DownloadInterrupted --> DownloadState"
    << "QWebEngineDownloadRequest.DownloadRequested --> DownloadState"
    << "QWebEngineDownloadRequest.FileAccessDenied --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.FileBlocked --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.FileFailed --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.FileHashMismatch --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.FileNameTooLong --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.FileNoSpace --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.FileSecurityCheckFailed --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.FileTooLarge --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.FileTooShort --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.FileTransientError --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.FileVirusInfected --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.MimeHtmlSaveFormat --> SavePageFormat"
    << "QWebEngineDownloadRequest.NetworkDisconnected --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.NetworkFailed --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.NetworkInvalidRequest --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.NetworkServerDown --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.NetworkTimeout --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.NoReason --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.ServerBadContent --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.ServerCertProblem --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.ServerFailed --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.ServerForbidden --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.ServerUnauthorized --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.ServerUnreachable --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.SingleHtmlSaveFormat --> SavePageFormat"
    << "QWebEngineDownloadRequest.UnknownSaveFormat --> SavePageFormat"
    << "QWebEngineDownloadRequest.UserCanceled --> DownloadInterruptReason"
    << "QWebEngineDownloadRequest.accept() --> void"
    << "QWebEngineDownloadRequest.cancel() --> void"
    << "QWebEngineDownloadRequest.id --> uint"
    << "QWebEngineDownloadRequest.interruptReason --> QWebEngineDownloadRequest::DownloadInterruptReason"
    << "QWebEngineDownloadRequest.interruptReasonChanged() --> void"
    << "QWebEngineDownloadRequest.interruptReasonString --> QString"
    << "QWebEngineDownloadRequest.isFinished --> bool"
    << "QWebEngineDownloadRequest.isFinishedChanged() --> void"
    << "QWebEngineDownloadRequest.isPaused --> bool"
    << "QWebEngineDownloadRequest.isPausedChanged() --> void"
    << "QWebEngineDownloadRequest.isSavePageDownload --> bool"
    << "QWebEngineDownloadRequest.mimeType --> QString"
    << "QWebEngineDownloadRequest.pause() --> void"
    << "QWebEngineDownloadRequest.receivedBytes --> qlonglong"
    << "QWebEngineDownloadRequest.receivedBytesChanged() --> void"
    << "QWebEngineDownloadRequest.resume() --> void"
    << "QWebEngineDownloadRequest.savePageFormat --> QWebEngineDownloadRequest::SavePageFormat"
    << "QWebEngineDownloadRequest.savePageFormatChanged() --> void"
    << "QWebEngineDownloadRequest.state --> QWebEngineDownloadRequest::DownloadState"
    << "QWebEngineDownloadRequest.stateChanged(QWebEngineDownloadRequest::DownloadState) --> void"
    << "QWebEngineDownloadRequest.totalBytes --> qlonglong"
    << "QWebEngineDownloadRequest.totalBytesChanged() --> void"
    << "QWebEngineDownloadRequest.url --> QUrl"
    << "QWebEngineDownloadRequest.suggestedFileName --> QString"
    << "QWebEngineDownloadRequest.downloadDirectory --> QString"
    << "QWebEngineDownloadRequest.downloadDirectoryChanged() --> void"
    << "QWebEngineDownloadRequest.downloadFileName --> QString"
    << "QWebEngineDownloadRequest.downloadFileNameChanged() --> void"
    << "QQuickWebEngineDownloadRequest.view --> QQuickWebEngineView*"
    << "QQuickWebEngineFileDialogRequest.FileModeOpen --> FileMode"
    << "QQuickWebEngineFileDialogRequest.FileModeOpenMultiple --> FileMode"
    << "QQuickWebEngineFileDialogRequest.FileModeSave --> FileMode"
    << "QQuickWebEngineFileDialogRequest.FileModeUploadFolder --> FileMode"
    << "QQuickWebEngineFileDialogRequest.accepted --> bool"
    << "QQuickWebEngineFileDialogRequest.acceptedMimeTypes --> QStringList"
    << "QQuickWebEngineFileDialogRequest.defaultFileName --> QString"
    << "QQuickWebEngineFileDialogRequest.dialogAccept(QStringList) --> void"
    << "QQuickWebEngineFileDialogRequest.dialogReject() --> void"
    << "QQuickWebEngineFileDialogRequest.mode --> QQuickWebEngineFileDialogRequest::FileMode"
    << "QWebEngineFindTextResult.numberOfMatches --> int"
    << "QWebEngineFindTextResult.activeMatch --> int"
    << "QQuickWebEngineTooltipRequest.Hide --> RequestType"
    << "QQuickWebEngineTooltipRequest.Show --> RequestType"
    << "QQuickWebEngineTooltipRequest.x --> int"
    << "QQuickWebEngineTooltipRequest.y --> int"
    << "QQuickWebEngineTooltipRequest.text --> QString"
    << "QQuickWebEngineTooltipRequest.type --> QQuickWebEngineTooltipRequest::RequestType"
    << "QQuickWebEngineTooltipRequest.accepted --> bool"
    << "QWebEngineFullScreenRequest.accept() --> void"
    << "QWebEngineFullScreenRequest.origin --> QUrl"
    << "QWebEngineFullScreenRequest.reject() --> void"
    << "QWebEngineFullScreenRequest.toggleOn --> bool"
    << "QWebEngineFileSystemAccessRequest.File --> HandleType"
    << "QWebEngineFileSystemAccessRequest.Directory --> HandleType"
    << "QWebEngineFileSystemAccessRequest.Read --> AccessFlags"
    << "QWebEngineFileSystemAccessRequest.Write --> AccessFlags"
    << "QWebEngineFileSystemAccessRequest.origin --> QUrl"
    << "QWebEngineFileSystemAccessRequest.filePath --> QUrl"
    << "QWebEngineFileSystemAccessRequest.handleType --> QWebEngineFileSystemAccessRequest::HandleType"
    << "QWebEngineFileSystemAccessRequest.accessFlags --> QFlags<QWebEngineFileSystemAccessRequest::AccessFlag>"
    << "QWebEngineFileSystemAccessRequest.accept() --> void"
    << "QWebEngineFileSystemAccessRequest.reject() --> void"
    << "QWebEngineHistory.backItems --> QWebEngineHistoryModel*"
    << "QWebEngineHistory.clear() --> void"
    << "QWebEngineHistory.forwardItems --> QWebEngineHistoryModel*"
    << "QWebEngineHistory.items --> QWebEngineHistoryModel*"
    << "QQuickWebEngineJavaScriptDialogRequest.DialogTypeAlert --> DialogType"
    << "QQuickWebEngineJavaScriptDialogRequest.DialogTypeBeforeUnload --> DialogType"
    << "QQuickWebEngineJavaScriptDialogRequest.DialogTypeConfirm --> DialogType"
    << "QQuickWebEngineJavaScriptDialogRequest.DialogTypePrompt --> DialogType"
    << "QQuickWebEngineJavaScriptDialogRequest.accepted --> bool"
    << "QQuickWebEngineJavaScriptDialogRequest.defaultText --> QString"
    << "QQuickWebEngineJavaScriptDialogRequest.dialogAccept() --> void"
    << "QQuickWebEngineJavaScriptDialogRequest.dialogAccept(QString) --> void"
    << "QQuickWebEngineJavaScriptDialogRequest.dialogReject() --> void"
    << "QQuickWebEngineJavaScriptDialogRequest.message --> QString"
    << "QQuickWebEngineJavaScriptDialogRequest.securityOrigin --> QUrl"
    << "QQuickWebEngineJavaScriptDialogRequest.title --> QString"
    << "QQuickWebEngineJavaScriptDialogRequest.type --> QQuickWebEngineJavaScriptDialogRequest::DialogType"
    << "QWebEngineLoadingInfo.errorCode --> int"
    << "QWebEngineLoadingInfo.responseHeaders --> QMultiMap<QByteArray,QByteArray>"
    << "QWebEngineLoadingInfo.errorDomain --> QWebEngineLoadingInfo::ErrorDomain"
    << "QWebEngineLoadingInfo.errorString --> QString"
    << "QWebEngineLoadingInfo.status --> QWebEngineLoadingInfo::LoadStatus"
    << "QWebEngineLoadingInfo.url --> QUrl"
    << "QWebEngineLoadingInfo.isErrorPage --> bool"
    << "QWebEngineLoadingInfo.LoadFailedStatus --> LoadStatus"
    << "QWebEngineLoadingInfo.LoadStartedStatus --> LoadStatus"
    << "QWebEngineLoadingInfo.LoadStoppedStatus --> LoadStatus"
    << "QWebEngineLoadingInfo.LoadSucceededStatus --> LoadStatus"
    << "QWebEngineLoadingInfo.HttpStatusCodeDomain --> ErrorDomain"
    << "QWebEngineLoadingInfo.CertificateErrorDomain --> ErrorDomain"
    << "QWebEngineLoadingInfo.ConnectionErrorDomain --> ErrorDomain"
    << "QWebEngineLoadingInfo.DnsErrorDomain --> ErrorDomain"
    << "QWebEngineLoadingInfo.FtpErrorDomain --> ErrorDomain"
    << "QWebEngineLoadingInfo.HttpErrorDomain --> ErrorDomain"
    << "QWebEngineLoadingInfo.InternalErrorDomain --> ErrorDomain"
    << "QWebEngineLoadingInfo.NoErrorDomain --> ErrorDomain"
    << "QWebEngineNavigationRequest.action --> QWebEngineNavigationRequest::NavigationRequestAction"
    << "QWebEngineNavigationRequest.actionChanged() --> void"
    << "QWebEngineNavigationRequest.isMainFrame --> bool"
    << "QWebEngineNavigationRequest.navigationType --> QWebEngineNavigationRequest::NavigationType"
    << "QWebEngineNavigationRequest.url --> QUrl"
    << "QWebEngineNavigationRequest.AcceptRequest --> NavigationRequestAction"
    << "QWebEngineNavigationRequest.IgnoreRequest --> NavigationRequestAction"
    << "QWebEngineNavigationRequest.BackForwardNavigation --> NavigationType"
    << "QWebEngineNavigationRequest.FormSubmittedNavigation --> NavigationType"
    << "QWebEngineNavigationRequest.LinkClickedNavigation --> NavigationType"
    << "QWebEngineNavigationRequest.OtherNavigation --> NavigationType"
    << "QWebEngineNavigationRequest.RedirectNavigation --> NavigationType"
    << "QWebEngineNavigationRequest.ReloadNavigation --> NavigationType"
    << "QWebEngineNavigationRequest.TypedNavigation --> NavigationType"
    << "QWebEngineNavigationRequest.accept() --> void"
    << "QWebEngineNavigationRequest.reject() --> void"
    << "QWebEngineNewWindowRequest.destination --> QWebEngineNewWindowRequest::DestinationType"
    << "QWebEngineNewWindowRequest.requestedUrl --> QUrl"
    << "QWebEngineNewWindowRequest.requestedGeometry --> QRect"
    << "QWebEngineNewWindowRequest.userInitiated --> bool"
    << "QWebEngineNewWindowRequest.InNewBackgroundTab --> DestinationType"
    << "QWebEngineNewWindowRequest.InNewDialog --> DestinationType"
    << "QWebEngineNewWindowRequest.InNewTab --> DestinationType"
    << "QWebEngineNewWindowRequest.InNewWindow --> DestinationType"
    << "QQuickWebEngineNewWindowRequest.openIn(QQuickWebEngineView*) --> void"
    << "QQuickWebEngineProfile.AllowPersistentCookies --> PersistentCookiesPolicy"
    << "QQuickWebEngineProfile.DiskHttpCache --> HttpCacheType"
    << "QQuickWebEngineProfile.ForcePersistentCookies --> PersistentCookiesPolicy"
    << "QQuickWebEngineProfile.MemoryHttpCache --> HttpCacheType"
    << "QQuickWebEngineProfile.NoCache --> HttpCacheType"
    << "QQuickWebEngineProfile.NoPersistentCookies --> PersistentCookiesPolicy"
    << "QQuickWebEngineProfile.cachePath --> QString"
    << "QQuickWebEngineProfile.cachePathChanged() --> void"
    << "QQuickWebEngineProfile.clearHttpCache() --> void"
    << "QQuickWebEngineProfile.downloadFinished(QQuickWebEngineDownloadRequest*) --> void"
    << "QQuickWebEngineProfile.downloadRequested(QQuickWebEngineDownloadRequest*) --> void"
    << "QQuickWebEngineProfile.downloadPath --> QString"
    << "QQuickWebEngineProfile.downloadPathChanged() --> void"
    << "QQuickWebEngineProfile.presentNotification(QWebEngineNotification*) --> void"
    << "QQuickWebEngineProfile.httpAcceptLanguage --> QString"
    << "QQuickWebEngineProfile.httpAcceptLanguageChanged() --> void"
    << "QQuickWebEngineProfile.httpCacheMaximumSize --> int"
    << "QQuickWebEngineProfile.httpCacheMaximumSizeChanged() --> void"
    << "QQuickWebEngineProfile.httpCacheType --> QQuickWebEngineProfile::HttpCacheType"
    << "QQuickWebEngineProfile.httpCacheTypeChanged() --> void"
    << "QQuickWebEngineProfile.httpUserAgent --> QString"
    << "QQuickWebEngineProfile.httpUserAgentChanged() --> void"
    << "QQuickWebEngineProfile.offTheRecord --> bool"
    << "QQuickWebEngineProfile.offTheRecordChanged() --> void"
    << "QQuickWebEngineProfile.persistentCookiesPolicy --> QQuickWebEngineProfile::PersistentCookiesPolicy"
    << "QQuickWebEngineProfile.persistentCookiesPolicyChanged() --> void"
    << "QQuickWebEngineProfile.persistentStoragePath --> QString"
    << "QQuickWebEngineProfile.persistentStoragePathChanged() --> void"
    << "QQuickWebEngineProfile.isPushServiceEnabled --> bool"
    << "QQuickWebEngineProfile.pushServiceEnabledChanged() --> void"
    << "QQuickWebEngineProfile.spellCheckEnabled --> bool"
    << "QQuickWebEngineProfile.spellCheckEnabledChanged() --> void"
    << "QQuickWebEngineProfile.spellCheckLanguages --> QStringList"
    << "QQuickWebEngineProfile.spellCheckLanguagesChanged() --> void"
    << "QQuickWebEngineProfile.storageName --> QString"
    << "QQuickWebEngineProfile.storageNameChanged() --> void"
    << "QQuickWebEngineProfile.userScripts --> QQuickWebEngineScriptCollection*"
    << "QQuickWebEngineSettings.AllowAllUnknownUrlSchemes --> UnknownUrlSchemePolicy"
    << "QQuickWebEngineSettings.AllowUnknownUrlSchemesFromUserInteraction --> UnknownUrlSchemePolicy"
    << "QQuickWebEngineSettings.DisallowUnknownUrlSchemes --> UnknownUrlSchemePolicy"
    << "QQuickWebEngineSettings.accelerated2dCanvasEnabled --> bool"
    << "QQuickWebEngineSettings.accelerated2dCanvasEnabledChanged() --> void"
    << "QQuickWebEngineSettings.allowGeolocationOnInsecureOrigins --> bool"
    << "QQuickWebEngineSettings.allowGeolocationOnInsecureOriginsChanged() --> void"
    << "QQuickWebEngineSettings.allowRunningInsecureContent --> bool"
    << "QQuickWebEngineSettings.allowRunningInsecureContentChanged() --> void"
    << "QQuickWebEngineSettings.allowWindowActivationFromJavaScript --> bool"
    << "QQuickWebEngineSettings.allowWindowActivationFromJavaScriptChanged() --> void"
    << "QQuickWebEngineSettings.autoLoadIconsForPage --> bool"
    << "QQuickWebEngineSettings.autoLoadIconsForPageChanged() --> void"
    << "QQuickWebEngineSettings.autoLoadImages --> bool"
    << "QQuickWebEngineSettings.autoLoadImagesChanged() --> void"
    << "QQuickWebEngineSettings.defaultTextEncoding --> QString"
    << "QQuickWebEngineSettings.defaultTextEncodingChanged() --> void"
    << "QQuickWebEngineSettings.dnsPrefetchEnabled --> bool"
    << "QQuickWebEngineSettings.dnsPrefetchEnabledChanged() --> void"
    << "QQuickWebEngineSettings.errorPageEnabled --> bool"
    << "QQuickWebEngineSettings.errorPageEnabledChanged() --> void"
    << "QQuickWebEngineSettings.focusOnNavigationEnabled --> bool"
    << "QQuickWebEngineSettings.focusOnNavigationEnabledChanged() --> void"
    << "QQuickWebEngineSettings.fullScreenSupportEnabled --> bool"
    << "QQuickWebEngineSettings.fullScreenSupportEnabledChanged() --> void"
    << "QQuickWebEngineSettings.hyperlinkAuditingEnabled --> bool"
    << "QQuickWebEngineSettings.hyperlinkAuditingEnabledChanged() --> void"
    << "QQuickWebEngineSettings.javascriptCanAccessClipboard --> bool"
    << "QQuickWebEngineSettings.javascriptCanAccessClipboardChanged() --> void"
    << "QQuickWebEngineSettings.javascriptCanOpenWindows --> bool"
    << "QQuickWebEngineSettings.javascriptCanOpenWindowsChanged() --> void"
    << "QQuickWebEngineSettings.javascriptCanPaste --> bool"
    << "QQuickWebEngineSettings.javascriptCanPasteChanged() --> void"
    << "QQuickWebEngineSettings.javascriptEnabled --> bool"
    << "QQuickWebEngineSettings.javascriptEnabledChanged() --> void"
    << "QQuickWebEngineSettings.linksIncludedInFocusChain --> bool"
    << "QQuickWebEngineSettings.linksIncludedInFocusChainChanged() --> void"
    << "QQuickWebEngineSettings.localContentCanAccessFileUrls --> bool"
    << "QQuickWebEngineSettings.localContentCanAccessFileUrlsChanged() --> void"
    << "QQuickWebEngineSettings.localContentCanAccessRemoteUrls --> bool"
    << "QQuickWebEngineSettings.localContentCanAccessRemoteUrlsChanged() --> void"
    << "QQuickWebEngineSettings.localStorageEnabled --> bool"
    << "QQuickWebEngineSettings.localStorageEnabledChanged() --> void"
    << "QQuickWebEngineSettings.navigateOnDropEnabled --> bool"
    << "QQuickWebEngineSettings.navigateOnDropEnabledChanged() --> void"
    << "QQuickWebEngineSettings.pdfViewerEnabled --> bool"
    << "QQuickWebEngineSettings.pdfViewerEnabledChanged() --> void"
    << "QQuickWebEngineSettings.playbackRequiresUserGesture --> bool"
    << "QQuickWebEngineSettings.playbackRequiresUserGestureChanged() --> void"
    << "QQuickWebEngineSettings.pluginsEnabled --> bool"
    << "QQuickWebEngineSettings.pluginsEnabledChanged() --> void"
    << "QQuickWebEngineSettings.printElementBackgrounds --> bool"
    << "QQuickWebEngineSettings.printElementBackgroundsChanged() --> void"
    << "QQuickWebEngineSettings.screenCaptureEnabled --> bool"
    << "QQuickWebEngineSettings.screenCaptureEnabledChanged() --> void"
    << "QQuickWebEngineSettings.showScrollBars --> bool"
    << "QQuickWebEngineSettings.showScrollBarsChanged() --> void"
    << "QQuickWebEngineSettings.spatialNavigationEnabled --> bool"
    << "QQuickWebEngineSettings.spatialNavigationEnabledChanged() --> void"
    << "QQuickWebEngineSettings.touchIconsEnabled --> bool"
    << "QQuickWebEngineSettings.touchIconsEnabledChanged() --> void"
    << "QQuickWebEngineSettings.unknownUrlSchemePolicy --> QQuickWebEngineSettings::UnknownUrlSchemePolicy"
    << "QQuickWebEngineSettings.unknownUrlSchemePolicyChanged() --> void"
    << "QQuickWebEngineSettings.webGLEnabled --> bool"
    << "QQuickWebEngineSettings.webGLEnabledChanged() --> void"
    << "QQuickWebEngineSettings.webRTCPublicInterfacesOnly --> bool"
    << "QQuickWebEngineSettings.webRTCPublicInterfacesOnlyChanged() --> void"
    << "QQuickWebEngineSettings.readingFromCanvasEnabled --> bool"
    << "QQuickWebEngineSettings.readingFromCanvasEnabledChanged() --> void"
    << "QQuickWebEngineSingleton.defaultProfile --> QQuickWebEngineProfile*"
    << "QQuickWebEngineSingleton.settings --> QQuickWebEngineSettings*"
    << "QQuickWebEngineSingleton.script() --> QWebEngineScript"
    << "QQuickWebEngineTouchSelectionMenuRequest.accepted --> bool"
    << "QQuickWebEngineTouchSelectionMenuRequest.Cut --> TouchSelectionCommandFlags"
    << "QQuickWebEngineTouchSelectionMenuRequest.Copy --> TouchSelectionCommandFlags"
    << "QQuickWebEngineTouchSelectionMenuRequest.Paste --> TouchSelectionCommandFlags"
    << "QQuickWebEngineTouchSelectionMenuRequest.selectionBounds --> QRect"
    << "QQuickWebEngineTouchSelectionMenuRequest.touchSelectionCommandFlags --> QFlags<QQuickWebEngineTouchSelectionMenuRequest::TouchSelectionCommandFlag>"
    << "QWebEngineScript.ApplicationWorld --> ScriptWorldId"
    << "QWebEngineScript.Deferred --> InjectionPoint"
    << "QWebEngineScript.DocumentCreation --> InjectionPoint"
    << "QWebEngineScript.DocumentReady --> InjectionPoint"
    << "QWebEngineScript.MainWorld --> ScriptWorldId"
    << "QWebEngineScript.UserWorld --> ScriptWorldId"
    << "QWebEngineScript.injectionPoint --> QWebEngineScript::InjectionPoint"
    << "QWebEngineScript.name --> QString"
    << "QWebEngineScript.runsOnSubFrames --> bool"
    << "QWebEngineScript.sourceCode --> QString"
    << "QWebEngineScript.sourceUrl --> QUrl"
    << "QWebEngineScript.worldId --> uint"
    << "QQuickWebEngineView.action(WebAction) --> QQuickWebEngineAction*"
    << "QQuickWebEngineView.A0 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A1 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A10 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A2 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A3 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A3Extra --> PrintedPageSizeId"
    << "QQuickWebEngineView.A4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A4Extra --> PrintedPageSizeId"
    << "QQuickWebEngineView.A4Plus --> PrintedPageSizeId"
    << "QQuickWebEngineView.A4Small --> PrintedPageSizeId"
    << "QQuickWebEngineView.A5 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A5Extra --> PrintedPageSizeId"
    << "QQuickWebEngineView.A6 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A7 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A8 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A9 --> PrintedPageSizeId"
    << "QQuickWebEngineView.AbnormalTerminationStatus --> RenderProcessTerminationStatus"
    << "QQuickWebEngineView.AlignCenter --> WebAction"
    << "QQuickWebEngineView.AlignJustified --> WebAction"
    << "QQuickWebEngineView.AlignLeft --> WebAction"
    << "QQuickWebEngineView.AlignRight --> WebAction"
    << "QQuickWebEngineView.AnsiA --> PrintedPageSizeId"
    << "QQuickWebEngineView.AnsiB --> PrintedPageSizeId"
    << "QQuickWebEngineView.AnsiC --> PrintedPageSizeId"
    << "QQuickWebEngineView.AnsiD --> PrintedPageSizeId"
    << "QQuickWebEngineView.AnsiE --> PrintedPageSizeId"
    << "QQuickWebEngineView.ArchA --> PrintedPageSizeId"
    << "QQuickWebEngineView.ArchB --> PrintedPageSizeId"
    << "QQuickWebEngineView.ArchC --> PrintedPageSizeId"
    << "QQuickWebEngineView.ArchD --> PrintedPageSizeId"
    << "QQuickWebEngineView.ArchE --> PrintedPageSizeId"
    << "QQuickWebEngineView.B0 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B1 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B10 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B2 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B3 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B5 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B5Extra --> PrintedPageSizeId"
    << "QQuickWebEngineView.B6 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B7 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B8 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B9 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Back --> WebAction"
    << "QQuickWebEngineView.C5E --> PrintedPageSizeId"
    << "QQuickWebEngineView.CertificateErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.ChangeTextDirectionLTR --> WebAction"
    << "QQuickWebEngineView.ChangeTextDirectionRTL --> WebAction"
    << "QQuickWebEngineView.Comm10E --> PrintedPageSizeId"
    << "QQuickWebEngineView.ConnectionErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.Copy --> WebAction"
    << "QQuickWebEngineView.CopyImageToClipboard --> WebAction"
    << "QQuickWebEngineView.CopyImageUrlToClipboard --> WebAction"
    << "QQuickWebEngineView.CopyLinkToClipboard --> WebAction"
    << "QQuickWebEngineView.CopyMediaUrlToClipboard --> WebAction"
    << "QQuickWebEngineView.CrashedTerminationStatus --> RenderProcessTerminationStatus"
    << "QQuickWebEngineView.Custom --> PrintedPageSizeId"
    << "QQuickWebEngineView.Cut --> WebAction"
    << "QQuickWebEngineView.DLE --> PrintedPageSizeId"
    << "QQuickWebEngineView.DesktopAudioVideoCapture --> Feature"
    << "QQuickWebEngineView.DesktopVideoCapture --> Feature"
    << "QQuickWebEngineView.DnsErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.DoublePostcard --> PrintedPageSizeId"
    << "QQuickWebEngineView.DownloadImageToDisk --> WebAction"
    << "QQuickWebEngineView.DownloadLinkToDisk --> WebAction"
    << "QQuickWebEngineView.DownloadMediaToDisk --> WebAction"
    << "QQuickWebEngineView.Envelope10 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Envelope11 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Envelope12 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Envelope14 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Envelope9 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeB4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeB5 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeB6 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC0 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC1 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC2 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC3 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC5 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC6 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC65 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC7 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeChou3 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeChou4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeDL --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeInvite --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeItalian --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeKaku2 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeKaku3 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeMonarch --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePersonal --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc1 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc10 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc2 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc3 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc5 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc6 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc7 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc8 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc9 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeYou4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.ErrorMessageLevel --> JavaScriptConsoleMessageLevel"
    << "QQuickWebEngineView.Executive --> PrintedPageSizeId"
    << "QQuickWebEngineView.ExecutiveStandard --> PrintedPageSizeId"
    << "QQuickWebEngineView.ExitFullScreen --> WebAction"
    << "QQuickWebEngineView.FanFoldGerman --> PrintedPageSizeId"
    << "QQuickWebEngineView.FanFoldGermanLegal --> PrintedPageSizeId"
    << "QQuickWebEngineView.FanFoldUS --> PrintedPageSizeId"
    << "QQuickWebEngineView.FindBackward --> FindFlags"
    << "QQuickWebEngineView.FindCaseSensitively --> FindFlags"
    << "QQuickWebEngineView.Folio --> PrintedPageSizeId"
    << "QQuickWebEngineView.Forward --> WebAction"
    << "QQuickWebEngineView.FtpErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.Geolocation --> Feature"
    << "QQuickWebEngineView.HttpErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.Imperial10x11 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial10x13 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial10x14 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial12x11 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial15x11 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial7x9 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial8x10 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial9x11 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial9x12 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Indent --> WebAction"
    << "QQuickWebEngineView.InfoMessageLevel --> JavaScriptConsoleMessageLevel"
    << "QQuickWebEngineView.InsertOrderedList --> WebAction"
    << "QQuickWebEngineView.InsertUnorderedList --> WebAction"
    << "QQuickWebEngineView.InspectElement --> WebAction"
    << "QQuickWebEngineView.InternalErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.JisB0 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB1 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB10 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB2 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB3 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB5 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB6 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB7 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB8 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB9 --> PrintedPageSizeId"
    << "QQuickWebEngineView.KilledTerminationStatus --> RenderProcessTerminationStatus"
    << "QQuickWebEngineView.Landscape --> PrintedPageOrientation"
    << "QQuickWebEngineView.LastPageSize --> PrintedPageSizeId"
    << "QQuickWebEngineView.Ledger --> PrintedPageSizeId"
    << "QQuickWebEngineView.Legal --> PrintedPageSizeId"
    << "QQuickWebEngineView.LegalExtra --> PrintedPageSizeId"
    << "QQuickWebEngineView.Letter --> PrintedPageSizeId"
    << "QQuickWebEngineView.LetterExtra --> PrintedPageSizeId"
    << "QQuickWebEngineView.LetterPlus --> PrintedPageSizeId"
    << "QQuickWebEngineView.LetterSmall --> PrintedPageSizeId"
    << "QQuickWebEngineView.LifecycleState.Active --> LifecycleState"
    << "QQuickWebEngineView.LifecycleState.Discarded --> LifecycleState"
    << "QQuickWebEngineView.LifecycleState.Frozen --> LifecycleState"
    << "QQuickWebEngineView.LoadFailedStatus --> LoadStatus"
    << "QQuickWebEngineView.LoadStartedStatus --> LoadStatus"
    << "QQuickWebEngineView.LoadStoppedStatus --> LoadStatus"
    << "QQuickWebEngineView.LoadSucceededStatus --> LoadStatus"
    << "QQuickWebEngineView.MediaAudioCapture --> Feature"
    << "QQuickWebEngineView.MediaAudioVideoCapture --> Feature"
    << "QQuickWebEngineView.MediaVideoCapture --> Feature"
    << "QQuickWebEngineView.NPageSize --> PrintedPageSizeId"
    << "QQuickWebEngineView.NPaperSize --> PrintedPageSizeId"
    << "QQuickWebEngineView.NoErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.Notifications --> Feature"
    << "QQuickWebEngineView.NoWebAction --> WebAction"
    << "QQuickWebEngineView.NormalTerminationStatus --> RenderProcessTerminationStatus"
    << "QQuickWebEngineView.Note --> PrintedPageSizeId"
    << "QQuickWebEngineView.OpenLinkInNewTab --> WebAction"
    << "QQuickWebEngineView.OpenLinkInNewWindow --> WebAction"
    << "QQuickWebEngineView.OpenLinkInThisWindow --> WebAction"
    << "QQuickWebEngineView.Outdent --> WebAction"
    << "QQuickWebEngineView.Paste --> WebAction"
    << "QQuickWebEngineView.PasteAndMatchStyle --> WebAction"
    << "QQuickWebEngineView.Portrait --> PrintedPageOrientation"
    << "QQuickWebEngineView.Postcard --> PrintedPageSizeId"
    << "QQuickWebEngineView.Prc16K --> PrintedPageSizeId"
    << "QQuickWebEngineView.Prc32K --> PrintedPageSizeId"
    << "QQuickWebEngineView.Prc32KBig --> PrintedPageSizeId"
    << "QQuickWebEngineView.Quarto --> PrintedPageSizeId"
    << "QQuickWebEngineView.Redo --> WebAction"
    << "QQuickWebEngineView.Reload --> WebAction"
    << "QQuickWebEngineView.ReloadAndBypassCache --> WebAction"
    << "QQuickWebEngineView.RequestClose --> WebAction"
    << "QQuickWebEngineView.SavePage --> WebAction"
    << "QQuickWebEngineView.SelectAll --> WebAction"
    << "QQuickWebEngineView.Statement --> PrintedPageSizeId"
    << "QQuickWebEngineView.Stop --> WebAction"
    << "QQuickWebEngineView.SuperA --> PrintedPageSizeId"
    << "QQuickWebEngineView.SuperB --> PrintedPageSizeId"
    << "QQuickWebEngineView.Tabloid --> PrintedPageSizeId"
    << "QQuickWebEngineView.TabloidExtra --> PrintedPageSizeId"
    << "QQuickWebEngineView.ToggleBold --> WebAction"
    << "QQuickWebEngineView.ToggleItalic --> WebAction"
    << "QQuickWebEngineView.ToggleMediaControls --> WebAction"
    << "QQuickWebEngineView.ToggleMediaLoop --> WebAction"
    << "QQuickWebEngineView.ToggleMediaMute --> WebAction"
    << "QQuickWebEngineView.ToggleMediaPlayPause --> WebAction"
    << "QQuickWebEngineView.ToggleStrikethrough --> WebAction"
    << "QQuickWebEngineView.ToggleUnderline --> WebAction"
    << "QQuickWebEngineView.Undo --> WebAction"
    << "QQuickWebEngineView.Unselect --> WebAction"
    << "QQuickWebEngineView.OpenLinkInNewBackgroundTab --> WebAction"
    << "QQuickWebEngineView.ViewSource --> WebAction"
    << "QQuickWebEngineView.WarningMessageLevel --> JavaScriptConsoleMessageLevel"
    << "QQuickWebEngineView.WebActionCount --> WebAction"
    << "QQuickWebEngineView.activeFocusOnPress --> bool"
    << "QQuickWebEngineView.activeFocusOnPressChanged(bool) --> void"
    << "QQuickWebEngineView.audioMuted --> bool"
    << "QQuickWebEngineView.audioMutedChanged(bool) --> void"
    << "QQuickWebEngineView.authenticationDialogRequested(QQuickWebEngineAuthenticationDialogRequest*) --> void"
    << "QQuickWebEngineView.backgroundColor --> QColor"
    << "QQuickWebEngineView.backgroundColorChanged() --> void"
    << "QQuickWebEngineView.canGoBack --> bool"
    << "QQuickWebEngineView.canGoBackChanged() --> void"
    << "QQuickWebEngineView.canGoForward --> bool"
    << "QQuickWebEngineView.canGoForwardChanged() --> void"
    << "QQuickWebEngineView.certificateError(QWebEngineCertificateError) --> void"
    << "QQuickWebEngineView.colorDialogRequested(QQuickWebEngineColorDialogRequest*) --> void"
    << "QQuickWebEngineView.contentsSize --> QSizeF"
    << "QQuickWebEngineView.contentsSizeChanged(QSizeF) --> void"
    << "QQuickWebEngineView.contextMenuRequested(QWebEngineContextMenuRequest*) --> void"
    << "QQuickWebEngineView.devToolsId --> QString"
    << "QQuickWebEngineView.devToolsView --> QQuickWebEngineView*"
    << "QQuickWebEngineView.devToolsViewChanged() --> void"
    << "QQuickWebEngineView.featurePermissionRequested(QUrl,QQuickWebEngineView::Feature) --> void"
    << "QQuickWebEngineView.fileDialogRequested(QQuickWebEngineFileDialogRequest*) --> void"
    << "QQuickWebEngineView.fileSystemAccessRequested(QWebEngineFileSystemAccessRequest) --> void"
    << "QQuickWebEngineView.findText(QString) --> void"
    << "QQuickWebEngineView.findText(QString,FindFlags) --> void"
    << "QQuickWebEngineView.findText(QString,FindFlags,QJSValue) --> void"
    << "QQuickWebEngineView.findTextFinished(QWebEngineFindTextResult) --> void"
    << "QQuickWebEngineView.fullScreenCancelled() --> void"
    << "QQuickWebEngineView.fullScreenRequested(QWebEngineFullScreenRequest) --> void"
    << "QQuickWebEngineView.geometryChangeRequested(QRect,QRect) --> void"
    << "QQuickWebEngineView.goBack() --> void"
    << "QQuickWebEngineView.goBackOrForward(int) --> void"
    << "QQuickWebEngineView.goForward() --> void"
    << "QQuickWebEngineView.grantFeaturePermission(QUrl,QQuickWebEngineView::Feature,bool) --> void"
    << "QQuickWebEngineView.history --> QWebEngineHistory*"
    << "QQuickWebEngineView.icon --> QUrl"
    << "QQuickWebEngineView.iconChanged() --> void"
    << "QQuickWebEngineView.inspectedView --> QQuickWebEngineView*"
    << "QQuickWebEngineView.inspectedViewChanged() --> void"
    << "QQuickWebEngineView.isFullScreen --> bool"
    << "QQuickWebEngineView.isFullScreenChanged() --> void"
    << "QQuickWebEngineView.javaScriptConsoleMessage(QQuickWebEngineView::JavaScriptConsoleMessageLevel,QString,int,QString) --> void"
    << "QQuickWebEngineView.javaScriptDialogRequested(QQuickWebEngineJavaScriptDialogRequest*) --> void"
    << "QQuickWebEngineView.lifecycleState --> QQuickWebEngineView::LifecycleState"
    << "QQuickWebEngineView.lifecycleStateChanged(QQuickWebEngineView::LifecycleState) --> void"
    << "QQuickWebEngineView.linkHovered(QUrl) --> void"
    << "QQuickWebEngineView.loadHtml(QString) --> void"
    << "QQuickWebEngineView.loadHtml(QString,QUrl) --> void"
    << "QQuickWebEngineView.loadProgress --> int"
    << "QQuickWebEngineView.loadProgressChanged() --> void"
    << "QQuickWebEngineView.loading --> bool"
    << "QQuickWebEngineView.loadingChanged(QWebEngineLoadingInfo) --> void"
    << "QQuickWebEngineView.navigationRequested(QWebEngineNavigationRequest*) --> void"
    << "QQuickWebEngineView.newWindowRequested(QQuickWebEngineNewWindowRequest*) --> void"
    << "QQuickWebEngineView.AcceptRequest --> NavigationRequestAction"
    << "QQuickWebEngineView.IgnoreRequest --> NavigationRequestAction"
    << "QQuickWebEngineView.BackForwardNavigation --> NavigationType"
    << "QQuickWebEngineView.FormSubmittedNavigation --> NavigationType"
    << "QQuickWebEngineView.LinkClickedNavigation --> NavigationType"
    << "QQuickWebEngineView.OtherNavigation --> NavigationType"
    << "QQuickWebEngineView.RedirectNavigation --> NavigationType"
    << "QQuickWebEngineView.ReloadNavigation --> NavigationType"
    << "QQuickWebEngineView.TypedNavigation --> NavigationType"
    << "QQuickWebEngineView.NewViewInBackgroundTab --> NewViewDestination"
    << "QQuickWebEngineView.NewViewInDialog --> NewViewDestination"
    << "QQuickWebEngineView.NewViewInTab --> NewViewDestination"
    << "QQuickWebEngineView.NewViewInWindow --> NewViewDestination"
    << "QQuickWebEngineView.pdfPrintingFinished(QString,bool) --> void"
    << "QQuickWebEngineView.printRequested() --> void"
    << "QQuickWebEngineView.printToPdf(QJSValue) --> void"
    << "QQuickWebEngineView.printToPdf(QJSValue,PrintedPageSizeId) --> void"
    << "QQuickWebEngineView.printToPdf(QJSValue,PrintedPageSizeId,PrintedPageOrientation) --> void"
    << "QQuickWebEngineView.printToPdf(QString) --> void"
    << "QQuickWebEngineView.printToPdf(QString,PrintedPageSizeId) --> void"
    << "QQuickWebEngineView.printToPdf(QString,PrintedPageSizeId,PrintedPageOrientation) --> void"
    << "QQuickWebEngineView.profile --> QQuickWebEngineProfile*"
    << "QQuickWebEngineView.profileChanged() --> void"
    << "QQuickWebEngineView.quotaRequested(QWebEngineQuotaRequest) --> void"
    << "QQuickWebEngineView.recentlyAudible --> bool"
    << "QQuickWebEngineView.recentlyAudibleChanged(bool) --> void"
    << "QQuickWebEngineView.renderProcessPid --> qlonglong"
    << "QQuickWebEngineView.renderProcessPidChanged(qlonglong) --> void"
    << "QQuickWebEngineView.recommendedState --> QQuickWebEngineView::LifecycleState"
    << "QQuickWebEngineView.recommendedStateChanged(QQuickWebEngineView::LifecycleState) --> void"
    << "QQuickWebEngineView.registerProtocolHandlerRequested(QWebEngineRegisterProtocolHandlerRequest) --> void"
    << "QQuickWebEngineView.reload() --> void"
    << "QQuickWebEngineView.reloadAndBypassCache() --> void"
    << "QQuickWebEngineView.renderProcessTerminated(QQuickWebEngineView::RenderProcessTerminationStatus,int) --> void"
    << "QQuickWebEngineView.replaceMisspelledWord(QString) --> void"
    << "QQuickWebEngineView.runJavaScript(QString) --> void"
    << "QQuickWebEngineView.runJavaScript(QString,QJSValue) --> void"
    << "QQuickWebEngineView.runJavaScript(QString,uint) --> void"
    << "QQuickWebEngineView.runJavaScript(QString,uint,QJSValue) --> void"
    << "QQuickWebEngineView.scrollPosition --> QPointF"
    << "QQuickWebEngineView.scrollPositionChanged(QPointF) --> void"
    << "QQuickWebEngineView.selectClientCertificate(QQuickWebEngineClientCertificateSelection*) --> void"
    << "QQuickWebEngineView.setActiveFocusOnPress(bool) --> void"
    << "QQuickWebEngineView.settings --> QQuickWebEngineSettings*"
    << "QQuickWebEngineView.stop() --> void"
    << "QQuickWebEngineView.title --> QString"
    << "QQuickWebEngineView.titleChanged() --> void"
    << "QQuickWebEngineView.tooltipRequested(QQuickWebEngineTooltipRequest*) --> void"
    << "QQuickWebEngineView.touchHandleDelegate --> QQmlComponent*"
    << "QQuickWebEngineView.touchHandleDelegateChanged() --> void"
    << "QQuickWebEngineView.touchSelectionMenuRequested(QQuickWebEngineTouchSelectionMenuRequest*) --> void"
    << "QQuickWebEngineView.triggerWebAction(WebAction) --> void"
    << "QQuickWebEngineView.url --> QUrl"
    << "QQuickWebEngineView.urlChanged() --> void"
    << "QQuickWebEngineView.userScripts --> QQuickWebEngineScriptCollection*"
#if QT_CONFIG(webengine_webchannel)
    << "QQuickWebEngineView.webChannel --> QQmlWebChannel*"
#endif
    << "QQuickWebEngineView.webChannelChanged() --> void"
    << "QQuickWebEngineView.webChannelWorld --> uint"
    << "QQuickWebEngineView.webChannelWorldChanged(uint) --> void"
    << "QQuickWebEngineView.windowCloseRequested() --> void"
    << "QQuickWebEngineView.zoomFactor --> double"
    << "QQuickWebEngineView.zoomFactorChanged(double) --> void"
    << "QQuickWebEngineView.acceptAsNewWindow(QWebEngineNewWindowRequest*) --> void"
    << "QQuickWebEngineView.save(QString) --> void"
    << "QQuickWebEngineView.save(QString,QWebEngineDownloadRequest::SavePageFormat) --> void"
    << "QWebEngineQuotaRequest.accept() --> void"
    << "QWebEngineQuotaRequest.origin --> QUrl"
    << "QWebEngineQuotaRequest.reject() --> void"
    << "QWebEngineQuotaRequest.requestedSize --> qlonglong"
    << "QWebEngineRegisterProtocolHandlerRequest.accept() --> void"
    << "QWebEngineRegisterProtocolHandlerRequest.origin --> QUrl"
    << "QWebEngineRegisterProtocolHandlerRequest.reject() --> void"
    << "QWebEngineRegisterProtocolHandlerRequest.scheme --> QString"
    << "QWebEngineNotification.origin --> QUrl"
    << "QWebEngineNotification.title --> QString"
    << "QWebEngineNotification.message --> QString"
    << "QWebEngineNotification.tag --> QString"
    << "QWebEngineNotification.language --> QString"
    << "QWebEngineNotification.direction --> Qt::LayoutDirection"
    << "QWebEngineNotification.show() --> void"
    << "QWebEngineNotification.click() --> void"
    << "QWebEngineNotification.close() --> void"
    << "QWebEngineNotification.closed() --> void"
    ;

static bool isCheckedEnum(QMetaType t)
{
    if (t.flags() & QMetaType::IsEnumeration) {
        if (const QMetaObject *metaObject = t.metaObject()) {
            QRegularExpression re("^QFlags<(.*)>$");
            QRegularExpressionMatch match = re.match(t.name());
            const QByteArray enumName =
                    match.hasMatch() ? match.captured(1).toUtf8() : QByteArray(t.name());
            const char *lastColon = std::strrchr(enumName, ':');
            QMetaEnum type = metaObject->enumerator(metaObject->indexOfEnumerator(
                    lastColon ? lastColon + 1 : enumName.constData()));
            for (auto knownEnum : knownEnumNames) {
                if (type.name() == knownEnum.name() && type.scope() == knownEnum.scope())
                    return true;
            }
        }
    }
    return false;
}

static bool isCheckedClass(const QByteArray &typeName)
{
    for (const QMetaObject *mo : typesToCheck) {
        QByteArray moTypeName(mo->className());
        if (moTypeName == typeName || moTypeName + "*" == typeName)
            return true;
    }
    return false;
}

static void checkKnownType(const QMetaType &type)
{
    const QByteArray typeName = type.name();
    // calling id() registers the object
    if (!hardcodedTypes.contains(typeName) && type.id() >= QMetaType::User) {
        bool knownEnum = isCheckedEnum(type);
        bool knownClass = isCheckedClass(typeName);
        QVERIFY2(knownEnum || knownClass, qPrintable(QString("The API uses an unknown type [%1], you might have to add it to the typesToCheck list.").arg(typeName.constData())));
    }
}

static void gatherAPI(const QString &prefix, const QMetaEnum &metaEnum, QStringList *output)
{
    const auto format = metaEnum.isScoped() ? "%1%3.%2 --> %3" : "%1%2 --> %3";
    for (int i = 0; i < metaEnum.keyCount(); ++i)
        *output << QString::fromLatin1(format).arg(prefix).arg(metaEnum.key(i)).arg(metaEnum.name());
    knownEnumNames << metaEnum;
}

static void gatherAPI(const QString &prefix, const QMetaProperty &property, QStringList *output)
{
    *output << QString::fromLatin1("%1%2 --> %3").arg(prefix).arg(property.name()).arg(property.typeName());
    checkKnownType(property.metaType());
}

static void gatherAPI(const QString &prefix, const QMetaMethod &method, QStringList *output)
{
    if (method.access() != QMetaMethod::Private) {
        const char *methodTypeName = !!strlen(method.typeName()) ? method.typeName() : "void";
        *output << QString::fromLatin1("%1%2 --> %3").arg(prefix).arg(QString::fromLatin1(method.methodSignature())).arg(QString::fromLatin1(methodTypeName));

        checkKnownType(method.returnMetaType());

        const auto parameterCount = method.parameterCount();
        for (int i = 0; i < parameterCount; ++i) {
            const QMetaType metaType = method.parameterMetaType(i);
            checkKnownType(metaType);
        }
    }
}

static void gatherAPI(const QString &prefix, const QMetaObject *meta, QStringList *output)
{
    // *Offset points us only at the leaf class members, we don't have inheritance in our API yet anyway.
    for (int i = meta->enumeratorOffset(); i < meta->enumeratorCount(); ++i)
        gatherAPI(prefix, meta->enumerator(i), output);
    for (int i = meta->propertyOffset(); i < meta->propertyCount(); ++i)
        gatherAPI(prefix, meta->property(i), output);
    for (int i = meta->methodOffset(); i < meta->methodCount(); ++i)
        gatherAPI(prefix, meta->method(i), output);
}

void tst_publicapi::publicAPI()
{
    QStringList actualAPI;
    for (const QMetaObject *meta : typesToCheck)
        gatherAPI(QString::fromLatin1(meta->className()) + ".", meta, &actualAPI);

    // Uncomment to print the actual API.
    // QStringList sortedAPI(actualAPI);
    // std::sort(sortedAPI.begin(), sortedAPI.end());
    // for (const QString &actual : std::as_const(sortedAPI))
    //     printf("    << \"%s\"\n", qPrintable(actual));

    bool apiMatch = true;
    // Make sure that nothing slips in the public API unintentionally.
    for (const QString &actual : std::as_const(actualAPI)) {
        if (!expectedAPI.contains(actual)) {
            qWarning("Expected list is not up-to-date: %ls", qUtf16Printable(actual));
            apiMatch = false;
        }
    }
    // Make sure that the expected list is up-to-date with intentionally added APIs.
    for (const QString &expected : expectedAPI) {
        if (!actualAPI.contains(expected)) {
            apiMatch = false;
            qWarning("Not implemented: %ls", qUtf16Printable(expected));
        }
    }

    QVERIFY2(apiMatch, "Unexpected, missing or misspelled API!");
}

QTEST_MAIN(tst_publicapi)

#include "tst_publicapi.moc"

