// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEURLREQUESTINFO_P_H
#define QWEBENGINEURLREQUESTINFO_P_H

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

#include "qtwebenginecoreglobal_p.h"

#include "qwebengineurlrequestinfo.h"

#include <QByteArray>
#include <QHash>
#include <QUrl>

namespace net {
class URLRequest;
}

QT_BEGIN_NAMESPACE

class QWebEngineUrlRequestInfoPrivate
{
    Q_DECLARE_PUBLIC(QWebEngineUrlRequestInfo)
public:
    QWebEngineUrlRequestInfoPrivate(QWebEngineUrlRequestInfo::ResourceType resource,
                                    QWebEngineUrlRequestInfo::NavigationType navigation,
                                    const QUrl &u, const QUrl &fpu, const QUrl &i,
                                    const QByteArray &m,
                                    const QHash<QByteArray, QByteArray> &h = {});

    QWebEngineUrlRequestInfo::ResourceType resourceType;
    QWebEngineUrlRequestInfo::NavigationType navigationType;
    bool shouldBlockRequest;
    bool shouldRedirectRequest;
    QUrl url;
    QUrl firstPartyUrl;
    QUrl initiator;
    const QByteArray method;
    bool changed;
    QHash<QByteArray, QByteArray> extraHeaders;

    QWebEngineUrlRequestInfo *q_ptr;
};

QT_END_NAMESPACE

#endif // QWEBENGINEURLREQUESTINFO_P_H
