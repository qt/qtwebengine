/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

QT_BEGIN_NAMESPACE

namespace ForeignWebEngineLoadingInfoNamespace
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE(QWebEngineLoadingInfo)
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

namespace ForeignWebEngineCertificateErrorNamespace
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE(QWebEngineCertificateError)
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

struct ForeignWebEngineQuotaRequest
{
    Q_GADGET
    QML_FOREIGN(QWebEngineQuotaRequest)
    QML_NAMED_ELEMENT(webEngineQuotaRequest)
    QML_ADDED_IN_VERSION(1, 7)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")
};

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

QT_END_NAMESPACE

#endif // QQUICKWEBENGINEFOREIGNTYPES_H
