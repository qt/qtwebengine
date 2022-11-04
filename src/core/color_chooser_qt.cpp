// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "color_chooser_qt.h"
#include "color_chooser_controller.h"
#include "color_chooser_controller_p.h"

#include <QColor>

namespace QtWebEngineCore {

ColorChooserQt::ColorChooserQt(content::WebContents *content, const QColor &color)
{
    m_controller.reset(new ColorChooserController(new ColorChooserControllerPrivate(content, color)));
}

QSharedPointer<ColorChooserController> ColorChooserQt::controller()
{
    return m_controller;
}

} // namespace QtWebEngineCore
