// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINENEWWINDOWREQUEST_P_H
#define QWEBENGINENEWWINDOWREQUEST_P_H

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

#include "qwebenginenewwindowrequest.h"

#include <QtCore/QRect>
#include <QtCore/QSharedPointer>
#include <QtCore/QUrl>

namespace QtWebEngineCore {
class WebContentsAdapter;
}

QT_BEGIN_NAMESPACE

struct QWebEngineNewWindowRequestPrivate
{
    void setHandled()
    {
        isRequestHandled = true;
        adapter.reset();
    }

    QWebEngineNewWindowRequest::DestinationType destination;
    QRect requestedGeometry;
    QUrl requestedUrl;
    QSharedPointer<QtWebEngineCore::WebContentsAdapter> adapter;
    bool isUserInitiated;
    bool isRequestHandled = false;
};

QT_END_NAMESPACE

#endif // QWEBENGINENEWWINDOWREQUEST_P_H
