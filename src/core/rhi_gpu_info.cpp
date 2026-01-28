// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "rhi_gpu_info.h"

#include <rhi/qrhi.h>

#if QT_CONFIG(opengl)
#include <QtGui/qoffscreensurface.h>
#endif

#if QT_CONFIG(webengine_vulkan)
#include <QtGui/qvulkaninstance.h>
#endif

using namespace Qt::StringLiterals;

namespace QtWebEngineCore {
static RhiGpuInfo::Vendor deviceNameToVendor(QLatin1StringView deviceName)
{
    // TODO: Test and add more vendors to the list.
    if (deviceName.contains("AMD"_L1, Qt::CaseInsensitive))
        return RhiGpuInfo::AMD;
    if (deviceName.contains("Intel"_L1, Qt::CaseInsensitive))
        return RhiGpuInfo::Intel;
    if (deviceName.contains("Nvidia"_L1, Qt::CaseInsensitive))
        return RhiGpuInfo::Nvidia;
    if (deviceName.contains("VMware"_L1, Qt::CaseInsensitive))
        return RhiGpuInfo::VMware;
    if (deviceName.contains("Mesa llvmpipe"_L1))
        return RhiGpuInfo::Mesa;
    if (deviceName.contains("Apple"_L1))
        return RhiGpuInfo::Apple;

    return RhiGpuInfo::Unknown;
}

RhiGpuInfo *RhiGpuInfo::instance()
{
    static RhiGpuInfo instance;
    return &instance;
}

QString RhiGpuInfo::vendorIdToString(const quint64 vendorId)
{
    for (const auto &entry : kVendorTable) {
        if (entry.id == vendorId)
            return QString::fromLatin1(entry.name);
    }

    return u"Unknown (0x%1)"_s.arg(vendorId, 0, 16);
}

RhiGpuInfo::RhiGpuInfo()
{
    auto vendorIdToVendor = [](const quint64 vendorId) -> Vendor {
        for (const auto &entry : kVendorTable) {
            if (entry.id == vendorId)
                return entry.vendor;
        }

        qWarning("Unknown Vendor ID: 0x%llx.", vendorId);
        return Unknown;
    };

#if defined(Q_OS_WIN)
    {
        static const bool preferSoftwareDevice =
                qEnvironmentVariableIntValue("QSG_RHI_PREFER_SOFTWARE_RENDERER");
        QRhiD3D11InitParams params;
        QRhi::Flags flags;
        if (preferSoftwareDevice) {
            flags |= QRhi::PreferSoftwareRenderer;
        }
        QScopedPointer<QRhi> d3d11Rhi(QRhi::create(QRhi::D3D11, &params, flags, nullptr));
        // mimic what QSGRhiSupport and QBackingStoreRhi does
        if (!d3d11Rhi && !preferSoftwareDevice) {
            flags |= QRhi::PreferSoftwareRenderer;
            d3d11Rhi.reset(QRhi::create(QRhi::D3D11, &params, flags, nullptr));
        }
        if (d3d11Rhi) {
            m_vendor = vendorIdToVendor(d3d11Rhi->driverInfo().vendorId);
            m_deviceName = QString::fromUtf8(d3d11Rhi->driverInfo().deviceName);

            const QRhiD3D11NativeHandles *handles =
                    static_cast<const QRhiD3D11NativeHandles *>(d3d11Rhi->nativeHandles());
            Q_ASSERT(handles);
            m_adapterLuid = QString::number(handles->adapterLuidHigh) % QLatin1Char(',')
                    % QString::number(handles->adapterLuidLow);
        }
    }
#elif defined(Q_OS_MACOS)
    {
        QRhiMetalInitParams params;
        QScopedPointer<QRhi> metalRhi(QRhi::create(QRhi::Metal, &params, QRhi::Flags(), nullptr));
        if (metalRhi) {
            m_vendor = deviceNameToVendor(QLatin1StringView(metalRhi->driverInfo().deviceName));
            m_deviceName = QString::fromUtf8(metalRhi->driverInfo().deviceName);
        }
    }
#endif

#if QT_CONFIG(opengl)
    if (m_vendor == Unknown) {
        QRhiGles2InitParams params;
        params.fallbackSurface = QRhiGles2InitParams::newFallbackSurface();
        QScopedPointer<QRhi> glRhi(QRhi::create(QRhi::OpenGLES2, &params, QRhi::Flags(), nullptr));
        if (glRhi) {
            m_vendor = deviceNameToVendor(QLatin1StringView(glRhi->driverInfo().deviceName));
            m_deviceName = QString::fromUtf8(glRhi->driverInfo().deviceName);
        }
    }
#endif

#if QT_CONFIG(webengine_vulkan)
    if (m_vendor == Unknown) {
        QVulkanInstance vulkanInstance;
        vulkanInstance.setApiVersion(QVersionNumber(1, 1));
        if (vulkanInstance.create()) {
            QRhiVulkanInitParams params;
            params.inst = &vulkanInstance;
            QScopedPointer<QRhi> vulkanRhi(
                    QRhi::create(QRhi::Vulkan, &params, QRhi::Flags(), nullptr));
            if (vulkanRhi) {
                // TODO: The primary GPU is not necessarily the one which is connected to the
                // display in case of a Multi-GPU setup on Linux. This can be workarounded by
                // installing the Mesa's Device Selection Layer,
                // see https://www.phoronix.com/news/Mesa-20.1-Vulkan-Dev-Selection
                // Try to detect this case and at least warn about it.
                m_vendor = vendorIdToVendor(vulkanRhi->driverInfo().vendorId);
                m_deviceName = QString::fromUtf8(vulkanRhi->driverInfo().deviceName);
            }
        }
    }
#endif

    if (m_vendor == Unknown)
        qWarning("Unable to detect GPU vendor for device: %s", qUtf8Printable(m_deviceName));
}

QString RhiGpuInfo::vendorName() const
{
    for (const auto &entry : kVendorTable) {
        if (entry.vendor == m_vendor)
            return QString::fromLatin1(entry.name);
    }

    return u"Unknown"_s;
}

} // namespace QtWebEngineCore
