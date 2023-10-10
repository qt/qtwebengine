// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "content/browser/web_contents/web_contents_impl.h"

#include "color_chooser_controller.h"
#include "color_chooser_controller_p.h"
#include "type_conversion.h"

#include <QColor>
#include <QVariant>

namespace QtWebEngineCore {

ColorChooserControllerPrivate::ColorChooserControllerPrivate(content::WebContents *content, const QColor &color)
    : m_content(content)
    , m_initialColor(color)
{
}

ColorChooserController::~ColorChooserController()
{
}

ColorChooserController::ColorChooserController(ColorChooserControllerPrivate *dd)
{
    Q_ASSERT(dd);
    d.reset(dd);
}

QColor ColorChooserController::initialColor() const
{
    return d->m_initialColor;
}

void ColorChooserController::didEndColorDialog()
{
    d->m_content->DidEndColorChooser();
}

void ColorChooserController::didChooseColorInColorDialog(const QColor &color)
{
    d->m_content->DidChooseColorInColorChooser(toSk(color));
}

void ColorChooserController::accept(const QColor &color)
{
    didChooseColorInColorDialog(color);
    didEndColorDialog();
}

void ColorChooserController::accept(const QVariant &color)
{
    QColor selectedColor;
    if (color.canConvert<QColor>()) {
        selectedColor = color.value<QColor>();
        didChooseColorInColorDialog(selectedColor);
    }

    didEndColorDialog();
}

void ColorChooserController::reject()
{
    didEndColorDialog();
}

} // namespace QtWebEngineCore
