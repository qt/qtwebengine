// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QCoreApplication>
#include <stdio.h>
#include <memory>

#if defined(Q_OS_LINUX)

struct tm;
struct stat;
struct stat64;

// exported in sandbox/linux/services/libc_interceptor.cc
namespace sandbox {
struct tm* localtime_override(const time_t* timep);
struct tm* localtime64_override(const time_t* timep);
struct tm* localtime_r_override(const time_t* timep, struct tm* result);
struct tm* localtime64_r_override(const time_t* timep, struct tm* result);
}

// from sandbox/linux/services/libc_interceptor.cc
__attribute__ ((__visibility__("default")))
struct tm* localtime_proxy(const time_t* timep) __asm__ ("localtime");
struct tm* localtime_proxy(const time_t* timep)
{
    return sandbox::localtime_override(timep);
}

__attribute__ ((__visibility__("default")))
struct tm* localtime64_proxy(const time_t* timep) __asm__ ("localtime64");
struct tm* localtime64_proxy(const time_t* timep)
{
    return sandbox::localtime64_override(timep);
}

__attribute__ ((__visibility__("default")))
struct tm* localtime_r_proxy(const time_t* timep, struct tm* result) __asm__ ("localtime_r");
struct tm* localtime_r_proxy(const time_t* timep, struct tm* result)
{
    return sandbox::localtime_r_override(timep, result);
}

__attribute__ ((__visibility__("default")))
struct tm* localtime64_r_proxy(const time_t* timep, struct tm* result) __asm__ ("localtime64_r");
struct tm* localtime64_r_proxy(const time_t* timep, struct tm* result)
{
    return sandbox::localtime64_r_override(timep, result);
}

#endif // defined(Q_OS_LINUX)

#if defined(Q_OS_WIN32)
namespace QtWebEngineProcess {
void initDpiAwareness();
}
#endif // defined(Q_OS_WIN32)

int main(int argc, const char **argv)
{
#if defined(Q_OS_WIN32)
    QtWebEngineSandbox::initializeStaticCopy(argc, argv);
    QtWebEngineProcess::initDpiAwareness();
#endif

    // Chromium on Linux manipulates argv to set a process title
    // (see set_process_title_linux.cc).
    // This can interfere with QCoreApplication::applicationFilePath,
    // which assumes that argv[0] only contains the executable path.
    //
    // Avoid this by making a deep copy of argv and pass this
    // to QCoreApplication. Use a unique_ptr with custom deleter to
    // clean up on exit.

    auto dt = [](char* av[]) {
        for (char **a = av; *a; a++)
          delete[] *a;
        delete[] av;
    };

    std::unique_ptr<char*[], decltype(dt)> argv_(new char*[argc+1], dt);
    for (int i = 0; i < argc; ++i) {
        size_t len = strlen(argv[i]) + 1;
        argv_[i] = new char[len];
        strcpy(argv_[i], argv[i]);
    }
    argv_[argc] = 0;

    QCoreApplication qtApplication(argc, argv_.get());

    if (argc == 1) {
        qInfo("%s(%s/%s)", qPrintable(qtApplication.applicationName()), qWebEngineVersion(),
              qWebEngineChromiumVersion());
        return 0;
    } else {
        return QtWebEngineCore::processMain(argc, argv);
    }
}

