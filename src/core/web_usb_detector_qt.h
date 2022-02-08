/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
