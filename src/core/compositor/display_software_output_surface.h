// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DISPLAY_SOFTWARE_OUTPUT_SURFACE_H
#define DISPLAY_SOFTWARE_OUTPUT_SURFACE_H

#include "components/viz/service/display_embedder/software_output_surface.h"

namespace QtWebEngineCore {

class DisplaySoftwareOutputSurface final : public viz::SoftwareOutputSurface
{
public:
    DisplaySoftwareOutputSurface(bool requiresAlpha);
    ~DisplaySoftwareOutputSurface() override;

    // Overridden from viz::OutputSurface.
    void SetFrameSinkId(const viz::FrameSinkId &id) override;

private:
    class Device;
};

} // namespace QtWebEngineCore

#endif // !DISPLAY_SOFTWARE_OUTPUT_SURFACE_H
