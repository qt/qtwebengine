// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQUICKWEBENGINEPROFILEPROTOTYPE_P_P_H
#define QQUICKWEBENGINEPROFILEPROTOTYPE_P_P_H

#include "qquickwebengineprofile_p.h"

#include <QScopedPointer>
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

QT_BEGIN_NAMESPACE

struct QQuickWebEngineProfilePrototypePrivate
{
    QString m_storageName;
    QString m_persistentStoragePath;
    QString m_cachePath;
    QStringList m_additionalTrustedCertificateFiles;
    QQuickWebEngineProfile::HttpCacheType m_httpCacheType = QQuickWebEngineProfile::DiskHttpCache;
    QQuickWebEngineProfile::PersistentCookiesPolicy m_persistentCookiesPolicy =
            QQuickWebEngineProfile::AllowPersistentCookies;
    int m_httpCacheMaxSize = 0;
    QQuickWebEngineProfile::PersistentPermissionsPolicy m_persistentPermissionsPolicy =
            QQuickWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk;
    bool m_isComponentComplete = false;

    QScopedPointer<QQuickWebEngineProfile> profile;
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINEPROFILEPROTOTYPE_P_P_H
