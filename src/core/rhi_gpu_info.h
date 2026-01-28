// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef RHI_GPU_INFO_H
#define RHI_GPU_INFO_H

#include "qtwebenginecoreglobal_p.h"

#include <QtCore/qstring.h>

#include <vector>

namespace QtWebEngineCore {

class RhiGpuInfo
{
public:
    enum Vendor {
        Unknown = -1,

        // PCI-SIG-registered vendors
        AMD,
        Apple,
        ARM,
        Google,
        ImgTec,
        Intel,
        Microsoft,
        Nvidia,
        Qualcomm,
        Samsung,
        Broadcom,
        VMware,
        VirtIO,

        // Khronos-registered vendors
        Vivante,
        VeriSilicon,
        Kazan,
        CodePlay,
        Mesa,
        PoCL,
    };

    static RhiGpuInfo *instance();
    static QString vendorIdToString(quint64 vendorId);

    Vendor vendor() const { return m_vendor; }
    QString vendorName() const;
    QString deviceName() const { return m_deviceName; }
    QString getAdapterLuid() const { return m_adapterLuid; }

private:
    struct VendorEntry
    {
        quint64 id;
        Vendor vendor;
        const char *name;
    };

    // clang-format off
    // Based on //third_party/angle/src/gpu_info_util/SystemInfo.h
    static inline const std::vector<VendorEntry> kVendorTable = {
        {0x1002, AMD, "AMD"},
        {0x106B, Apple, "Apple"},
        {0x13B5, ARM, "ARM"},
        {0x1AE0, Google, "Google"},
        {0x1010, ImgTec, "Img Tec"},
        {0x8086, Intel, "Intel"},
        {0x1414, Microsoft, "Microsoft"},
        {0x10DE, Nvidia, "Nvidia"},
        {0x5143, Qualcomm, "Qualcomm"},
        {0x144D, Samsung, "Samsung"},
        {0x14E4, Broadcom, "Broadcom"},
        {0x15AD, VMware, "VMware"},
        {0x1AF4, VirtIO, "VirtIO"},
        {0x10001, Vivante, "Vivante"},
        {0x10002, VeriSilicon, "VeriSilicon"},
        {0x10003, Kazan, "Kazan"},
        {0x10004, CodePlay, "CodePlay"},
        {0x10005, Mesa, "Mesa"},
        {0x10006, PoCL, "PoCL"}
    };
    // clang-format on

    RhiGpuInfo();

    Vendor m_vendor = Unknown;
    QString m_deviceName;
    QString m_adapterLuid;
};

} // namespace QtWebEngineCore

#endif // RHI_GPU_INFO_H
