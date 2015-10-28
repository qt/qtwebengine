/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "content/browser/web_contents/web_contents_impl.h"

#include "color_chooser_controller.h"
#include "color_chooser_controller_p.h"
#include "type_conversion.h"

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


} // namespace
