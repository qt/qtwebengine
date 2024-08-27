// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef LOCATION_PROVIDER_QT_H
#define LOCATION_PROVIDER_QT_H

#include <QtCore/qtconfigmacros.h>

#include "services/device/public/cpp/geolocation/geoposition.h"
#include "services/device/public/cpp/geolocation/location_provider.h"

QT_FORWARD_DECLARE_CLASS(QThread)

namespace QtWebEngineCore {
class QtPositioningHelper;

class LocationProviderQt : public device::LocationProvider
{
public:
    LocationProviderQt();
    virtual ~LocationProviderQt();

    // LocationProvider
    void StartProvider(bool high_accuracy) override;
    void StopProvider() override;
    const device::mojom::GeopositionResult* GetPosition() override { return m_lastKnownPosition.get(); }
    void OnPermissionGranted() override;
    void SetUpdateCallback(const LocationProviderUpdateCallback &callback) override;
    void FillDiagnostics(device::mojom::GeolocationDiagnostics &) override {}

private:
    friend class QtPositioningHelper;

    void updatePosition(device::mojom::GeopositionResultPtr);

    device::mojom::GeopositionResultPtr m_lastKnownPosition;
    LocationProviderUpdateCallback m_callback;
    QtPositioningHelper *m_positioningHelper;
};
} // namespace QtWebEngineCore

#endif // LOCATION_PROVIDER_QT_H
