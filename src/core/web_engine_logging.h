// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

#ifndef WEB_ENGINE_LOGGING_H
#define WEB_ENGINE_LOGGING_H

#include <QLoggingCategory>

// The Q_WEBENGINE_LOGGING_CATEGORY macro is a stopgap fix for the deprecation warnings
// introduced alongside the new logging category macros in Qt 6.9. Since we need to support
// building with older versions up to and including the last LTS release (6.8), we can't simply
// switch to using Q_STATIC_LOGGING_CATEGORY. This file and the macro definition within
// are intended to be removed in the next Qt 6 LTS release.
#if defined(Q_STATIC_LOGGING_CATEGORY)
#define Q_WEBENGINE_LOGGING_CATEGORY(name, ...) Q_STATIC_LOGGING_CATEGORY(name, __VA_ARGS__)
#else
#define Q_WEBENGINE_LOGGING_CATEGORY(name, ...) Q_LOGGING_CATEGORY(name, __VA_ARGS__)
#endif

#endif // WEB_ENGINE_LOGGING_H
