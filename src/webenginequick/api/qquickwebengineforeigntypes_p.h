// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#ifndef QQUICKWEBENGINEFOREIGNTYPES_H
#define QQUICKWEBENGINEFOREIGNTYPES_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQml/qqml.h>
#include <QtWebEngineCore/qwebenginedownloadrequest.h>
#include <QtWebEngineCore/qwebenginenavigationrequest.h>
#include <QtWebEngineCore/qwebenginescript.h>
#include <QtWebEngineCore/qwebenginenewwindowrequest.h>
#include <QtWebEngineCore/qwebengineloadinginfo.h>
#include <QtWebEngineCore/qwebenginehistory.h>
#include <QtWebEngineCore/qwebenginequotarequest.h>
#include <QtWebEngineCore/qwebenginenotification.h>
#include <QtWebEngineCore/qwebenginefindtextresult.h>
#include <QtWebEngineCore/qwebenginecertificateerror.h>
#include <QtWebEngineCore/qwebenginefullscreenrequest.h>
#include <QtWebEngineCore/qwebenginecontextmenurequest.h>
#include <QtWebEngineCore/qwebengineregisterprotocolhandlerrequest.h>
#include <QtWebEngineCore/qwebenginefilesystemaccessrequest.h>

QT_BEGIN_NAMESPACE

// To prevent the same type from being exported twice into qmltypes
// (for value type and for the enums)
struct QWebEngineLoadingInfoDerived : public QWebEngineLoadingInfo
{
    Q_GADGET
};

namespace ForeignWebEngineLoadingInfoNamespace
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE(QWebEngineLoadingInfoDerived)
    QML_NAMED_ELEMENT(WebEngineLoadingInfo)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
}

struct ForeignWebEngineLoadingInfo
{
    Q_GADGET
    QML_FOREIGN(QWebEngineLoadingInfo)
    QML_NAMED_ELEMENT(webEngineLoadingInfo)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")
};

// To prevent the same type from being exported twice into qmltypes
// (for value type and for the enums)
struct QWebEngineCertificateErrorDerived : public QWebEngineCertificateError
{
    Q_GADGET
};

namespace ForeignWebEngineCertificateErrorNamespace
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE(QWebEngineCertificateErrorDerived)
    QML_NAMED_ELEMENT(WebEngineCertificateError)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
}

struct ForeignWebEngineCertificateError
{
    Q_GADGET
    QML_FOREIGN(QWebEngineCertificateError)
    QML_NAMED_ELEMENT(webEngineCertificateError)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")
};

struct ForeignWebEngineNavigationRequest
{
    Q_GADGET
    QML_FOREIGN(QWebEngineNavigationRequest)
    QML_NAMED_ELEMENT(WebEngineNavigationRequest)
    QML_ADDED_IN_VERSION(1, 0)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")
};

namespace ForeignWebEngineScriptNamespace
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE(QWebEngineScript)
    QML_NAMED_ELEMENT(WebEngineScript)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
}

struct ForeignWebEngineScript
{
    Q_GADGET
    QML_FOREIGN(QWebEngineScript)
    QML_NAMED_ELEMENT(webEngineScript)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")
};

struct ForeignWebEngineHistory
{
    Q_GADGET
    QML_FOREIGN(QWebEngineHistory)
    QML_NAMED_ELEMENT(WebEngineHistory)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")
};

struct ForeignWebEngineHistoryModel
{
    Q_GADGET
    QML_FOREIGN(QWebEngineHistoryModel)
    QML_NAMED_ELEMENT(WebEngineHistoryModel)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")
};

struct ForeignWebEngineFullScreenRequest
{
    Q_GADGET
    QML_FOREIGN(QWebEngineFullScreenRequest)
    QML_NAMED_ELEMENT(fullScreenRequest)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")
};

struct ForeignWebEngineContextMenuRequest
{
    Q_GADGET
    QML_FOREIGN(QWebEngineContextMenuRequest)
    QML_NAMED_ELEMENT(ContextMenuRequest)
    QML_ADDED_IN_VERSION(1, 7)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")
};

#if QT_DEPRECATED_SINCE(6, 5)
struct ForeignWebEngineQuotaRequest
{
    Q_GADGET
    QML_FOREIGN(QWebEngineQuotaRequest)
    QML_NAMED_ELEMENT(webEngineQuotaRequest)
    QML_ADDED_IN_VERSION(1, 7)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")
};
#endif

struct ForeignWebEngineRegisterProtocolHandlerRequest
{
    Q_GADGET
    QML_FOREIGN(QWebEngineRegisterProtocolHandlerRequest)
    QML_NAMED_ELEMENT(registerProtocolHandlerRequest)
    QML_ADDED_IN_VERSION(1, 7)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")
};

struct ForeignWebEngineNotification
{
    Q_GADGET
    QML_FOREIGN(QWebEngineNotification)
    QML_NAMED_ELEMENT(WebEngineNotification)
    QML_ADDED_IN_VERSION(1, 9)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")
};

struct ForeignWebEngineFindTextResult
{
    Q_GADGET
    QML_FOREIGN(QWebEngineFindTextResult)
    QML_NAMED_ELEMENT(findTextResult)
    QML_ADDED_IN_VERSION(1, 10)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")
};

struct ForeginWebEngineFileSystemAccessRequest
{
    Q_GADGET
    QML_FOREIGN(QWebEngineFileSystemAccessRequest)
    QML_NAMED_ELEMENT(webEngineFileSystemAccessRequest)
    QML_ADDED_IN_VERSION(6, 4)
    QML_UNCREATABLE("")
};

// To prevent the same type from being exported twice into qmltypes
// (for value type and for the enums)
struct QWebEngineFileSystemAccessRequestDerived : public QWebEngineFileSystemAccessRequest
{
    Q_GADGET
};

namespace ForeginWebEngineFileSystemAccessRequestNamespace
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE(QWebEngineFileSystemAccessRequestDerived)
    QML_NAMED_ELEMENT(WebEngineFileSystemAccessRequest)
    QML_ADDED_IN_VERSION(6, 4)
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINEFOREIGNTYPES_H
