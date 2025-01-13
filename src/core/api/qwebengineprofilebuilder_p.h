// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEPROFILEBUILDER_P_H
#define QWEBENGINEPROFILEBUILDER_P_H

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

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtCore/QString>
#include "qwebengineprofile.h"

QT_BEGIN_NAMESPACE
struct QWebEngineProfileBuilderPrivate
{
    QString m_dataPath;
    QString m_cachePath;
    QWebEngineProfile::HttpCacheType m_httpCacheType = QWebEngineProfile::DiskHttpCache;
    QWebEngineProfile::PersistentCookiesPolicy m_persistentCookiesPolicy =
            QWebEngineProfile::AllowPersistentCookies;
    int m_httpCacheMaxSize;
    QWebEngineProfile::PersistentPermissionsPolicy m_persistentPermissionPolicy =
            QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk;
};
QT_END_NAMESPACE
#endif // QWEBENGINEPROFILEBUILDER_P_H
