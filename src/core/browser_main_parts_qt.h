// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BROWSER_MAIN_PARTS_QT_H
#define BROWSER_MAIN_PARTS_QT_H

#include "content/public/browser/browser_main_parts.h"

#include "web_usb_detector_qt.h"

namespace base {
class MessagePump;
}

namespace content {
class ServiceManagerConnection;
}

namespace device {
class GeolocationManager;
}

namespace performance_manager {
class PerformanceManagerLifetime;
}

namespace QtWebEngineCore {

std::unique_ptr<base::MessagePump> messagePumpFactory();

class BrowserMainPartsQt : public content::BrowserMainParts
{
public:
    BrowserMainPartsQt() = default;
    ~BrowserMainPartsQt() override = default;

    int PreEarlyInitialization() override;
    void PreCreateMainMessageLoop() override;
    void PostCreateMainMessageLoop() override;
    int PreMainMessageLoopRun() override;
    void PostMainMessageLoopRun() override;
    int PreCreateThreads() override;
    void PostCreateThreads() override;

#if BUILDFLAG(IS_MAC)
    device::GeolocationManager *GetGeolocationManager();
#endif

private:
    std::unique_ptr<performance_manager::PerformanceManagerLifetime> performance_manager_lifetime_;
    std::unique_ptr<WebUsbDetectorQt> m_webUsbDetector;
#if BUILDFLAG(IS_MAC)
    std::unique_ptr<device::GeolocationManager> m_geolocationManager;
#endif
};

} // namespace QtWebEngineCore

#endif // BROWSER_MAIN_PARTS_QT_H
