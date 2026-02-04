// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "rhi_gpu_info.h"

#include <QtQuick/private/qsgrhisupport_p.h>
#include <rhi/qrhi.h>

#if BUILDFLAG(IS_OZONE)
#include "ozone/ozone_util_qt.h"
#endif

#if QT_CONFIG(opengl)
#include <QtGui/qoffscreensurface.h>
#endif

#if QT_CONFIG(webengine_vulkan)
#include <QtGui/qvulkaninstance.h>
#endif

using namespace Qt::StringLiterals;

namespace QtWebEngineCore {
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

#if QT_CONFIG(webengine_vulkan)
bool RhiGpuInfo::isVulkanSupported()
{
    static bool supported = []() {
        QVulkanInstance vulkanInstance;
        vulkanInstance.setApiVersion(QVersionNumber(1, 1));
        if (!vulkanInstance.create())
            return false;

        QRhiVulkanInitParams params;
        params.inst = &vulkanInstance;
        return QRhi::probe(QRhi::Vulkan, &params);
    }();

    return supported;
}
#endif // QT_CONFIG(webengine_vulkan)

class ScopedRhi
{
public:
    ScopedRhi(QRhi::Implementation backend)
    {
        switch (backend) {
#if QT_CONFIG(opengl)
        case QRhi::OpenGLES2: {
            QRhiGles2InitParams params;
            m_fallbackSurface.reset(QRhiGles2InitParams::newFallbackSurface());
            params.fallbackSurface = m_fallbackSurface.get();
            m_rhi.reset(QRhi::create(QRhi::OpenGLES2, &params));
            break;
        }
#endif // QT_CONFIG(opengl)
#if QT_CONFIG(webengine_vulkan)
        case QRhi::Vulkan: {
            // TODO: The primary GPU is not necessarily the one which is connected to the
            // display in case of a Multi-GPU setup on Linux. This can be workarounded by
            // installing the Mesa's Device Selection Layer,
            // see https://www.phoronix.com/news/Mesa-20.1-Vulkan-Dev-Selection
            // Try to detect this case and at least warn about it.
            m_vulkanInstance.reset(new QVulkanInstance());
            m_vulkanInstance->setApiVersion(QVersionNumber(1, 1));
            if (m_vulkanInstance->create()) {
                QRhiVulkanInitParams params;
                params.inst = m_vulkanInstance.get();
                m_rhi.reset(QRhi::create(QRhi::Vulkan, &params));
            }
            break;
        }
#endif // QT_CONFIG(webengine_vulkan)
#if defined(Q_OS_WIN)
        case QRhi::D3D11: {
            static const bool preferSoftware =
                    qEnvironmentVariableIntValue("QSG_RHI_PREFER_SOFTWARE_RENDERER");
            QRhiD3D11InitParams params;

            QRhi::Flags flags = preferSoftware ? QRhi::PreferSoftwareRenderer : QRhi::Flags();
            m_rhi.reset(QRhi::create(QRhi::D3D11, &params, flags));

            // mimic what QSGRhiSupport and QBackingStoreRhi does
            if (!m_rhi && !preferSoftware)
                m_rhi.reset(QRhi::create(QRhi::D3D11, &params, QRhi::PreferSoftwareRenderer));
            break;
        }
        case QRhi::D3D12:
            // TODO:
            qWarning("D3D12 backend is currently not supported.");
            break;
#endif // defined(Q_OS_WIN)
#if QT_CONFIG(metal)
        case QRhi::Metal: {
            QRhiMetalInitParams params;
            m_rhi.reset(QRhi::create(QRhi::Metal, &params));
            break;
        }
#endif // QT_CONFIG(metal)
        case QRhi::Null: {
            QRhiNullInitParams params;
            m_rhi.reset(QRhi::create(QRhi::Null, &params));
            break;
        }
        default:
            Q_UNREACHABLE();
        }
    }

    bool isValid() const
    {
        if (!m_rhi)
            return false;

        // The deviceName property is expected to be available across all graphics APIs.
        // If empty, assume initialization failed.
        if (m_rhi->driverInfo().deviceName.isEmpty())
            return false;

        return true;
    }

    // Forward QRhi API:
    QRhiDriverInfo driverInfo() const { return m_rhi->driverInfo(); }
    const QRhiNativeHandles *nativeHandles() { return m_rhi->nativeHandles(); }

private:
#if QT_CONFIG(opengl)
    QScopedPointer<QOffscreenSurface> m_fallbackSurface;
#endif
#if QT_CONFIG(webengine_vulkan)
    QScopedPointer<QVulkanInstance> m_vulkanInstance;
#endif

