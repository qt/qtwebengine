// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

#ifdef QT_WEBENGINE_LOGGING
#define QT_NOT_YET_IMPLEMENTED fprintf(stderr, "function %s not implemented! - %s:%d\n", __func__, __FILE__, __LINE__);
#define QT_NOT_USED fprintf(stderr, "# function %s should not be used! - %s:%d\n", __func__, __FILE__, __LINE__); Q_UNREACHABLE();
#else
#define QT_NOT_YET_IMPLEMENTED qt_noop();
#define QT_NOT_USED Q_UNREACHABLE(); // This will assert in debug.
#endif

#define Q_WEBENGINECORE_PRIVATE_EXPORT Q_WEBENGINECORE_EXPORT

namespace QtWebEngineCore {
Q_WEBENGINECORE_PRIVATE_EXPORT int processMain(int argc, const char **argv);
Q_WEBENGINECORE_PRIVATE_EXPORT bool closingDown();
} // namespace
#if defined(Q_OS_WIN)
namespace sandbox {
struct SandboxInterfaceInfo;
}
namespace QtWebEngineSandbox {
Q_WEBENGINECORE_PRIVATE_EXPORT sandbox::SandboxInterfaceInfo *staticSandboxInterfaceInfo(sandbox::SandboxInterfaceInfo *info = nullptr);
void initializeStaticCopy(int argc, const char **argv);
}
#endif
#endif // QTWEBENGINECOREGLOBAL_P_H
