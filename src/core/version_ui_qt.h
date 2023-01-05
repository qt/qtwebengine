// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef VERSION_UI_QT_H_
#define VERSION_UI_QT_H_

#include "build/build_config.h"
#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/web_ui_data_source.h"

class VersionUIQt : public content::WebUIController
{
public:
    explicit VersionUIQt(content::WebUI *web_ui);
    ~VersionUIQt() override;

    VersionUIQt(const VersionUIQt &) = delete;
    VersionUIQt &operator=(const VersionUIQt &) = delete;
};

#endif // VERSION_UI_QT_H
