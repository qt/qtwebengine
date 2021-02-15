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

#include "web_usb_detector_qt.h"

#include "qtwebenginecoreglobal_p.h"

#include "content/public/browser/device_service.h"
#include "device/base/features.h"

WebUsbDetectorQt::WebUsbDetectorQt() = default;

WebUsbDetectorQt::~WebUsbDetectorQt() = default;

void WebUsbDetectorQt::Initialize()
{
#if defined(OS_WIN)
    // The WebUSB device detector is disabled on Windows due to jank and hangs
    // caused by enumerating devices. The new USB backend is designed to resolve
    // these issues so enable it for testing. https://crbug.com/656702
    if (!base::FeatureList::IsEnabled(device::kNewUsbBackend))
        return;
#endif // defined(OS_WIN)

    if (!m_deviceManager) {
        // Receive mojo::Remote<UsbDeviceManager> from DeviceService.
        content::GetDeviceService().BindUsbDeviceManager(
                m_deviceManager.BindNewPipeAndPassReceiver());
    }
    DCHECK(m_deviceManager);
    m_deviceManager.set_disconnect_handler(base::BindOnce(
            &WebUsbDetectorQt::OnDeviceManagerConnectionError, base::Unretained(this)));

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

void WebUsbDetectorQt::OnDeviceManagerConnectionError()
{
    m_deviceManager.reset();
    m_clientReceiver.reset();

    // Try to reconnect the device manager.
    Initialize();
}
