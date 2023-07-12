// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTWEBENGINECOREGLOBAL_H
#define QTWEBENGINECOREGLOBAL_H

#include <QtCore/qglobal.h>
#include <QtWebEngineCore/qtwebenginecore-config.h>

QT_BEGIN_NAMESPACE

class QUrl;

#if defined(BUILDING_CHROMIUM)
#  define Q_WEBENGINECORE_EXPORT Q_DECL_EXPORT
#else
#  define Q_WEBENGINECORE_EXPORT Q_DECL_IMPORT
#endif

#define ASSERT_ENUMS_MATCH(A, B) Q_STATIC_ASSERT_X(static_cast<int>(A) == static_cast<int>(B), "The enum values must match");

Q_WEBENGINECORE_EXPORT Q_DECL_CONST_FUNCTION const char *qWebEngineVersion() noexcept;
Q_WEBENGINECORE_EXPORT Q_DECL_CONST_FUNCTION const char *qWebEngineProcessName() noexcept;
Q_WEBENGINECORE_EXPORT Q_DECL_CONST_FUNCTION const char *qWebEngineChromiumVersion() noexcept;
Q_WEBENGINECORE_EXPORT Q_DECL_CONST_FUNCTION const char *qWebEngineChromiumSecurityPatchVersion() noexcept;

Q_WEBENGINECORE_EXPORT QString qWebEngineGetDomainAndRegistry(const QUrl &url);

QT_END_NAMESPACE

#endif // QTWEBENGINECOREGLOBAL_H