    // Must be declared last to ensure QRhi is destroyed before its dependencies.
    QScopedPointer<QRhi> m_rhi;
};

RhiGpuInfo::RhiGpuInfo()
{
    QSGRhiSupport *rhiSupport = QSGRhiSupport::instance();
    QRhi::Implementation backend = rhiSupport->rhiBackend();
    m_backendName = rhiSupport->rhiBackendName();

    ScopedRhi rhi(backend);
    if (!rhi.isValid()) {
        qWarning("Failed to create RHI for backend: %s", qUtf8Printable(m_backendName));
        return;
    }

#if defined(Q_OS_WIN)
    if (backend == QRhi::D3D11) {
        const QRhiD3D11NativeHandles *handles =
                static_cast<const QRhiD3D11NativeHandles *>(rhi.nativeHandles());
        Q_ASSERT(handles);
        m_adapterLuid = QString::number(handles->adapterLuidHigh) % QLatin1Char(',')
                % QString::number(handles->adapterLuidLow);
    }
#endif

    const QRhiDriverInfo &driverInfo = rhi.driverInfo();
    m_deviceName = QString::fromUtf8(driverInfo.deviceName);
    m_vendor = determineVendor(driverInfo.vendorId);

#if BUILDFLAG(IS_OZONE)
    m_isGbmSupported = determineGbmSupport();
#endif
}

RhiGpuInfo::Vendor RhiGpuInfo::determineVendor(const quint64 vendorId) const
{
    auto vendorIdToVendor = [](const quint64 vendorId) -> Vendor {
        for (const auto &entry : kVendorTable) {
            if (entry.id == vendorId)
                return entry.vendor;
        }

        qWarning("Unknown Vendor ID: 0x%llx.", vendorId);
        return Unknown;
    };

    // vendorId is always 0x0 for OpenGL and Metal.
    if (vendorId != 0x0)
        return vendorIdToVendor(vendorId);

#if QT_CONFIG(webengine_vulkan)
    // Attempt to detect the vendor using Vulkan as a fallback.
    QRhi::Implementation backend = QSGRhiSupport::instance()->rhiBackend();
    if (backend != QRhi::Vulkan && backend != QRhi::Null) {
        ScopedRhi vulkanRhi(QRhi::Vulkan);
        if (vulkanRhi.isValid() && vulkanRhi.driverInfo().vendorId != 0x0)
            return vendorIdToVendor(vulkanRhi.driverInfo().vendorId);
    }
#endif

    // Attempt to extract the vendor from deviceName.
    if (m_deviceName.contains("AMD"_L1, Qt::CaseInsensitive))
        return AMD;
    if (m_deviceName.contains("Intel"_L1, Qt::CaseInsensitive))
        return Intel;
    if (m_deviceName.contains("Nvidia"_L1, Qt::CaseInsensitive) || isNouveau())
        return Nvidia;
    if (m_deviceName.contains("VMware"_L1, Qt::CaseInsensitive))
        return VMware;
    if (m_deviceName.contains("Mesa llvmpipe"_L1, Qt::CaseInsensitive))
        return Mesa;
    if (m_deviceName.contains("Apple"_L1, Qt::CaseInsensitive))
        return Apple;

    qWarning("Unable to detect GPU vendor for device: %s", qUtf8Printable(m_deviceName));
    return Unknown;
}

#if BUILDFLAG(IS_OZONE)
bool RhiGpuInfo::determineGbmSupport() const
{
    const char kForceGbmEnv[] = "QTWEBENGINE_FORCE_USE_GBM";
    if (Q_UNLIKELY(qEnvironmentVariableIsSet(kForceGbmEnv))) {
        qWarning("%s environment variable is set and it is for debugging purposes only.",
                 kForceGbmEnv);
        bool ok;
        int forceGbm = qEnvironmentVariableIntValue(kForceGbmEnv, &ok);
        if (ok) {
            qWarning("GBM support is force %s.", forceGbm != 0 ? "enabled" : "disabled");
            return (forceGbm != 0);
        }

        qWarning("Ignoring invalid value of %s and do not force GBM. "
                 "Use 0 to force disable or 1 to force enable.",
                 kForceGbmEnv);
    }

    if (m_vendor == Nvidia && !isNouveau()) {
        // FIXME: This disables GBM for Nvidia. Remove this when Nvidia fixes its GBM support.
        //
        // "Buffer allocation and submission to DRM KMS using gbm is not currently supported."
        // See: https://download.nvidia.com/XFree86/Linux-x86_64/570.86.16/README/kms.html"
        //
        // Chromium uses GBM to allocate scanout buffers. Scanout requires DRM KMS. If KMS is
        // enabled, gbm_device and gbm_buffer are created without any issues but rendering to
        // the buffer will malfunction. It is not known how to detect this problem before
        // rendering so we just disable GBM for Nvidia.
        return false;
    }

#if !BUILDFLAG(IS_OZONE_X11)
    if (OzoneUtilQt::usingGLX()) {
        qWarning("GLX: Disable GBM because Ozone X11 is not available. "
                 "Possibly caused by missing libraries for qpa-xcb support.");
        return false;
    }
#endif

    return true;
}
#endif // BUILDFLAG(IS_OZONE)

QString RhiGpuInfo::vendorName() const
{
    for (const auto &entry : kVendorTable) {
        if (entry.vendor == m_vendor)
            return QString::fromLatin1(entry.name);
    }

    return u"Unknown"_s;
}

bool RhiGpuInfo::isNouveau() const
{
    // OpenGL
    if (m_deviceName.contains("Mesa NV"_L1, Qt::CaseInsensitive)
        || m_deviceName.contains("nouveau"_L1, Qt::CaseInsensitive)) {
        return true;
    }

    // Vulkan and zink
    if (m_deviceName.contains("(NVK"_L1, Qt::CaseInsensitive))
        return true;

    return false;
}

} // namespace QtWebEngineCore
