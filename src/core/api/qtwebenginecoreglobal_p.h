// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTWEBENGINECOREGLOBAL_P_H
#define QTWEBENGINECOREGLOBAL_P_H

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
#include <QtCore/private/qglobal_p.h>
#include <QtWebEngineCore/private/qtwebenginecore-config_p.h>
#include <QLoggingCategory>

#ifdef QT_WEBENGINE_LOGGING
#define QT_NOT_YET_IMPLEMENTED fprintf(stderr, "function %s not implemented! - %s:%d\n", __func__, __FILE__, __LINE__);
#define QT_NOT_USED fprintf(stderr, "# function %s should not be used! - %s:%d\n", __func__, __FILE__, __LINE__); Q_UNREACHABLE();
#else
#define QT_NOT_YET_IMPLEMENTED qt_noop();
#define QT_NOT_USED Q_UNREACHABLE(); // This will assert in debug.
#endif

// The Q_WEBENGINE_LOGGING_CATEGORY macro is a stopgap fix for the deprecation warnings
// introduced alongside the new logging category macros in Qt 6.9. Since we need to support
// building with older versions up to and including the last LTS release (6.8), we can't simply
// switch to using Q_STATIC_LOGGING_CATEGORY. This file and the macro definition within
// are intended to be removed in the next Qt 6 LTS release.
#if QT_VERSION < QT_VERSION_CHECK(6, 11, 0)
#if defined(Q_STATIC_LOGGING_CATEGORY)
#define Q_WEBENGINE_LOGGING_CATEGORY(name, ...) Q_STATIC_LOGGING_CATEGORY(name, __VA_ARGS__)
#else
#define Q_WEBENGINE_LOGGING_CATEGORY(name, ...) Q_LOGGING_CATEGORY(name, __VA_ARGS__)
#endif
#endif

namespace QtWebEngineCore {
Q_WEBENGINECORE_EXPORT int processMain(int argc, const char **argv);
Q_WEBENGINECORE_EXPORT bool closingDown();
} // namespace
#if defined(Q_OS_WIN)
namespace sandbox {
struct SandboxInterfaceInfo;
}
namespace QtWebEngineSandbox {
Q_WEBENGINECORE_EXPORT sandbox::SandboxInterfaceInfo *staticSandboxInterfaceInfo(sandbox::SandboxInterfaceInfo *info = nullptr);
void initializeStaticCopy(int argc, const char **argv);
}
#endif
#endif // QTWEBENGINECOREGLOBAL_P_H
