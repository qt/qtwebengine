// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

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
#include <QtWebEngineCore/qwebenginecertificateerror.h>
#include <QtWebEngineCore/qwebengineclienthints.h>
#include <QtWebEngineCore/qwebenginecontextmenurequest.h>
#include <QtWebEngineCore/qwebenginedesktopmediarequest.h>
#include <QtWebEngineCore/qwebenginedownloadrequest.h>
#include <QtWebEngineCore/qwebenginefilesystemaccessrequest.h>
#include <QtWebEngineCore/qwebenginefindtextresult.h>
#include <QtWebEngineCore/qwebengineframe.h>
#include <QtWebEngineCore/qwebenginefullscreenrequest.h>
#include <QtWebEngineCore/qwebenginehistory.h>
#include <QtWebEngineCore/qwebengineloadinginfo.h>
#include <QtWebEngineCore/qwebenginenavigationrequest.h>
#include <QtWebEngineCore/qwebenginenewwindowrequest.h>
#include <QtWebEngineCore/qwebenginenotification.h>
#include <QtWebEngineCore/qwebenginequotarequest.h>
#include <QtWebEngineCore/qwebenginepermission.h>
#include <QtWebEngineCore/qwebengineregisterprotocolhandlerrequest.h>
#include <QtWebEngineCore/qwebenginescript.h>
#include <QtWebEngineCore/qwebenginewebauthuxrequest.h>

#if QT_CONFIG(webengine_extensions)
#include <QtWebEngineCore/qwebengineextensioninfo.h>
#include <QtWebEngineCore/qwebengineextensionmanager.h>
#endif

QT_BEGIN_NAMESPACE

// To prevent the same type from being exported twice into qmltypes
// (for value type and for the enums)
#define CREATE_DERIVED_FOREIGN_NAMESPACE(TYPE)                                                     \
    struct TYPE##Derived : public TYPE                                                             \
    {                                                                                              \
        Q_GADGET                                                                                   \
    };

// QML object types

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

struct ForeignWebEngineNavigationRequest
{
    Q_GADGET
    QML_FOREIGN(QWebEngineNavigationRequest)
    QML_NAMED_ELEMENT(WebEngineNavigationRequest)
    QML_ADDED_IN_VERSION(1, 0)
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

struct ForeignWebEngineNotification
{
    Q_GADGET
    QML_FOREIGN(QWebEngineNotification)
    QML_NAMED_ELEMENT(WebEngineNotification)
    QML_ADDED_IN_VERSION(1, 9)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")
};

struct ForeignWebEngineWebAuthUxRequest
{
    Q_GADGET
    QML_FOREIGN(QWebEngineWebAuthUxRequest)
    QML_NAMED_ELEMENT(WebEngineWebAuthUxRequest)
    QML_ADDED_IN_VERSION(6, 7)
    QML_UNCREATABLE("")
};

struct ForeignWebEngineClientHints : public QObject
{
    Q_OBJECT
    QML_FOREIGN(QWebEngineClientHints)
    QML_NAMED_ELEMENT(WebEngineClientHints)
    QML_ADDED_IN_VERSION(6, 8)
    QML_UNCREATABLE("")
};

#if QT_CONFIG(webengine_extensions)
struct ForeignWebEngineExtensionManager
{
    Q_GADGET
    QML_NAMED_ELEMENT(WebEngineExtensionManager)
    QML_FOREIGN(QWebEngineExtensionManager)
    QML_ADDED_IN_VERSION(6, 10)
    QML_UNCREATABLE("")
};
#endif

// QML value types

struct ForeignWebEngineDesktopMediaRequest
{
    Q_GADGET
    QML_FOREIGN(QWebEngineDesktopMediaRequest)
    QML_VALUE_TYPE(webEngineDesktopMediaRequest)
    QML_ADDED_IN_VERSION(6, 10)
};

CREATE_DERIVED_FOREIGN_NAMESPACE(QWebEngineLoadingInfo)

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
    QML_VALUE_TYPE(webEngineLoadingInfo)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
};

