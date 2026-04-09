// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "content_gpu_client_qt.h"

#include "compositor/compositor.h"
#include "rhi_gpu_info.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/gpu_data_manager.h"
#include "content/public/browser/gpu_data_manager_observer.h"
#include "gpu/config/gpu_driver_bug_workarounds.h"
#include "gpu/config/gpu_info.h"
#include "mojo/public/cpp/bindings/binder_map.h"

#include <QtCore/qregularexpression.h>
#include <QtCore/qstring.h>

#include <optional>
#include <tuple>

#if BUILDFLAG(IS_WIN)
#include "ui/gl/gl_utils.h"
#endif

using namespace Qt::StringLiterals;

namespace QtWebEngineCore {

namespace {
static inline bool isSameDevice(const gpu::GPUInfo::GPUDevice &d1,
                                const gpu::GPUInfo::GPUDevice &d2)
{
    return std::tie(d1.vendor_id, d1.device_id, d1.system_device_id, d1.vendor_string,
                    d1.device_string, d1.driver_vendor, d1.driver_version)
            == std::tie(d2.vendor_id, d2.device_id, d2.system_device_id, d2.vendor_string,
                        d2.device_string, d2.driver_vendor, d2.driver_version);
}

static QString gpuDeviceToString(const gpu::GPUInfo::GPUDevice &device)
{
    if (device.vendor_id == 0x0)
        return "Disabled"_L1;

    QString deviceString;
    deviceString += RhiGpuInfo::vendorIdToString(device.vendor_id);
    deviceString += ", device id: 0x"_L1 + QString::number(device.device_id, 16);

    if (!device.driver_vendor.empty()) {
        deviceString += ", driver: "_L1 + QLatin1StringView(device.driver_vendor) + u' '
                + QLatin1StringView(device.driver_version);
    }
    deviceString += ", system device id: 0x"_L1 + QString::number(device.system_device_id, 16);

    deviceString += ", preference: "_L1;
    switch (device.gpu_preference) {
    case gl::GpuPreference::kNone:
        deviceString += "None"_L1;
        break;
    case gl::GpuPreference::kDefault:
        deviceString += "Default"_L1;
        break;
    case gl::GpuPreference::kLowPower:
        deviceString += "LowPower"_L1;
        break;
    case gl::GpuPreference::kHighPerformance:
        deviceString += "HighPerformance"_L1;
        break;
    }

    deviceString += ", active: "_L1 + (device.active ? "yes"_L1 : "no"_L1);
    return deviceString;
}

static inline const char *gpuFeatureStatusToString(const gpu::GpuFeatureStatus &status)
{
    switch (status) {
    case gpu::kGpuFeatureStatusEnabled:
        return "Enabled";
    case gpu::kGpuFeatureStatusBlocklisted:
        return "Blocklisted";
    case gpu::kGpuFeatureStatusDisabled:
        return "Disabled";
    case gpu::kGpuFeatureStatusSoftware:
        return "Software";
    case gpu::kGpuFeatureStatusUndefined:
        return "Undefined";
    case gpu::kGpuFeatureStatusMax:
        return "Max";
    }
}

static QString angleInfo(const gpu::GPUInfo &gpuInfo)
{
    QString info;

    if (gpuInfo.gl_vendor.empty() || gpuInfo.gl_vendor == "Disabled") {
        info = "ANGLE is disabled:\n"_L1;
        info += "  GL Renderer: "_L1 + QLatin1StringView(gpuInfo.gl_renderer) + u'\n';
        info += "  Software Renderer: "_L1 + (gpuInfo.gpu.IsSoftwareRenderer() ? "yes"_L1 : "no"_L1)
                + u'\n';
        info += "  Primary GPU: "_L1 + gpuDeviceToString(gpuInfo.gpu) + u'\n';
        return info;
    }

    info = QLatin1StringView(gpuInfo.display_type) + " display is initialized:\n"_L1;
    info += "  GL Renderer: "_L1 + QLatin1StringView(gpuInfo.gl_renderer) + u'\n';
    info += "  "_L1 + QString::number(gpuInfo.GpuCount()) + " GPU(s) detected:\n"_L1;
    info += "    "_L1 + gpuDeviceToString(gpuInfo.gpu) + u'\n';
    for (auto &secondary : gpuInfo.secondary_gpus)
        info += "    "_L1 + gpuDeviceToString(secondary) + u'\n';

    info += "  NVIDIA Optimus: "_L1 + (gpuInfo.optimus ? "enabled"_L1 : "disabled"_L1) + u'\n';
    info += "  AMD Switchable: "_L1 + (gpuInfo.amd_switchable ? "enabled"_L1 : "disabled"_L1);

    return info;
}

#if BUILDFLAG(IS_WIN)
static QString windowsInfo(const gpu::GPUInfo &gpuInfo)
{
    QString info;
    info = "Windows specific driver information:\n"_L1;

    info += "  Direct Composition: "_L1;
    if (gpuInfo.overlay_info.direct_composition)
        info += "enabled\n"_L1;
    else
        info += "disabled\n"_L1;

    info += "  Supports Overlays: "_L1
            + (gpuInfo.overlay_info.supports_overlays ? "yes"_L1 : "no"_L1) + u'\n';
    info += "  Supports D3D Shared Images: "_L1 + (gpuInfo.shared_image_d3d ? "yes"_L1 : "no"_L1);
    return info;
}
#endif
} // namespace

class GpuObserver : public content::GpuDataManagerObserver
{
public:
    GpuObserver(ContentGpuClientQt *client)
#if BUILDFLAG(IS_OZONE)
        : m_client(client)
#endif
    {
        content::GpuDataManager *manager = content::GpuDataManager::GetInstance();
        if (manager->IsEssentialGpuInfoAvailable())
            OnGpuInfoUpdate();
    }

