// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEPERMISSION_P_H
#define QWEBENGINEPERMISSION_P_H

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

#include "qwebenginepermission.h"

#include <QtCore/qpointer.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qshareddata.h>

namespace QtWebEngineCore {
class WebContentsAdapter;
class ProfileAdapter;
}

QT_BEGIN_NAMESPACE

struct QWebEnginePermissionPrivate : public QSharedData
{
    Q_WEBENGINECORE_EXPORT QWebEnginePermissionPrivate();
    Q_WEBENGINECORE_EXPORT QWebEnginePermissionPrivate(const QUrl &, QWebEnginePermission::PermissionType,
        QSharedPointer<QtWebEngineCore::WebContentsAdapter>, QtWebEngineCore::ProfileAdapter *);

    QUrl origin;
    QWebEnginePermission::PermissionType permissionType;

    QWeakPointer<QtWebEngineCore::WebContentsAdapter> webContentsAdapter;
    QPointer<QtWebEngineCore::ProfileAdapter> profileAdapter;
};

QT_END_NAMESPACE

#endif // QWEBENGINEPERMISSION_P_H
