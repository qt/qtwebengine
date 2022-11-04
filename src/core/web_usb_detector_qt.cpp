// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chrome/browser/usb/web_usb_detector.cc
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "web_usb_detector_qt.h"

#include "qtwebenginecoreglobal_p.h"

#include "content/public/browser/device_service.h"
#include "device/base/features.h"
#include "services/device/public/mojom/usb_device.mojom.h"

WebUsbDetectorQt::WebUsbDetectorQt() = default;

WebUsbDetectorQt::~WebUsbDetectorQt() = default;

void WebUsbDetectorQt::Initialize()
{
    if (!m_deviceManager) {
        // Receive mojo::Remote<UsbDeviceManager> from DeviceService.
        content::GetDeviceService().BindUsbDeviceManager(
                m_deviceManager.BindNewPipeAndPassReceiver());
    }
    DCHECK(m_deviceManager);

    // Listen for added/removed device events.
    DCHECK(!m_clientReceiver.is_bound());
    m_deviceManager->SetClient(m_clientReceiver.BindNewEndpointAndPassRemote());
}

void WebUsbDetectorQt::OnDeviceAdded(device::mojom::UsbDeviceInfoPtr device_info)
{
    Q_UNUSED(device_info);
    QT_NOT_YET_IMPLEMENTED
}

void WebUsbDetectorQt::OnDeviceRemoved(device::mojom::UsbDeviceInfoPtr device_info)
{
    Q_UNUSED(device_info);
    QT_NOT_YET_IMPLEMENTED
}
