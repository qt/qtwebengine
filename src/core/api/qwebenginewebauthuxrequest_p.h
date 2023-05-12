// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEWEBAUTHUXREQUEST_P_H
#define QWEBENGINEWEBAUTHUXREQUEST_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qtwebenginecoreglobal_p.h"
#include "qwebenginewebauthuxrequest.h"
#include <QtCore/QSharedPointer>

namespace QtWebEngineCore {
class WebContentsAdapterClient;
class AuthenticatorRequestDialogController;
}

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_PRIVATE_EXPORT QWebEngineWebAuthUXRequestPrivate
{

public:
    QWebEngineWebAuthUXRequestPrivate(
            QtWebEngineCore::AuthenticatorRequestDialogController *controller);
    ~QWebEngineWebAuthUXRequestPrivate();

    QWebEngineWebAuthUXRequest::WebAuthUXState m_currentState =
            QWebEngineWebAuthUXRequest::NotStarted;
    QtWebEngineCore::AuthenticatorRequestDialogController *webAuthDialogController;
};

QT_END_NAMESPACE
#endif // QWEBENGINEWEBAUTHUXREQUEST_P_H