    ~GpuObserver() { content::GpuDataManager::GetInstance()->RemoveObserver(this); }

    void OnGpuInfoUpdate() override
    {
        content::GpuDataManager *manager = content::GpuDataManager::GetInstance();
        Q_ASSERT(manager->IsEssentialGpuInfoAvailable());

        const gpu::GPUInfo &gpuInfo = manager->GetGPUInfo();
        if (!gpuInfo.IsInitialized()) {
            qWarning("GPUInfo not initialized on GpuInfoUpdate");
            return;
        }

        // Avoid logging the info again if the device hasn't changed.
        // A change in the device is unexpected, as we currently don't support or implement
        // fallback. Logging the info multiple times may indicate a problem.
        if (m_gpuInfo && isSameDevice(m_gpuInfo->gpu, gpuInfo.gpu))
            return;
        m_gpuInfo = gpuInfo;

        const gpu::GpuFeatureStatus gpuCompositingStatus =
                manager->GetFeatureStatus(gpu::GPU_FEATURE_TYPE_ACCELERATED_GL);
        qCDebug(lcWebEngineCompositor, "GPU Compositing: %s",
                gpuFeatureStatusToString(gpuCompositingStatus));

#if BUILDFLAG(IS_OZONE)
        if (gpuCompositingStatus == gpu::kGpuFeatureStatusEnabled) {
            // See entry 3 in //gpu/config/software_rendering_list.json
            QRegularExpression filter(u"software|llvmpipe|softpipe"_s,
                                      QRegularExpression::CaseInsensitiveOption);
            if (filter.match(QLatin1StringView(gpuInfo.gl_renderer)).hasMatch()) {
                qWarning("Hardware rendering is enabled but it is not supported with Mesa software "
                         "rasterizer. Expect troubles.");

                if (m_client->gpuPreferences().ignore_gpu_blocklist)
                    qWarning("Rendering may fail because --ignore-gpu-blocklist is set.");
            }
        }
#endif

        qCDebug(lcWebEngineCompositor, "%ls", qUtf16Printable(angleInfo(gpuInfo)));
#if BUILDFLAG(IS_WIN)
        qCDebug(lcWebEngineCompositor, "%ls", qUtf16Printable(windowsInfo(gpuInfo)));
#endif
    }

private:
#if BUILDFLAG(IS_OZONE)
    ContentGpuClientQt *m_client;
#endif
    std::optional<gpu::GPUInfo> m_gpuInfo;
};

ContentGpuClientQt::ContentGpuClientQt() = default;
ContentGpuClientQt::~ContentGpuClientQt() = default;

void ContentGpuClientQt::GpuServiceInitialized()
{
    // This is expected to be called on the GPU thread.
    Q_ASSERT(!content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    m_gpuTaskRunner = base::SingleThreadTaskRunner::GetCurrentDefault();

    m_gpuObserver.reset(new GpuObserver(this));
    content::GpuDataManager::GetInstance()->AddObserver(m_gpuObserver.get());
}

void ContentGpuClientQt::ExposeInterfacesToBrowser(
        viz::GpuServiceImpl *gpu_service,
        const gpu::GpuPreferences &gpu_preferences,
        const gpu::GpuDriverBugWorkarounds &gpu_workarounds, mojo::BinderMap *binders)
{
    Q_UNUSED(gpu_service);
    Q_UNUSED(gpu_workarounds);
    Q_UNUSED(binders);
    m_gpuPreferences = gpu_preferences;
}

} // namespace QtWebEngineCore
