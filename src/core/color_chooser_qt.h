// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef COLOR_CHOOSER_QT_H
#define COLOR_CHOOSER_QT_H

#include "content/public/browser/color_chooser.h"
#include "type_conversion.h"

#include <QSharedPointer>

QT_FORWARD_DECLARE_CLASS(QColor)

namespace content {
class WebContents;
}

namespace QtWebEngineCore {

class ColorChooserController;

class ColorChooserQt : public content::ColorChooser
{
public:
    ColorChooserQt(content::WebContents *, const QColor &);

    void SetSelectedColor(SkColor /*color*/) override { }
    void End() override {}

    QSharedPointer<ColorChooserController> controller();

private:
    QSharedPointer<ColorChooserController> m_controller;
};

} // namespace QtWebEngineCore

#endif // COLOR_CHOOSER_QT_H
