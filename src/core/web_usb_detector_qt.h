// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WEB_USB_DETECTOR_QT_H
#define WEB_USB_DETECTOR_QT_H

#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "services/device/public/mojom/usb_manager.mojom.h"
#include "services/device/public/mojom/usb_manager_client.mojom.h"
#include "url/gurl.h"

class WebUsbDetectorQt : public device::mojom::UsbDeviceManagerClient
{
public:
    WebUsbDetectorQt();
    ~WebUsbDetectorQt() override;

    void Initialize();

private:
    // device::mojom::UsbDeviceManagerClient implementation.
    void OnDeviceAdded(device::mojom::UsbDeviceInfoPtr device_info) override;
    void OnDeviceRemoved(device::mojom::UsbDeviceInfoPtr device_info) override;

    // Connection to |device_manager_instance_|.
    mojo::Remote<device::mojom::UsbDeviceManager> m_deviceManager;
    mojo::AssociatedReceiver<device::mojom::UsbDeviceManagerClient> m_clientReceiver { this };

    base::WeakPtrFactory<WebUsbDetectorQt> m_weakFactory { this };
};

#endif // WEB_USB_DETECTOR_QT_H