CREATE_DERIVED_FOREIGN_NAMESPACE(QWebEngineCertificateError)

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
    QML_VALUE_TYPE(webEngineCertificateError)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
};

CREATE_DERIVED_FOREIGN_NAMESPACE(QWebEngineScript)

namespace ForeignWebEngineScriptNamespace
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE(QWebEngineScriptDerived)
    QML_NAMED_ELEMENT(WebEngineScript)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
}

struct ForeignWebEngineScript
{
    Q_GADGET
    QML_FOREIGN(QWebEngineScript)
    QML_STRUCTURED_VALUE
    QML_VALUE_TYPE(webEngineScript)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
};

struct ForeignWebEngineFullScreenRequest
{
    Q_GADGET
    QML_FOREIGN(QWebEngineFullScreenRequest)
    QML_VALUE_TYPE(fullScreenRequest)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
};

#if QT_DEPRECATED_SINCE(6, 5)
struct ForeignWebEngineQuotaRequest
{
    Q_GADGET
    QML_FOREIGN(QWebEngineQuotaRequest)
    QML_VALUE_TYPE(webEngineQuotaRequest)
    QML_ADDED_IN_VERSION(1, 7)
    QML_EXTRA_VERSION(2, 0)
};
#endif

struct ForeignWebEngineRegisterProtocolHandlerRequest
{
    Q_GADGET
    QML_FOREIGN(QWebEngineRegisterProtocolHandlerRequest)
    QML_VALUE_TYPE(registerProtocolHandlerRequest)
    QML_ADDED_IN_VERSION(1, 7)
    QML_EXTRA_VERSION(2, 0)
};

struct ForeignWebEngineFindTextResult
{
    Q_GADGET
    QML_FOREIGN(QWebEngineFindTextResult)
    QML_VALUE_TYPE(findTextResult)
    QML_ADDED_IN_VERSION(1, 10)
    QML_EXTRA_VERSION(2, 0)
};

struct ForeginWebEngineFileSystemAccessRequest
{
    Q_GADGET
    QML_FOREIGN(QWebEngineFileSystemAccessRequest)
    QML_VALUE_TYPE(webEngineFileSystemAccessRequest)
    QML_ADDED_IN_VERSION(6, 4)
};

CREATE_DERIVED_FOREIGN_NAMESPACE(QWebEngineFileSystemAccessRequest)

namespace ForeginWebEngineFileSystemAccessRequestNamespace
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE(QWebEngineFileSystemAccessRequestDerived)
    QML_NAMED_ELEMENT(WebEngineFileSystemAccessRequest)
    QML_ADDED_IN_VERSION(6, 4)
};

struct ForeginWebEngineWebAuthPinRequest
{
    Q_GADGET
    QML_FOREIGN(QWebEngineWebAuthPinRequest)
    QML_VALUE_TYPE(webEngineWebAuthPinRequest)
    QML_ADDED_IN_VERSION(6, 8)
};

CREATE_DERIVED_FOREIGN_NAMESPACE(QWebEnginePermission)

namespace ForeignWebEnginePermissionNamespace
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE(QWebEnginePermissionDerived)
    QML_NAMED_ELEMENT(WebEnginePermission)
    QML_ADDED_IN_VERSION(6, 8)
}

struct ForeignWebEnginePermission
{
    Q_GADGET
    QML_FOREIGN(QWebEnginePermission)
    QML_VALUE_TYPE(webEnginePermission)
    QML_ADDED_IN_VERSION(6, 8)
};

#if QT_CONFIG(webengine_extensions)
struct ForeignWebEngineExtensionInfo
{
    Q_GADGET
    QML_VALUE_TYPE(webEngineExtension)
    QML_FOREIGN(QWebEngineExtensionInfo)
    QML_ADDED_IN_VERSION(6, 10)
};
#endif

#undef CREATE_DERIVED_FOREIGN_NAMESPACE

QT_END_NAMESPACE

#endif // QQUICKWEBENGINEFOREIGNTYPES_H
