// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <qlibrary.h>
#include <qoperatingsystemversion.h>
#include <qsysinfo.h>
#include <qt_windows.h>
#include <TlHelp32.h>

class User32DLL {
public:
    User32DLL()
        : setProcessDPIAware(0)
    {
        library.setFileName(QStringLiteral("User32"));
        if (!library.load())
            return;
        setProcessDPIAware = (SetProcessDPIAware)library.resolve("SetProcessDPIAware");
    }

    bool isValid() const
    {
        return setProcessDPIAware;
    }

    typedef BOOL (WINAPI *SetProcessDPIAware)();

    // Windows Vista onwards
    SetProcessDPIAware setProcessDPIAware;

private:
    QLibrary library;
};

// This must match PROCESS_DPI_AWARENESS in ShellScalingApi.h
enum DpiAwareness {
    PROCESS_PER_UNAWARE = 0,
    PROCESS_PER_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
};

// Shell scaling library (Windows 8.1 onwards)
class ShcoreDLL {
public:
    ShcoreDLL()
        : getProcessDpiAwareness(0), setProcessDpiAwareness(0)
    {
        if (QOperatingSystemVersion::current() < QOperatingSystemVersion::Windows8_1)
            return;
        library.setFileName(QStringLiteral("SHCore"));
        if (!library.load())
            return;
        getProcessDpiAwareness = (GetProcessDpiAwareness)library.resolve("GetProcessDpiAwareness");
        setProcessDpiAwareness = (SetProcessDpiAwareness)library.resolve("SetProcessDpiAwareness");
    }

    bool isValid() const
    {
        return getProcessDpiAwareness && setProcessDpiAwareness;
    }

    typedef HRESULT (WINAPI *GetProcessDpiAwareness)(HANDLE, DpiAwareness *);
    typedef HRESULT (WINAPI *SetProcessDpiAwareness)(DpiAwareness);

    GetProcessDpiAwareness getProcessDpiAwareness;
    SetProcessDpiAwareness setProcessDpiAwareness;

private:
    QLibrary library;
};


static DWORD getParentProcessId()
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        qErrnoWarning(GetLastError(), "CreateToolhelp32Snapshot failed.");
        return NULL;
    }

    PROCESSENTRY32 pe = {0};
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe)) {
        qWarning("Cannot retrieve parent process handle.");
        return NULL;
    }

    DWORD parentPid = NULL;
    const DWORD pid = GetCurrentProcessId();
    do {
        if (pe.th32ProcessID == pid) {
            parentPid = pe.th32ParentProcessID;
            break;
        }
    } while (Process32Next(hSnapshot, &pe));
    CloseHandle(hSnapshot);
    return parentPid;
}

namespace QtWebEngineProcess {

void initDpiAwareness()
{
    ShcoreDLL shcore;
    if (shcore.isValid()) {
        DpiAwareness dpiAwareness = PROCESS_PER_MONITOR_DPI_AWARE;
        const DWORD pid = getParentProcessId();
        if (pid) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
            DpiAwareness parentDpiAwareness;
            HRESULT hr = shcore.getProcessDpiAwareness(hProcess, &parentDpiAwareness);
            CloseHandle(hProcess);
            if (hr == S_OK)
                dpiAwareness = parentDpiAwareness;
        }
        if (shcore.setProcessDpiAwareness(dpiAwareness) != S_OK)
            qErrnoWarning(GetLastError(), "SetProcessDPIAwareness failed.");
    } else {
        // Fallback. Use SetProcessDPIAware unconditionally.
        User32DLL user32;
        if (user32.isValid())
            user32.setProcessDPIAware();
    }
}

} // namespace QtWebEngineProcess
